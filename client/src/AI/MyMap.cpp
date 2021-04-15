#include "MyMap.h"
#include <bits/stdc++.h>
using namespace std;

MyMap::MyMap() : enemyX(-1), enemyY(-1) {}

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

void MyMap::update(const MyCell& cell) {
    return update(cell.getX(), cell.getY(), cell.get_seen(), cell.get_state(), cell.is_self());
}

int MyMap::distance(int x1, int y1, int x2, int y2) const {
    int dx = min((x1 - x2 + W) % W, (x2 - x1 + W) % W);
    int dy = min((y1 - y2 + H) % H, (y2 - y1 + H) % H);
    return dx + dy;
}

bool MyMap::is_danger(int x, int y) const {
    if (enemyX < 0 || enemyY < 0)
        return false;
    return distance(x, y, enemyX, enemyY) <= 6;
}

int MyMap::addmod(int a, int b, int md) {
    int c = (a + b) % md;
    return c < 0 ? c + md : c;
}

const pair<string, int> MyMap::get_updates(int turn, int max_size) const {
    vector<const MyCell*> cells;
    for (int x = 0; x < W; x++)
    for (int y = 0; y < H; y++) {
        const MyCell& cell = at(x, y);
        if (!cell.is_self())
            continue;
        CellState state = cell.get_state();
        if (state != C_WALL && state != C_BASE && cell.get_seen() < turn)
            continue;
        cells.push_back(&cell);
    }
    if (cells.empty())
        return {"", 0};
    random_shuffle(begin(cells), end(cells));
    stable_sort(begin(cells), end(cells),
        [&] (const MyCell* u, const MyCell* v) {
            return u->get_importance() > v->get_importance();
        }
    );
    string result;
    int importance = 0;
    for (const MyCell* cell : cells) {
        string cur = cell->encode();
        if (result.size() + cur.size() > max_size)
            break;
        result += cur;
        importance = max(importance, cell->get_importance());
    }
    return {result, importance};
}
