#include "AI.h"
#include <bits/stdc++.h>
#include "Models/enums.h"

using namespace std;

Answer *AI::turn(Game *game) {
    int turn = 1;
    for (const Chat* chat : game->getChatBox()->getAllChats()) {
        turn = max(turn, chat->getTurn() + 1);
    }
    const int W = game->getMapWidth(), H = game->getMapHeight();
    mymap.init(W, H, turn);

    const Ant* me = game->getAnt();
    const int me_x = me->getX(), me_y = me->getY();
    const int viewdist = game->getViewDistance();

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
    
    return new Answer(UP);
}
