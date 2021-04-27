#ifndef AIC21_CLIENT_CPP_GAME_H
#define AIC21_CLIENT_CPP_GAME_H

class GameConfigMessage;
class CurrentStateMessage;
class Ant;

#include "Models/enums.h"
#include "Models/Ant.h"
#include "Models/ChatBox.h"
#include "Core/Message/Parse/GameConfigMessage.h"
#include "Core/EventQueue.h"
#include "Attack.h"
#include "Base.h"

class Game {

private:
    Ant *ant_;
    ChatBox *chat_box_;
    AntType ant_type_;
    int map_width_;
    int map_height_;
    int base_x_;
    int base_y_;
    int health_queen_;
    int health_scorpion_;
    int attack_distance_;
    int view_distance_;
    int generate_queen_;
    int generate_scorpion_;
    int rate_death_resource_;
    EventQueue &event_queue_;
    vector<const Attack*> attacks;
    void initGameConfig(GameConfigMessage *initMessage);
    void setCurrentState(CurrentStateMessage *stateMessage);
    vector<const Base*> bases_;

    friend class Controller;

public:
    explicit Game(EventQueue &eventQueue);
    Game(const Game&);
    ~Game();
    const Ant* getAnt() const;
    AntType getAntType() const;
    const ChatBox* getChatBox() const;
    int getMapWidth() const;
    int getMapHeight() const;
    int getBaseX() const;
    int getBaseY() const;
    int getHealthQueen() const;
    int getHealthScorpion() const;
    int getAttackDistance() const;
    int getViewDistance() const;
    int getGenerateQueen() const;
    int getGenerateScorpion() const;
    int getRateDeathResource() const;
    const vector<const Attack*>& getAttacks() const;
    const vector<const Base*>& getBases() const;
};

#endif //AIC21_CLIENT_CPP_GAME_H