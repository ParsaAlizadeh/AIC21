#ifndef AIC21_CLIENT_CPP_AI_H
#define AIC21_CLIENT_CPP_AI_H

#include <bits/stdc++.h>
#include "Models/Game.h"
#include "Models/Answer.h"
#include "MyMap.h"
#include "Search.h"

class AI {
private:
    int randid, live_turn;
    MyMap mymap;
    std::pair<int, int> target;
    std::function<bool(const MyMap&, const Search&)> target_rule;
public:
    AI();
    Answer* turn(Game *game);

    static std::string binary_str(std::string normal);
    static std::string normal_str(std::string binary);
};

#endif // AIC21_CLIENT_CPP_AI_H
