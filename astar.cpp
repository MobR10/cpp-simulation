#include <vector>
#include <queue>
#include <cmath>
#include <limits>
#include <algorithm>

#include "types.h"
#include "agents/agents.h"
#include "astar.h"

struct State {
    Pair coord;
    int battery;
};

struct Node {
    // Pair coord;
    // int g, f;
    // int h;
    // Pair parent;

     State state;
    int g;
    int f;
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
        if (currentBattery * 100 <= 50 * maxBattery) {
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

std::vector<Pair> aStar(const std::vector<std::vector<Cell>>& map, Pair start, Pair end, Agent& agent){
    const size_t rows = map.size();
    const size_t cols = map[0].size();

    const int maxBattery = (int)agent.getMaxBattery();
    const int consumption = (int)agent.getConsumption();
    const int recharge = (int)(0.25 * maxBattery);

    // dist[x][y][battery] = best g
    std::vector dist(rows,
        std::vector(cols,
            std::vector<int>(maxBattery + 1, std::numeric_limits<int>::max())
        )
    );

    // parent[x][y][battery] = previous state
    struct Parent {
        Pair coord;
        int battery;
    };

    std::vector parent(rows,
        std::vector(cols,
            std::vector<Parent>(maxBattery + 1, {{0,0}, -1})
        )
    );

    auto cmp = [](const Node& a, const Node& b) {
        return a.f > b.f;
    };

    std::priority_queue<Node, std::vector<Node>, decltype(cmp)> open(cmp);

    State startState{start, (int)agent.getCurrentBattery()};
    dist[start.first][start.second][startState.battery] = 0;

    open.push({
        startState,
        0,
        heuristic(start, end)
    });

    std::vector<Pair> directions = {
        {1,0}, {-1,0}, {0,1}, {0,-1}
    };

    while (!open.empty()) {
        Node curr = open.top();
        open.pop();

        Pair c = curr.state.coord;
        int b = curr.state.battery;

        // Goal reached
        if (c == end && b > 0) {
            std::vector<Pair> path;
            Pair p = c;
            int pb = b;

            while (!(p == start && pb == startState.battery)) {
                path.push_back(p);
                Parent pr = parent[p.first][p.second][pb];
                p = pr.coord;
                pb = pr.battery;
            }

            std::reverse(path.begin(), path.end());
            return path;
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

            int nextBattery = b - consumption;
            if (nextBattery <= 0)
                continue;

            Cell cell = map[next.first][next.second];
            if (cell == Cell::BASE || cell == Cell::STATION) {
                nextBattery = std::min(maxBattery, nextBattery + recharge);
            }

            int nextG = curr.g + 1;

            if (nextG < dist[next.first][next.second][nextBattery]) {
                dist[next.first][next.second][nextBattery] = nextG;
                parent[next.first][next.second][nextBattery] = {c, b};

                int f = nextG + heuristic(next, end);
                open.push({
                    {next, nextBattery},
                    nextG,
                    f
                });
            }
        }
    }

    return {}; // no feasible path
}
