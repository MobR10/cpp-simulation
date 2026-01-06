#include "agents.h"
#include "../hivemind.h"
#include "../pathfinding.h"
#include "../types.h"

size_t Agent::numberOfAgents = 0;

void Agent::logMessage(const std::string& message){
    std::printf("Agent #%llu coords(%llu,%llu), state %s: %s\n",id,coordinates.first,coordinates.second,agentStateToString[state].c_str(),message.c_str());
}

Agent::Agent(char _symbol,TerrainType _terrain, size_t _speed, size_t _maxBattery, size_t _consumption, size_t _cost, size_t _capacity):
symbol(_symbol),
terrain(_terrain),
speed(_speed),
maxBattery(_maxBattery),
consumption(_consumption),
cost(_cost),
capacity(_capacity)
{
    currentBattery = maxBattery;
    id = ++Agent::numberOfAgents;

}

void Agent::takePackages(){
    size_t packageCount = 0;
    for(auto& package: packages){
        if(package->location == Package::Location::BASE){
            package->location = Package::Location::AGENT;
            packageCount++;
        } 
    }
    if(packageCount > 0){
        logMessage("");
        std::printf("Picked %llu packages from base\n",packageCount);
    }
}

bool Agent::at(std::pair<size_t,size_t> _coordinates){
    return coordinates == _coordinates;
}

void Agent::tick(const std::vector<std::vector<Cell>>& map, HiveMind& hiveMind, int& profit, size_t currentTick, size_t& delivered, size_t& deadAgents, size_t& dropped){
    if (state == AgentState::DEAD)
        return;

    if(state != AgentState::IDLE)
        profit -= cost;

    if(coordinates == hiveMind.getBaseCoords())
        takePackages();
    
    // always charge fully whenever at a base or station
    if (state == AgentState::CHARGING && currentBattery < maxBattery) {
        currentBattery = std::min(currentBattery + static_cast<size_t>(maxBattery * 0.25),maxBattery);
        logMessage("Battery charged: ");
        std::cout<< currentBattery<< std::endl;
        return;
    }
    
    if(currentPath.empty()){
        decideNextPath(map, hiveMind);
    }

    if(state == AgentState::IDLE && !currentPath.empty()){
        profit -= cost;
    }

    if(!currentPath.empty() && currentBattery * 100 < 25 * maxBattery){
        currentPath = aStar(map,coordinates,packages.front()->client,*this);
        logMessage("Low battery, recalculating path to client");
    }

    if (!currentPath.empty()) {

        state = AgentState::MOVING;
        currentBattery -= consumption;
        logMessage("Battery consumed: ");
        std::cout<< currentBattery<< std::endl;

        size_t steps = 0;

        while (steps < speed && !currentPath.empty()) {
            state = AgentState::MOVING;
            coordinates = currentPath.front();
            currentPath.erase(currentPath.begin());
            steps++;

            Cell cell = map[coordinates.first][coordinates.second];

            if (cell == Cell::BASE || cell == Cell::STATION) {

                if (cell == Cell::BASE)
                    takePackages();

                currentBattery = std::min(currentBattery + static_cast<size_t>(maxBattery * 0.25),maxBattery);
                logMessage("Battery charged: ");
                std::cout<< currentBattery<< std::endl;
                
                if(currentBattery < maxBattery){
                    logMessage("stopped to charge.");
                    state = AgentState::CHARGING;
                    return;
                }
            }
        }

        tryDelivery(profit, currentTick, delivered);
    }

    if (currentBattery <= 0) {
        state = AgentState::DEAD;
        profit += deadAgent;
        deadAgents++;
        dropPackages(profit, dropped,hiveMind);
        logMessage("DEAD.");
    }
}

void Agent::decideNextPath(const std::vector<std::vector<Cell>>& map, HiveMind& hiveMind){   
    if (hasPackages()) {
        currentPath = aStar(map, coordinates, packages.front()->client, *this);
        logMessage("Assigning path to client");
    }
    else if (!at(hiveMind.getBaseCoords())) {
        currentPath = aStar(map, coordinates, hiveMind.getBaseCoords(), *this);
        logMessage("Assigning path to base");
    }
    else {
        state = (currentBattery < maxBattery) ? AgentState::CHARGING : AgentState::IDLE;
        // logMessage("No packages and inside base. No path assigned. Staying at base.");
    }
}

bool Agent::hasPackages(){
    for(auto& package: packages){
        if(package->location == Package::Location::AGENT)
            return true;
    }
    return false;
}

void Agent::tryDelivery(int& profit, size_t currentTick,size_t& delivered){
    if(!packages.empty()){
        if(coordinates == packages.front()->client){
            delivered++;
            if(currentTick - packages.front()->firstTick > packages.front()->deadline){
                profit += deliveredLate; 
                logMessage("Package arrived LATE.");
            }else logMessage("Package arrived IN TIME."); 
            // Remove the package from agent's list and set state to IDLE
            std::printf("REWARD: %llu - %d\n",packages.front()->reward,currentTick - packages.front()->firstTick > packages.front()->deadline ? (-deliveredLate) : 0);
            profit += packages.front()->reward;
            packages.erase(packages.begin()); 
        }
    }
}

void Agent::dropPackages(int& profit,size_t& dropped,HiveMind& hiveMind){
    for(size_t i = 0; i < packages.size(); i++){
        auto package = packages[i];
        if(package->location == Package::Location::AGENT){
            profit += undelivered;
            dropped++;
        }
        if(package->location == Package::Location::BASE){
            package->agentId = 0;
            hiveMind.getPackages().push_back(package);
            packages.erase(packages.begin() + i);
            i--;
        }
    }
}