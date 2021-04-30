#ifndef AIC21_CLIENT_CPP_AI_H
#define AIC21_CLIENT_CPP_AI_H

#include <bits/stdc++.h>
#include "Models/Game.h"
#include "Models/Answer.h"
#include "MyMap.h"
#include "EnemyMap.h"
#include "Search.h"

struct World {
    const Game* game;
    const Ant* me = game->getAnt();
    const int W = game->getMapWidth(), H = game->getMapHeight();
    const int me_x = me->getX(), me_y = me->getY();
    const AntType mytype = me->getType();
    const int viewdist = game->getViewDistance();

    World(const Game* game) : game(game) {}
};

struct Point {
    int x, y;
};

enum TargetReason {
    T_ATTACK,
    T_THREAT,
    T_NONE
};

class AI {
private:
    int randid, live_turn, cur_turn;

    unique_ptr<World> world;
    unique_ptr<Search> from_me;

    MyMap mymap;
    EnemyMap enemymap;

    Point target;
    TargetReason reason;
public:
    AI();
    Answer* turn(Game *game);
    void decide();
    bool manage_attack();
    bool manage_threat();

    Point find_threat();
    Direction find_dark();

    int count_local_enemy();
    bool is_threat(int x, int y);

    static std::string binary_str(std::string normal);
    static std::string normal_str(std::string binary);
    static int count_sarbaz(const Cell* cell, AntTeam team = ALLY);
    static CellState get_cellstate(const Cell* cell);
};

#endif // AIC21_CLIENT_CPP_AI_H
