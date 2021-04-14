#include "MyMap.h"
#include <bits/stdc++.h>
using namespace std;

void MyMap::init(int w, int h, int turn) {
    if (W == 0) {
        W = w;
        H = h;
        data.assign(W, vector<MyCell>(H));
        for (int i = 0; i < W; i++)
        for (int j = 0; j < H; j++) {
            data[i][j] = MyCell(i, j, turn, C_UNKNOWN, false);
        }
    }
}

const MyCell& MyMap::at(int x, int y) const {
    return data[x][y];
}

void MyMap::update(int x, int y, int turn, CellState state, bool self) {
    auto lastcell = at(x, y);
    if (state != lastcell.get_state()) {
        if (lastcell.get_state() == C_BASE)
            return;
        data[x][y] = MyCell(x, y, turn, state, self);
        return;
    }
    if (turn < lastcell.get_seen())
        return;
    data[x][y] = MyCell(
        x, y,
        turn, 
        state, 
        self && lastcell.is_self()
    );
}

int MyMap::distance(int x1, int y1, int x2, int y2) const {
    int dx = min((x1 - x2 + W) % W, (x2 - x1 + W) % W);
    int dy = min((y1 - y2 + H) % H, (y2 - y1 + H) % H);
    return dx + dy;
}
