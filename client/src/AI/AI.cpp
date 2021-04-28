#include "AI.h"
#include <bits/stdc++.h>
#include "Models/enums.h"
using namespace std;
using namespace chrono;

const int MIN_SOLDIER_ATTACK = 3;

const int MAX_SOLDIER_WAIT = 1;
const int MAX_TURN_WAIT = 3;

const int MIN_RES_RETURN = 5;

const int MAX_CHAT_LEN = 32;

const int MIN_ENEMY_THREAT = 2;
const int MAX_LAST_THREAT = 5;

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

int AI::count_sarbaz(const Cell* cell, AntTeam team) {
    const auto& ants = cell->getPresentAnts();
    return count_if(begin(ants), end(ants), [&] (const Ant* ant) {
        return ant->getTeam() == team && ant->getType() == SARBAZ;
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

CellState AI::get_cellstate(const Cell* cell) {
    auto cell_type = cell->getType();
    auto res_type = cell->getResource()->getType();
    if (cell_type == WALL)
        return C_WALL;
    if (cell_type == TRAP)
        return C_TRAP;
    if (cell_type == SWAMP)
        return C_SWAMP;
    if (res_type == BREAD)
        return C_BREAD;
    if (res_type == GRASS)
        return C_GRASS;
    return C_EMPTY;
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
    enemymap.init(world->W, world->H);

    if (live_turn == 1) {
        is_explorer = (rand() % 5 == 0) && (world->mytype == SARBAZ);
    }
    is_waiting &= (
        world->mytype == SARBAZ &&
        !is_explorer &&
        live_turn <= MAX_TURN_WAIT &&
        count_sarbaz(world->me->getLocationCell()) <= MAX_SOLDIER_WAIT
    );
    if (is_waiting)
        return new Answer(CENTER);


    /* read messages */
    for (const Chat* chat : game->getChatBox()->getAllChats())
    if (live_turn == 1 || chat->getTurn() == cur_turn - 1) {
        string binary = binary_str(chat->getText());
        enemymap.decode(binary.substr(0, EnemyMap::size), chat->getTurn());
        for (int i = EnemyMap::size; i + MyCell::size <= binary.size(); i += MyCell::size) {
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
        CellState state = get_cellstate(cell);
        mymap.update(cell->getX(), cell->getY(), cur_turn, state, true);
    }

    { /* update enemymap at my position */
        int cnt = count_local_enemy();
        if (cnt >= MIN_ENEMY_THREAT) {
            enemymap.update(world->me_x, world->me_y, cur_turn);
        }
    }

    /* log mymap */
    if (live_turn == 1)
        cout << world->W << " " << world->H << endl;
    cout << world->me_x << " " << world->me_y << endl;
    for (int i = 0; i < world->W; i++)
    for (int j = 0; j < world->H; j++) {
        const MyCell& cell = mymap.at(i, j);
        int state = (int) cell.get_state();
        bool is_self = cell.is_self();
        cout << state << " " << is_self << " " << is_threat(i, j) << " \n"[j==world->H-1];
    }

    {/* pre search map */
        bool trap = world->myresource->getValue() == 0;
        from_me = unique_ptr<Search>(new Search(mymap, world->me_x, world->me_y, is_danger, trap));
        from_base = unique_ptr<Search>(new Search(mymap, world->base_x, world->base_y, false, false));
    }

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
    tie(response, importance) = mymap.get_updates(cur_turn, MAX_CHAT_LEN * 7 - EnemyMap::size);

    response = enemymap.encode(world->me_x, world->me_y, cur_turn) + response;
    if (enemymap.at(world->me_x, world->me_y) >= cur_turn)
        importance = max(importance, 3);
    
    return new Answer(finale, normal_str(response), importance);
}

void AI::decide() {
    if (world->myresource->getValue() >= MIN_RES_RETURN) {
        target = {world->base_x, world->base_y};
        reason = T_PUTBACK;
        return;
    }

    if (world->mytype == SARBAZ) {
        manage_attack();
        if (reason == T_ATTACK)
            return;

        manage_threat();
        if (reason == T_THREAT)
            return;
    }

    manage_resource();
    if (reason == T_RESOURCE)
        return;
}

bool AI::manage_resource() {
    if (is_explorer)
        return false;
    if (reason == T_RESOURCE) {
        CellState state = mymap.at(target.x, target.y).get_state();
        if (!match_resource(world->myresource->getType(), state)) {
            reason = T_NONE;
        }
    }
    Point new_res = find_resource();
    if (new_res.x < 0)
        return false;
    if (reason == T_RESOURCE) {
        int now = from_me->get_dist(new_res.x, new_res.y);
        int last = from_me->get_dist(target.x, target.y);
        if (last >= 0 && last <= now)
            return false;
    }
    target = new_res;
    reason = T_RESOURCE;
    return true;
}

bool AI::manage_attack() {
    if (reason == T_ATTACK)
        return false;
    int enemy_x = mymap.get_enemyX(), enemy_y = mymap.get_enemyY();
    if (enemy_x < 0)
        return false;
    if (!is_explorer && count_sarbaz(world->me->getLocationCell()) < MIN_SOLDIER_ATTACK)
        return false;
    
    is_danger = true;
    from_me = unique_ptr<Search>(new Search(mymap, world->me_x, world->me_y, is_danger, true));

    vector<Point> options;
    for (int dx = -world->viewdist; dx <= world->viewdist; dx++)
    for (int dy = -world->viewdist; dy <= world->viewdist; dy++) 
    if (abs(dx) + abs(dy) <= world->viewdist) {
        int x = mymap.addmod(enemy_x, dx, world->W);
        int y = mymap.addmod(enemy_y, dy, world->H);
        if (from_me->to(x, y) == CENTER)
            continue;
        options.push_back({x, y});
    }
    if (options.empty()) {
        // something must goes wrong
        is_danger = false;
        return false;
    }
    sort(begin(options), end(options), 
        [&] (const Point& a, const Point& b) {
            int a1 = mymap.distance(a.x, a.y, enemy_x, enemy_y);
            int a2 = from_me->get_dist(a.x, a.y);
            int b1 = mymap.distance(b.x, b.y, enemy_x, enemy_y);
            int b2 = from_me->get_dist(b.x, b.y);
            return (a1 + a2 != b1 + b2) ? (a1 + a2 < b1 + b2) : (a1 < b1);
        }
    );
    target = options[0];
    reason = T_ATTACK;
    return true;
}

bool AI::manage_threat() {
    if (is_explorer)
        return false;
    if (reason == T_THREAT && !is_threat(target.x, target.y)) {
        reason = T_NONE;
    }
    Point new_threat = find_threat();
    if (new_threat.x < 0)
        return false;
    if (reason == T_THREAT) {
        int last = from_me->get_dist(target.x, target.y);
        int now = from_me->get_dist(new_threat.x, new_threat.y);
        if (last >= 0 && last <= now)
            return false;
    }
    target = new_threat;
    reason = T_THREAT;
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
        if (from_me->get_dist(x, y) < 0 || from_base->get_dist(x, y) < 0)
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

Point AI::find_threat() {
    vector<Point> options;
    for (int x = 0; x < world->W; x++)
    for (int y = 0; y < world->H; y++) {
        if (from_me->get_dist(x, y) < 0)
            continue;
        if (!is_threat(x, y))
            continue;
        options.push_back({x, y});
    }
    if (options.empty())
        return {-1, -1};
    random_shuffle(begin(options), end(options));
    stable_sort(begin(options), end(options), 
        [&] (const Point& a, const Point& b) {
            return from_me->get_dist(a.x, a.y) < from_me->get_dist(b.x, b.y);
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

int AI::count_local_enemy() {
    int cnt = 0;
    for (int dx = -world->viewdist; dx <= world->viewdist; dx++)
    for (int dy = -world->viewdist; dy <= world->viewdist; dy++) {
        const Cell* cell = world->me->getNeighborCell(dx, dy);
        if (!cell)
            continue;
        const auto& ants = cell->getPresentAnts();
        cnt += count_if(begin(ants), end(ants), [&] (const Ant* ant) {
            return ant->getTeam() == ENEMY;
        });
    }
    return cnt;
}

bool AI::is_threat(int x, int y) {
    return cur_turn <= enemymap.at(x, y) + MAX_LAST_THREAT;
}
