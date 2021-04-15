#include "Search.h"
#include <bits/stdc++.h>
using namespace std;

Search::Search(const MyMap& mymap, int cx, int cy, bool danger) :
    mymap(mymap),
    W(mymap.getW()),
    H(mymap.getH()),
    cx(cx),
    cy(cy),
    dist(W, vector<int>(H, -1)),
    nxt(W, vector<Direction>(H, CENTER))
{
    // danger |= mymap.is_danger(cx, cy);
    int _dir[] = {1, 2, 3, 4};
    dist[cx][cy] = 0;
    queue<int> q;
    q.push(cx); q.push(cy);
    while (!q.empty()) {
        int ux = q.front(); q.pop();
        int uy = q.front(); q.pop();
        auto ucell = mymap.at(ux, uy);
        random_shuffle(_dir, _dir + 4);
        for (int i : _dir) {
            int vx = mymap.addmod(ux, dirx[i], W);
            int vy = mymap.addmod(uy, diry[i], H);
            if (dist[vx][vy] >= 0)
                continue;
            if (!danger && mymap.is_danger(vx, vy))
                continue;
            auto vcell = mymap.at(vx, vy);
            if (vcell.get_state() == C_WALL)
                continue;
            q.push(vx); q.push(vy);
            dist[vx][vy] = dist[ux][uy] + 1;
            nxt[vx][vy] = (dist[ux][uy] == 0 ? Direction(i) : nxt[ux][uy]);
        }
    }
}