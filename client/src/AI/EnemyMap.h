#pragma once
#include <vector>
#include <string>
using std::vector;
using std::string;

class EnemyMap {
private:
    int W, H;
    vector<vector<int>> data;
public:
    EnemyMap();
    void init(int w, int h);
    int getW() const { return W; }
    int getH() const { return H; }
    int at(int x, int y) const { return data[x][y]; }
    void update(int x, int y, int turn);

    static int size;
    void decode(string s, int turn);
    string encode(int x, int y, int turn) const;
};