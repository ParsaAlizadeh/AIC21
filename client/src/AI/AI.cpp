#include "AI.h"
#include <bits/stdc++.h>
#include "Models/enums.h"
using namespace std;
using namespace chrono;

AI::AI() :
    live_turn(0),
    is_explorer(false),
    is_danger(false),
    is_waiting(true)
{
    auto rseed = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    srand(rseed);
    randid = rand() % 128;
    live_turn = 0;
    freopen(("/tmp/ai/log_" + to_string(randid)).c_str(), "w", stdout);
}

string AI::normal_str(string binary) {
    while (binary.size() % 7) {
        binary.push_back('0');
    }
    string result;
    for (int i = 0; i < binary.size(); i += 7) {
        bitset<7> ch(binary, i, 7);
        result.push_back((char) ch.to_ulong());
    }
    return result;
}

string AI::binary_str(string normal) {
    string result;
    for (char c : normal) {
        bitset<7> ch((int) c);
        result += ch.to_string();
    }
    return result;
}

int AI::count_sarbaz(const Cell* cell) {
    const auto& ants = cell->getPresentAnts();
    return count_if(begin(ants), end(ants), [] (const Ant* ant) {
        return ant->getTeam() == ALLY && ant->getType() == SARBAZ;
    });
}

Answer *AI::turn(Game *game) {
    /* initialize turn and map */
    live_turn++;
    cur_turn = 1;
    for (const Chat* chat : game->getChatBox()->getAllChats()) {
        cur_turn = max(cur_turn, chat->getTurn() + 1);
    }

    /* constants */
    world = make_unique<World>(game);

    /* pre-phase init */
    mymap.init(world->W, world->H, cur_turn);
    if (live_turn == 1) {
        is_explorer = (rand() % 5 == 0) && (world->mytype == SARBAZ);
    }
    is_waiting &= (
        world->mytype == SARBAZ &&
        !is_explorer &&
        live_turn <= 3 &&
        count_sarbaz(world->me->getLocationCell()) <= 1
    );
    if (is_waiting)
        return new Answer(CENTER);


    /* read messages */
    for (const Chat* chat : game->getChatBox()->getAllChats())
    if (live_turn == 1 || chat->getTurn() == cur_turn - 1) {
        string binary = binary_str(chat->getText());
        for (int i = 0; i + 14 <= binary.size(); i += 14) {
            MyCell cell = MyCell::decode(binary.substr(i, 14), chat->getTurn());
            mymap.update(cell);
        }
    }

    /* find base attacks */
    for (const Attack* attack : game->getAttacks()) {
        if (!attack->isAttackerEnemy())
            continue;
        int x1 = attack->getAttackerColumn();
        int y1 = attack->getAttackerRow();
        int x2 = attack->getDefenderColumn();
        int y2 = attack->getDefenderRow();
        if (mymap.distance(x1, y1, x2, y2) > game->getAttackDistance()) {
            mymap.update(x1, y1, cur_turn, C_BASE, true);
        }
    }

    /* update visited cells */
    for (int dx = -world->viewdist; dx <= world->viewdist; dx++)
    for (int dy = -world->viewdist; dy <= world->viewdist; dy++) {
        const Cell* cell = world->me->getNeighborCell(dx, dy);
        if (!cell)
            continue;
        CellState state = C_EMPTY;
        if (cell->getType() == WALL)
            state = C_WALL;
        if (cell->getResource()->getType() != NONE)
            state = C_RES;
        mymap.update(cell->getX(), cell->getY(), cur_turn, state, true);
    }

    /* log mymap */
    if (live_turn == 1)
        cout << world->W << " " << world->H << endl;
    cout << world->me_x << " " << world->me_y << endl;
    for (int i = 0; i < world->W; i++)
    for (int j = 0; j < world->H; j++) {
        const MyCell& cell = mymap.at(i, j);
        cout << (int)cell.get_state() << " " << cell.is_self() << " \n"[j==world->H-1];
    }

    /* pre search map */
    from_me = make_unique<Search>(mymap, world->me_x, world->me_y, is_danger);
    from_base = make_unique<Search>(mymap, world->base_x, world->base_y, false);

    /* make a response for updates */
    string response;
    int importance;
    tie(response, importance) = mymap.get_updates(cur_turn, 32 * 7);
    
    return new Answer(UP, normal_str(response), importance);
}