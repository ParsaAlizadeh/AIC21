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
    const int base_x = game->getBaseX(), base_y = game->getBaseY();
    const int viewdist = game->getViewDistance();
    const Resource* myresource = me->getCurrentResource();

    World(const Game* game) : game(game) {}
};

struct Point {
    int x, y;
};

enum TargetReason {
    T_PUTBACK,
    T_RESOURCE,
    T_ATTACK,
    T_THREAT,
    T_NONE
};

class AI {
private:
    int randid, live_turn, cur_turn;
    bool is_explorer;
    bool is_danger; // I'm not in danger, I am the danger!
    bool is_waiting;

    unique_ptr<World> world;
    unique_ptr<Search> from_me, from_base;

    MyMap mymap;
    EnemyMap enemymap;

    Point target;
    TargetReason reason;
public:
    AI();
    Answer* turn(Game *game);
    void decide();
    bool manage_resource();
    bool manage_attack();
    bool manage_threat();

    Point find_resource();
    Point find_threat();
    
    Direction find_dark();
    int count_local_enemy();
    bool is_threat(int x, int y);

    static std::string binary_str(std::string normal);
    static std::string normal_str(std::string binary);
    static int count_sarbaz(const Cell* cell, AntTeam team = ALLY);
    static bool match_resource(ResourceType restype, CellState state);
    static CellState get_cellstate(const Cell* cell);
};

#endif // AIC21_CLIENT_CPP_AI_H
