#pragma once
#include "Models/enums.h"
#include "MyMap.h"
#include <vector>
using namespace std;

const int dirx[] = {0, +1, 0, -1, 0};
const int diry[] = {0, 0, -1, 0, +1};

class Search {
private:
    const MyMap& mymap;
    int W, H;
    int cx, cy;
    vector<vector<int>> dist;
    vector<vector<Direction>> nxt;
public:
    Search(const MyMap& mymap, int cx, int cy, bool danger, bool trap);
    int get_dist(int x, int y) const { return dist[x][y]; }
    Direction to(int x, int y) const { return nxt[x][y]; }
};