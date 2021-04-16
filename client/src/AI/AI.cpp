#include "AI.h"
#include <bits/stdc++.h>
#include "Models/enums.h"
using namespace std;

AI::AI() {
    srand(time(nullptr));
    randid = rand() % 128;
    freopen(("/tmp/ai/log_" + to_string(randid)).c_str(), "w", stdout);
    goal = {-1, -1};
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

Answer *AI::turn(Game *game) {
    /* initialize turn and map */
    live_turn++;
    int turn = 1;
    for (const Chat* chat : game->getChatBox()->getAllChats()) {
        turn = max(turn, chat->getTurn() + 1);
    }
    const int W = game->getMapWidth(), H = game->getMapHeight();
    mymap.init(W, H, turn);

    /* read messages */
    for (const Chat* chat : game->getChatBox()->getAllChats())
    if (live_turn == 1 || chat->getTurn() == turn - 1) {
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
            mymap.update(x1, y1, turn, C_BASE, true);
        }
    }

    /* constants */
    const Ant* me = game->getAnt();
    const int me_x = me->getX(), me_y = me->getY();
    const int base_x = game->getBaseX(), base_y = game->getBaseY();
    const int viewdist = game->getViewDistance();

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
        mymap.update(cell->getX(), cell->getY(), turn, state, true);
    }

    /* log mymap */
    if (live_turn == 1)
        cout << W << " " << H << endl;
    cout << me_x << " " << me_y << endl;
    for (int i = 0; i < W; i++)
    for (int j = 0; j < H; j++) {
        auto cell = mymap.at(i, j);
        cout << (int)cell.get_state() << " " << cell.is_self() << " \n"[j==H-1];
    }

    /* reset goal */
    if (me_x == goal.first && me_y == goal.second) {
        goal = {-1, -1};
    }

    Direction finale = CENTER;
    const Search from_me(mymap, me_x, me_y, false);
    const Search from_base(mymap, base_x, base_y, false);

    /* kargar put resource back */
    if (finale == CENTER && me->getCurrentResource()->getValue() > 0) {
        finale = from_me.to(base_x, base_y);
    }
    /* kargar find the nearest resource */
    if (finale == CENTER && me->getType() == KARGAR) {
        int best_dist = INT_MAX;
        for (int x = 0; x < W; x++)
        for (int y = 0; y < H; y++) {
            if (from_me.get_dist(x, y) < 0)
                continue;
            auto cell = mymap.at(x, y);
            if (cell.get_state() != C_RES)
                continue;
            int dist = from_me.get_dist(x, y) + from_base.get_dist(x, y);
            if (dist < best_dist) {
                best_dist = dist;
                finale = from_me.to(x, y);
            }
        }
    }
    /* sarbaz attack to the base */
    if (finale == CENTER && me->getType() == SARBAZ && mymap.get_enemyX() >= 0) {
        const Search to_attack(mymap, me_x, me_y, true);
        finale = to_attack.to(mymap.get_enemyX(), mymap.get_enemyY());
    }
    /* just to the latest goal */
    if (finale == CENTER && goal != make_pair(-1, -1)) {
        finale = from_me.to(goal.first, goal.second);
    }
    /* see dark areas of mymap */
    if (finale == CENTER) {
        int best_dist = INT_MAX;
        for (int i = 0; i < W; i++)
        for (int j = 0; j < H; j++) {
            if (from_me.get_dist(i, j) < 0)
                continue;
            auto cell = mymap.at(i, j);
            if (cell.get_state() != C_UNKNOWN)
                continue;
            int dist = max(from_me.get_dist(i, j), from_base.get_dist(i, j));
            if (dist <= best_dist) {
                best_dist = dist;
                finale = from_me.to(i, j);
                goal = {i, j};
            }
        }
    }
    /* random walk */
    if (finale == CENTER) {
        finale = Direction(rand() % 4 + 1);
    }

    /* make a response for updates */
    string response;
    int importance;
    tie(response, importance) = mymap.get_updates(turn, 32 * 7);
    
    return new Answer(finale, normal_str(response), importance);
}
