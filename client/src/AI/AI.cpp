#include "AI.h"
#include <bits/stdc++.h>
#include "Models/enums.h"

using namespace std;

Answer *AI::turn(Game *game) {
    const Ant* me = game->getAnt();

    int viewDist = me->getViewDistance();
    int targetX = -1, targetY = -1, dist = -1;
    if (me->getType() == KARGAR) {
        for (int i = -viewDist; i <= viewDist; i++) {
            for (int j = -viewDist; j <= viewDist; j++) {
                const Cell* cell = me->getNeighborCell(i, j);
                int _dist = abs(i) + abs(j);
                if (cell && cell->getResource()->getType() != NONE && (dist == -1 || _dist < dist)) {
                    targetX = cell->getX();
                    targetY = cell->getY();
                    dist = _dist;
                    break;
                }
            }
            if (targetX != -1)
                break;
        }
    }

    if (me->getCurrentResource()->getType() != NONE) {
        targetX = game->getBaseX();
        targetY = game->getBaseY();
    }

    if (targetX == -1) {
        return new Answer(LEFT, "up", 5);
    }

    int x = me->getX();
    int y = me->getY();
    Direction direction = CENTER;
    if (targetX > x) {
        direction = RIGHT;
    }
    else if (targetX < x) {
        direction = LEFT;
    }
    else if (targetY > y) {
        direction = DOWN;
    }
    else {
        direction = UP;
    }
    return new Answer(direction, "res", 10);
}
