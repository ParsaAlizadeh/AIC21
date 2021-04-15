#include "MyCell.h"
#include <bits/stdc++.h>
using namespace std;

MyCell::MyCell() : 
    x(-1),
    y(-1),
    lastseen(0),
    state(C_UNKNOWN),
    self(false)
{}

MyCell::MyCell(int x, int y, int lastseen, CellState state, bool self) :
    x(x),
    y(y),
    lastseen(lastseen),
    state(state),
    self(self)
{}

int MyCell::get_importance() const {
    switch (state) {
    case C_BASE:
        return 4;
    case C_RES:
        return 3;
    case C_WALL:
        return 2;
    case C_EMPTY:
        return 1;
    default:
        return 0;
    }
}

string MyCell::encode() const {
    stringstream stream;
    stream << bitset<6>(x);
    stream << bitset<6>(y);
    stream << bitset<2>((int) state);
    return stream.str();
}

MyCell MyCell::decode(string s, int turn) {
    int x = (int) bitset<6>(s, 0, 6).to_ulong();
    int y = (int) bitset<6>(s, 6, 6).to_ulong();
    CellState state = (CellState) bitset<2>(s, 12, 2).to_ulong();
    return MyCell(x, y, turn, state, false);
}