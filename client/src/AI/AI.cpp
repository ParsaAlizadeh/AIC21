#include "AI.h"
#include <bits/stdc++.h>
#include "Models/enums.h"
using namespace std;
using namespace chrono;

AI::AI() :
    live_turn(0),
    is_explorer(false),
    is_danger(false),
    is_waiting(true),
    reason(T_NONE)
{
    auto rseed = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    srand(rseed);
    randid = rand() % 128;
    live_turn = 0;
    freopen(("/tmp/ai/log_" + to_string(randid)).c_str(), "w", stdout);
}

string AI::normal_str(string binary) {
    while (binary.size() % 7) {
        binary.push_back('0');
    }
    string result;
    for (int i = 0; i < binary.size(); i += 7) {
        bitset<7> ch(binary, i, 7);
        result.push_back((char) ch.to_ulong());
    }
    return result;
}

string AI::binary_str(string normal) {
    string result;
    for (char c : normal) {
        bitset<7> ch((int) c);
        result += ch.to_string();
    }
    return result;
}

int AI::count_sarbaz(const Cell* cell) {
    const auto& ants = cell->getPresentAnts();
    return count_if(begin(ants), end(ants), [] (const Ant* ant) {
        return ant->getTeam() == ALLY && ant->getType() == SARBAZ;
    });
}

inline bool AI::match_resource(ResourceType restype, CellState state) {
    if (restype == NONE)
        return state == C_GRASS || state == C_BREAD;
    if (restype == GRASS)
        return state == C_GRASS;
    if (restype == BREAD)
        return state == C_BREAD;
    return false;   // never happens
}

Answer *AI::turn(Game *game) {
    /* initialize turn and map */
    live_turn++;
    cur_turn = 1;
    for (const Chat* chat : game->getChatBox()->getAllChats()) {
        cur_turn = max(cur_turn, chat->getTurn() + 1);
    }

    /* constants */
    world = unique_ptr<World>(new World(game));

    /* pre-phase init */
    mymap.init(world->W, world->H, cur_turn);
    if (live_turn == 1) {
        is_explorer = (rand() % 5 == 0) && (world->mytype == SARBAZ);
    }
    is_waiting &= (
        world->mytype == SARBAZ &&
        !is_explorer &&
        live_turn <= 3 &&
        count_sarbaz(world->me->getLocationCell()) <= 1
    );
    if (is_waiting)
        return new Answer(CENTER);


    /* read messages */
    for (const Chat* chat : game->getChatBox()->getAllChats())
    if (live_turn == 1 || chat->getTurn() == cur_turn - 1) {
        string binary = binary_str(chat->getText());
        for (int i = 0; i + MyCell::size <= binary.size(); i += MyCell::size) {
            MyCell cell = MyCell::decode(binary.substr(i, MyCell::size), chat->getTurn());
            mymap.update(cell);
        }
    }

    /* find base attacks */
    for (const Attack* attack : game->getAttacks()) {
        if (!attack->isAttackerEnemy())
            continue;
        int x1 = attack->getAttackerColumn();
        int y1 = attack->getAttackerRow();
        int x2 = attack->getDefenderColumn();
        int y2 = attack->getDefenderRow();
        if (mymap.distance(x1, y1, x2, y2) > game->getAttackDistance()) {
            mymap.update(x1, y1, cur_turn, C_BASE, true);
        }
    }

    /* update visited cells */
    for (int dx = -world->viewdist; dx <= world->viewdist; dx++)
    for (int dy = -world->viewdist; dy <= world->viewdist; dy++) {
        const Cell* cell = world->me->getNeighborCell(dx, dy);
        if (!cell)
            continue;
        CellState state = C_EMPTY;
        if (cell->getType() == WALL)
            state = C_WALL;
        auto res_type = cell->getResource()->getType();
        if (res_type == BREAD)
            state = C_BREAD;
        if (res_type == GRASS)
            state = C_GRASS;
        mymap.update(cell->getX(), cell->getY(), cur_turn, state, true);
    }

    /* log mymap */
    if (live_turn == 1)
        cout << world->W << " " << world->H << endl;
    cout << world->me_x << " " << world->me_y << endl;
    for (int i = 0; i < world->W; i++)
    for (int j = 0; j < world->H; j++) {
        const MyCell& cell = mymap.at(i, j);
        cout << (int)cell.get_state() << " " << cell.is_self() << " \n"[j==world->H-1];
    }

    /* pre search map */
    from_me = unique_ptr<Search>(new Search(mymap, world->me_x, world->me_y, is_danger));
    from_base = unique_ptr<Search>(new Search(mymap, world->base_x, world->base_y, false));

    /* decide where */
    if (reason != T_NONE && from_me->to(target.x, target.y) == CENTER) {
        reason = T_NONE;
    }

    decide();
    
    Direction finale = CENTER;
    if (reason != T_NONE) {
        finale = from_me->to(target.x, target.y);
    }
    else {
        finale = find_dark();
    }

    /* make a response for updates */
    string response;
    int importance;
    tie(response, importance) = mymap.get_updates(cur_turn, 32 * 7);
    
    return new Answer(finale, normal_str(response), importance);
}

void AI::decide() {
    if (world->myresource->getValue() >= 5) {
        target = {world->base_x, world->base_y};
        reason = T_PUTBACK;
        return;
    }

    if (world->mytype == KARGAR) {
        manage_resource();
        if (reason == T_RESOURCE)
            return;
    }
}

bool AI::manage_resource() {
    if (reason == T_RESOURCE) {
        CellState state = mymap.at(target.x, target.y).get_state();
        if (!match_resource(world->myresource->getType(), state)) {
            reason = T_NONE;
        }
    }
    Point new_res = find_resource();
    if (new_res.x < 0)
        return false;
    if (
        reason == T_RESOURCE && 
        from_me->get_dist(new_res.x, new_res.y) >= from_me->get_dist(target.x, target.y)
    ) {
        return false;
    }
    target = new_res;
    reason = T_RESOURCE;
    return true;
}

Point AI::find_resource() {
    vector<Point> options;
    ResourceType restype = world->myresource->getType();
    for (int x = 0; x < world->W; x++)
    for (int y = 0; y < world->H; y++) {
        const MyCell& mycell = mymap.at(x, y);
        if (!match_resource(restype, mycell.get_state()))
            continue;
        if (from_me->to(x, y) == CENTER || from_base->to(x, y) == CENTER)
            continue;
        options.push_back({x, y});
    }
    if (options.empty())
        return {-1, -1};
    random_shuffle(begin(options), end(options));
    stable_sort(begin(options), end(options), 
        [&] (const Point& a, const Point& b) {
            int a1 = from_me->get_dist(a.x, a.y);
            int a2 = from_base->get_dist(a.x, a.y);
            int b1 = from_me->get_dist(b.x, b.y);
            int b2 = from_base->get_dist(b.x, b.y);
            return (a1 + a2 != b1 + b2) ? (a1 + a2 < b1 + b2) : (a1 < b1);
        }
    );
    return options[0];
}

Direction AI::find_dark() {
    vector<int> cnt(5, 0);
    for (int x = 0; x < world->W; x++)
    for (int y = 0; y < world->H; y++) {
        const MyCell& cell = mymap.at(x, y);
        if (cell.get_state() != C_UNKNOWN)
            continue;
        if (from_me->to(x, y) == CENTER)
            continue;
        cnt[(int) from_me->to(x, y)]++;
    }
    int dir = max_element(begin(cnt), end(cnt)) - begin(cnt);
    return Direction(dir);
}
