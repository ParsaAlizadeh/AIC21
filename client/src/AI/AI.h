#ifndef AIC21_CLIENT_CPP_AI_H
#define AIC21_CLIENT_CPP_AI_H

#include "Models/Game.h"
#include "Models/Answer.h"
#include "MyMap.h"

class AI {
private:
    int randid;
    MyMap mymap;
public:
    AI();
    Answer* turn(Game *game);
};

#endif // AIC21_CLIENT_CPP_AI_H
