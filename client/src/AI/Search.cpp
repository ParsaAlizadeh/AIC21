#include "Search.h"
#include <bits/stdc++.h>
using namespace std;

const int INF = 1e4;

Search::Search(const MyMap& mymap, int cx, int cy, bool danger, bool trap) :
    mymap(mymap),
    W(mymap.getW()),
    H(mymap.getH()),
    cx(cx),
    cy(cy),
    dist(W, vector<int>(H, -1)),
    nxt(W, vector<Direction>(H, CENTER))
{
    danger |= mymap.is_danger(cx, cy);
    int _dir[] = {1, 2, 3, 4};
    dist[cx][cy] = 0;
    queue<int> q;
    q.push(cx); q.push(cy);
    while (!q.empty()) {
        int ux = q.front(); q.pop();
        int uy = q.front(); q.pop();
        const MyCell& ucell = mymap.at(ux, uy);
        random_shuffle(_dir, _dir + 4);
        for (int i : _dir) {
            int vx = mymap.addmod(ux, dirx[i], W);
            int vy = mymap.addmod(uy, diry[i], H);
            const MyCell& vcell = mymap.at(vx, vy);
            if (vcell.get_state() == C_WALL)
                continue;
		    int cost = 1;
            if (!danger && mymap.is_danger(vx, vy))
                cost = 10 * INF;
		    if (!trap && vcell.get_state() == C_TRAP)
		        cost = INF;
		    if (vcell.get_state() == C_SWAMP)
		        cost = 4;
		    if (dist[vx][vy] != -1 && dist[vx][vy] <= dist[ux][uy] + cost)
		        continue;
            q.push(vx); q.push(vy);
            dist[vx][vy] = dist[ux][uy] + cost;
            nxt[vx][vy] = (dist[ux][uy] == 0 ? Direction(i) : nxt[ux][uy]);
        }
    }
}
