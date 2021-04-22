#include "AI.h"
#include <bits/stdc++.h>
#include "Models/enums.h"
using namespace std;
using namespace chrono;

AI::AI() :
    live_turn(0),
    is_explorer(false),
    is_danger(false),
    is_waiting(true)
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

Answer *AI::turn(Game *game) {
    /* initialize turn and map */
    live_turn++;
    cur_turn = 1;
    for (const Chat* chat : game->getChatBox()->getAllChats()) {
        cur_turn = max(cur_turn, chat->getTurn() + 1);
    }

    /* constants */
    const Ant* me = game->getAnt();
    const int me_x = me->getX(), me_y = me->getY();
    const int base_x = game->getBaseX(), base_y = game->getBaseY();
    const int viewdist = game->getViewDistance();
    const int W = game->getMapWidth(), H = game->getMapHeight();

    /* pre-phase init */
    mymap.init(W, H, cur_turn);
    if (live_turn == 1) {
        is_explorer = (rand() % 5 == 0) && (me->getType() == SARBAZ);
    }
    is_waiting &= (
        me->getType() == SARBAZ &&
        !is_explorer &&
        live_turn <= 3 &&
        count_sarbaz(me->getLocationCell()) <= 1
    );
    if (is_waiting)
        return new Answer(CENTER);


    /* read messages */
    for (const Chat* chat : game->getChatBox()->getAllChats())
    if (live_turn == 1 || chat->getTurn() == cur_turn - 1) {
        string binary = binary_str(chat->getText());
        for (int i = 0; i + 14 <= binary.size(); i += 14) {
            MyCell cell = MyCell::decode(binary.substr(i, 14), chat->getTurn());
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
    for (int dx = -viewdist; dx <= viewdist; dx++)
    for (int dy = -viewdist; dy <= viewdist; dy++) {
        const Cell* cell = me->getNeighborCell(dx, dy);
        if (!cell)
            continue;
        CellState state = C_EMPTY;
        if (cell->getType() == WALL)
            state = C_WALL;
        if (cell->getResource()->getType() != NONE)
            state = C_RES;
        mymap.update(cell->getX(), cell->getY(), cur_turn, state, true);
    }

    /* log mymap */
    if (live_turn == 1)
        cout << W << " " << H << endl;
    cout << me_x << " " << me_y << endl;
    for (int i = 0; i < W; i++)
    for (int j = 0; j < H; j++) {
        const MyCell& cell = mymap.at(i, j);
        cout << (int)cell.get_state() << " " << cell.is_self() << " \n"[j==H-1];
    }

    const Search from_me(mymap, me_x, me_y, is_danger);
    const Search from_base(mymap, base_x, base_y, false);

    /* reset target, based on search */
    if (target_rule && target_rule(mymap, from_me)) {
        target_rule = nullptr;
    }

    Direction finale = decide(game, from_me, from_base);

    /* make a response for updates */
    string response;
    int importance;
    tie(response, importance) = mymap.get_updates(cur_turn, 32 * 7);
    
    return new Answer(finale, normal_str(response), importance);
}

Direction AI::decide(Game *game, const Search& from_me, const Search& from_base) {
    const Ant* me = game->getAnt();
    const int W = game->getMapWidth(), H = game->getMapHeight();
    const int me_x = me->getX(), me_y = me->getY();
    const int base_x = game->getBaseX(), base_y = game->getBaseY();
    const int viewdist = game->getViewDistance();
    
    /* kargar put resource back */
    if (me->getCurrentResource()->getValue() > 0) {
        return from_me.to(base_x, base_y);
    }

    /* just to the latest target */
    if (target_rule) {
        return from_me.to(target.first, target.second);
    }

    /* sarbaz attack to the base */
    if (me->getType() == SARBAZ && mymap.get_enemyX() >= 0) {
        const Search attack(mymap, me_x, me_y, true);
        if (attack_base(game, from_me, from_base, attack)) {
            return attack.to(target.first, target.second);
        }
    }

    /* non-explorers find the nearest resource */
    if (!is_explorer && find_resource(game, from_me, from_base)) {
        return from_me.to(target.first, target.second);
    }

    /* see dark areas of mymap */ 
    if (find_dark(game, from_me, from_base)) {
        return from_me.to(target.first, target.second);
    }

    /* random walk */
    return Direction(rand() % 4 + 1);
}

bool AI::find_resource(Game *game, const Search& from_me, const Search& from_base) {
    const Ant* me = game->getAnt();
    const int W = game->getMapWidth(), H = game->getMapHeight();
    const int me_x = me->getX(), me_y = me->getY();
    const int base_x = game->getBaseX(), base_y = game->getBaseY();
    const int viewdist = game->getViewDistance();

    int best_dist = INT_MAX;
    int me_dist = 0;

    for (int x = 0; x < W; x++)
    for (int y = 0; y < H; y++) {
        if (from_me.get_dist(x, y) < 0 || from_base.get_dist(x, y) < 0)
            continue;
        const MyCell& cell = mymap.at(x, y);
        if (cell.get_state() != C_RES)
            continue;
        int dist = from_me.get_dist(x, y) + from_base.get_dist(x, y);
        if (dist < best_dist) {
            best_dist = dist;
            me_dist = from_me.get_dist(x, y);
        }
        if (dist <= best_dist && from_me.get_dist(x, y) <= me_dist) {
            best_dist = dist;
            me_dist = from_me.get_dist(x, y);
            target = {x, y};
        }
    }
    if (best_dist == INT_MAX)
        return false;
    bool is_karger = me->getType() == KARGAR;
    target_rule = [=] (const MyMap& mymap, const Search& from_me) {
        return (
            from_me.to(target.first, target.second) == CENTER ||
            mymap.at(target.first, target.second).get_state() != C_RES ||
            is_karger
        );
    };
    return true;
}

bool AI::find_dark(Game *game, const Search& from_me, const Search& from_base) {
    const Ant* me = game->getAnt();
    const int W = game->getMapWidth(), H = game->getMapHeight();
    const int me_x = me->getX(), me_y = me->getY();
    const int base_x = game->getBaseX(), base_y = game->getBaseY();
    const int viewdist = game->getViewDistance();

    int best_dist = INT_MAX;
    for (int i = 0; i < W; i++)
    for (int j = 0; j < H; j++) {
        if (from_me.get_dist(i, j) < 0)
            continue;
        const MyCell& cell = mymap.at(i, j);
        if (cell.get_state() != C_UNKNOWN)
            continue;
        int dist = max(from_me.get_dist(i, j), from_base.get_dist(i, j));
        if (dist <= best_dist) {
            best_dist = dist;
            target = {i, j};
        }
    }
    if (best_dist == INT_MAX)
        return false;
    target_rule = [=] (const MyMap& mymap, const Search& from_me) {
        return (
            from_me.to(target.first, target.second) == CENTER ||
            mymap.at(target.first, target.second).get_state() != C_UNKNOWN
        );
    };
    return true;
}

bool AI::attack_base(Game *game, const Search& from_me, const Search& from_base, const Search& attack) {
    const Ant* me = game->getAnt();
    const int W = game->getMapWidth(), H = game->getMapHeight();
    const int me_x = me->getX(), me_y = me->getY();
    const int base_x = game->getBaseX(), base_y = game->getBaseY();
    const int attackdist = game->getAttackDistance();

    int enemyx = mymap.get_enemyX(), enemyy = mymap.get_enemyY();

    if (me->getType() != SARBAZ || enemyx < 0)
        return false;
    if (!is_explorer && count_sarbaz(me->getLocationCell()) < 3)
        return false;

    int best = INT_MAX, base_dist = 0;
    for (int dx = -attackdist; dx <= attackdist; dx++)
    for (int dy = -attackdist; dy <= attackdist; dy++)
    if (abs(dx) + abs(dy) <= attackdist) {
        int x = mymap.addmod(enemyx, dx, W);
        int y = mymap.addmod(enemyy, dy, H);
        int d1 = attack.get_dist(x, y), d2 = abs(dx) + abs(dy);
        if (d1 < 0 || d1 + d2 > best)
            continue;
        if ((d1 + d2 < best) || (d1 + d2 <= best && d2 < base_dist)) {
            best = d1 + d2;
            base_dist = d2;
            target = {x, y};
        }
    }

    if (best == INT_MAX)
        return false;
    
    is_danger = true;
    target_rule = [=] (const MyMap& mymap, const Search& from_me) {
        return false;
    };

    return true;
}