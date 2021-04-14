#pragma once
#include "MyCell.h"
#include <vector>
using namespace std;

class MyMap {
private:
    int W, H;
    vector<vector<MyCell>> data;
public:
    void init(int w, int h, int turn);
    int getW() const { return W; }
    int getH() const { return H; }
    const MyCell& at(int x, int y) const;
    void update(int x, int y, int turn, CellState state, bool self);
    int distance(int x1, int y1, int x2, int y2) const;
};
