#ifndef AIC21_CLIENT_CPP_AI_H
#define AIC21_CLIENT_CPP_AI_H

#include <bits/stdc++.h>
#include "Models/Game.h"
#include "Models/Answer.h"
#include "MyMap.h"
#include "Search.h"

struct World {
    const Game* game;
    const Ant* me = game->getAnt();
    const int W = game->getMapWidth(), H = game->getMapHeight();
    const int me_x = me->getX(), me_y = me->getY();
    const AntType mytype = me->getType();
    const int base_x = game->getBaseX(), base_y = game->getBaseY();
    const int viewdist = game->getViewDistance();

    World(const Game* game) : game(game) {}
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
public:
    AI();
    Answer* turn(Game *game);

    static std::string binary_str(std::string normal);
    static std::string normal_str(std::string binary);
    static int count_sarbaz(const Cell* cell);
};

#endif // AIC21_CLIENT_CPP_AI_H
