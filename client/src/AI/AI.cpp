#include "AI.h"
#include <bits/stdc++.h>
#include "Models/enums.h"

using namespace std;

AI::AI() {
    srand(time(nullptr));
    randid = rand() % 128;
    freopen(("/tmp/ai/log_" + to_string(randid)).c_str(), "w", stdout);
}

Answer *AI::turn(Game *game) {
    /* initialize turn and map */
    int turn = 1;
    for (const Chat* chat : game->getChatBox()->getAllChats()) {
        turn = max(turn, chat->getTurn() + 1);
    }
    const int W = game->getMapWidth(), H = game->getMapHeight();
    mymap.init(W, H, turn);

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
    if (turn == 1)
        cout << W << " " << H << endl;
    cout << me_x << " " << me_y << endl;
    for (int i = 0; i < W; i++)
    for (int j = 0; j < H; j++) {
        auto cell = mymap.at(i, j);
        cout << (int)cell.get_state() << " " << cell.is_self() << " \n"[j==H-1];
    }
    
    return new Answer(UP, "nothing", 0);
}
