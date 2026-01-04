#include "agents.h"
#include "../hivemind.h"
#include "../astar.h"

size_t Agent::numberOfAgents = 0;

void Agent::logMessage(const std::string& message){
    std::printf("Agent #%llu coords(%llu,%llu): %s\n",id,coordinates.first,coordinates.second,message.c_str());
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

bool Agent::hasPath() const{
    return !currentPath.empty();
}

bool Agent::canMove() const{
    return currentBattery >= consumption;
}

void Agent::tick(const std::vector<std::vector<Cell>>& map, HiveMind& hiveMind, int& profit, size_t currentTick, size_t& delivered, size_t& deadAgents, size_t& dropped){
    if (state == AgentState::DEAD)
        return;

    profit -= cost;

    if(state == AgentState::IDLE)
            takePackages();
    
    if (state != AgentState::MOVING && currentBattery < maxBattery) {
        currentBattery = std::min(currentBattery + static_cast<size_t>(maxBattery * 0.25),maxBattery);
        logMessage("Battery charged: ");
        std::cout<< currentBattery<< std::endl;
        return;
    }
    
    if (currentPath.empty()){
        decideNextPath(map, hiveMind);
    }

    if (!currentPath.empty()) {

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
                state = (cell == Cell::BASE) ? AgentState::IDLE : AgentState::CHARGING;

                if (cell == Cell::BASE)
                    takePackages();

                currentBattery = std::min(currentBattery + static_cast<size_t>(maxBattery * 0.25),maxBattery);
                logMessage("Battery charged: ");
                std::cout<< currentBattery<< std::endl;
                
                if(currentBattery < maxBattery){
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
        dropPackages(profit, dropped);
        logMessage("mf is deadass.");
    }
}

void Agent::decideNextPath(const std::vector<std::vector<Cell>>& map, HiveMind& hiveMind)
{   
    if (hasPackages()) {
        currentPath = aStar(map, coordinates, packages.front()->client, *this);
        logMessage("Assigning path to client");
    }
    else if (!at(hiveMind.getBaseCoords())) {
        currentPath = aStar(map, coordinates, hiveMind.getBaseCoords(), *this);
        logMessage("Assigning path to base");
    }
    else {
        state = AgentState::IDLE;
    }
}

bool Agent::isPathEmpty(){
    return currentPath.empty();
}

bool Agent::hasPackages(){
    for(auto& package: packages){
        if(package->location == Package::Location::AGENT)
            return true;
    }
    return false;
}

bool Agent::tryDelivery(int& profit, size_t currentTick,size_t& delivered){
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
        return true;
    }
    return false;
}

void Agent::dropPackages(int& profit,size_t& dropped){
    profit += undelivered * static_cast<int>(packages.size());
    dropped += packages.size();
}