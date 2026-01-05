#include <vector>
#include <queue>
#include <cmath>
#include <limits>
#include <algorithm>

#include "types.h"
#include "agents/agents.h"
#include "astar.h"

struct Node {
    Pair coord;
    int g, h, f;
    Pair parent;
};

inline int heuristic(Pair a, Pair b) {
    // Manhattan distance
    return std::abs((int)a.first - (int)b.first) + std::abs((int)a.second - (int)b.second);
}

inline bool isValid(Pair c, size_t rows, size_t cols) {
    return c.first < rows && c.second < cols;
}

constexpr int ROAD_COST = 10;
constexpr int CLIENT_COST = 6;
constexpr int STATION_HIGH_COST = 30; 
constexpr int STATION_LOW_COST  = 2;

inline int getG(Cell cell, size_t currentBattery, size_t maxBattery) {
    if (cell == Cell::STATION || cell == Cell::BASE) {
        // the condition is the same with currentBattery*100 / maxBattery <= 10, but it is okay for size_t and bug-free
        if (currentBattery * 100 <= 25 * maxBattery) {
            return STATION_LOW_COST;
        } else {
            return STATION_HIGH_COST;
        }
    }

    if (cell == Cell::CLIENT)
        return CLIENT_COST;

    return ROAD_COST;
}


inline bool isPassable(const std::vector<std::vector<Cell>>& map, Pair c,Agent& agent) {
    if(agent.getTerrain() == TerrainType::AIR)
        return true;
    else
        return map[c.first][c.second] != Cell::WALL;
}

std::vector<Pair> aStar(const std::vector<std::vector<Cell>>& map, Pair start, Pair end, Agent& agent) {
    if(start == end){
        return {start};
    }
    size_t rows = map.size();
    size_t cols = map[0].size();

    std::vector<std::vector<bool>> closed(rows, std::vector<bool>(cols, false));
    std::vector<std::vector<Pair>> parents(rows, std::vector<Pair>(cols, {-1,-1}));

    struct PQNode { Pair coord; int f; };
    auto cmp = [](const PQNode &a, const PQNode &b){ return a.f > b.f; };
    std::priority_queue<PQNode, std::vector<PQNode>, decltype(cmp)> open(cmp);
    

    std::vector<std::vector<int>> g(rows, std::vector<int>(cols, std::numeric_limits<int>::max()));

    g[start.first][start.second] = 0;
    open.push({start, heuristic(start,end)});
    parents[start.first][start.second] = start;

    std::vector<Pair> directions = {{-1,0},{1,0},{0,-1},{0,1}}; // N, S, W, E

    while(!open.empty()) {
        Pair curr = open.top().coord;
        open.pop();

        if(curr == end) {
            // reconstruct path
            std::vector<Pair> path;
            Pair p = end;
            while(p != start) {
                path.push_back(p);
                p = parents[p.first][p.second];
            }
            std::reverse(path.begin(), path.end());
            return path;
        }

        if(closed[curr.first][curr.second]) continue;
        closed[curr.first][curr.second] = true;

        for(Pair d : directions) {
            int ni = curr.first + d.first;
            int nj = curr.second + d.second;
            Pair neighbor = {ni, nj};

            if(!isValid(neighbor, rows, cols) || !isPassable(map, neighbor,agent) || closed[ni][nj])
                continue;

            int tentativeG = g[curr.first][curr.second] + getG(map[ni][nj],agent.getCurrentBattery(),agent.getMaxBattery());
            if(tentativeG < g[ni][nj]) {
                g[ni][nj] = tentativeG;
                int f = tentativeG + heuristic(neighbor, end);
                open.push({neighbor, f});
                parents[ni][nj] = curr;
            }
        }
    }

    return {};
}

std::pair<int,int> bfsDistance(const std::vector<std::vector<Cell>>& map, Pair start, Pair end, Agent& agent){
    const size_t rows = map.size();
    const size_t cols = map[0].size();

    std::vector<std::vector<bool>> visited(rows, std::vector<bool>(cols, false));
    std::queue<std::pair<Pair, int>> q; // ((x,y), distance)

    int stationDensityHint = 0;

    visited[start.first][start.second] = true;
    q.push({start, 0});

    std::vector<Pair> directions = {
        {1,0}, {-1,0}, {0,1}, {0,-1}
    };

    while (!q.empty()) {
        auto [c, dist] = q.front();
        q.pop();

        if (c == end) {
            return {dist,stationDensityHint};
        }

        for (auto d : directions) {
            int ni = (int)c.first + d.first;
            int nj = (int)c.second + d.second;

            if (ni < 0 || nj < 0)
                continue;

            Pair next = {(size_t)ni, (size_t)nj};

            if (!isValid(next, rows, cols))
                continue;

            if (!isPassable(map, next, agent))
                continue;

            if (!visited[next.first][next.second]) {
                visited[next.first][next.second] = true;
                if(map[next.first][next.second] == Cell::STATION || map[next.first][next.second] == Cell::BASE)
                    stationDensityHint++;
                q.push({next, dist + 1});
            }
        }
    }

    return {-1,0};
}