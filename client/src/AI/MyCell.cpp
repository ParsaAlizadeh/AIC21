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