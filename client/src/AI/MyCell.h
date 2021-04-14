#pragma once

enum CellState {
    C_EMPTY,
    C_WALL,
    C_RES,
    C_BASE,
    C_UNKNOWN
};

class MyCell {
private:
    int x, y, lastseen;
    CellState state;
    bool self;
public:
    MyCell();
    MyCell(int x, int y, int lastseen, CellState state, bool self);
    int getX() const { return x; }
    int getY() const { return y; }
    int get_seen() const { return lastseen; }
    CellState get_state() const { return state; }
    bool is_self() const { return self; }
};