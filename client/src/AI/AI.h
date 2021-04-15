#ifndef AIC21_CLIENT_CPP_AI_H
#define AIC21_CLIENT_CPP_AI_H

#include <string>
#include "Models/Game.h"
#include "Models/Answer.h"
#include "MyMap.h"
#include "Search.h"
using std::string;

class AI {
private:
    int randid, live_turn;
    MyMap mymap;
public:
    AI();
    Answer* turn(Game *game);

    static string binary_str(string normal);
    static string normal_str(string binary);
};

#endif // AIC21_CLIENT_CPP_AI_H
