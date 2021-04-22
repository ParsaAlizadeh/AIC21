#ifndef AIC21_CLIENT_CPP_AI_H
#define AIC21_CLIENT_CPP_AI_H

#include <bits/stdc++.h>
#include "Models/Game.h"
#include "Models/Answer.h"
#include "MyMap.h"
#include "Search.h"

class AI {
private:
    int randid, live_turn, cur_turn;
    bool is_explorer;
    bool is_danger; // I'm not in danger, I am the danger!
    bool is_waiting;
    MyMap mymap;
    std::pair<int, int> target;
    std::function<bool(const MyMap&, const Search&)> target_rule;
public:
    AI();
    Answer* turn(Game *game);

    Direction decide(Game *game, const Search& from_me, const Search& from_base);
    bool find_resource(Game *game, const Search& from_me, const Search& from_base);
    bool find_dark(Game *game, const Search& from_me, const Search& from_base);
    bool attack_base(Game *game, const Search& from_me, const Search& from_base, const Search& attack);

    static std::string binary_str(std::string normal);
    static std::string normal_str(std::string binary);
    static int count_sarbaz(const Cell* cell);
};

#endif // AIC21_CLIENT_CPP_AI_H
