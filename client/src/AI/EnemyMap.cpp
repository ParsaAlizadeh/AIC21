#include "EnemyMap.h"
#include <bits/stdc++.h>
using namespace std;

EnemyMap::EnemyMap() : W(0), H(0) {}

void EnemyMap::init(int w, int h) {
    if (W == 0) {
        W = w;
        H = h;
        data.assign(W, vector<int>(H, INT_MIN));
    }
}

void EnemyMap::update(int x, int y, int turn) {
    data[x][y] = max(at(x, y), turn);
}

int EnemyMap::size = 13;

void EnemyMap::decode(string s, int turn) {
    int x = (int) bitset<6>(s, 0, 6).to_ulong();
    int y = (int) bitset<6>(s, 6, 6).to_ulong();
    int state = (int) bitset<1>(s, 12, 1).to_ulong();
    if (state)
        update(x, y, turn);
}

string EnemyMap::encode(int x, int y, int turn) const {
    int state = at(x, y) >= turn;
    stringstream stream;
    stream << bitset<6>(x);
    stream << bitset<6>(y);
    stream << bitset<1>(state);
    return stream.str();
}