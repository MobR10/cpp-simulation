#include "hivemind.h"
#include "types.h"
#include "pathfinding.h"

#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <utility>

template<typename t>
t getRandomNumber(t start, t end){
    static std::random_device rd;   // seed
    static std::mt19937 gen(rd());  // mersenne twister engine
    std::uniform_int_distribution<t> dist(start , end);
    return dist(gen);
}

template<typename t>
void readField(std::ifstream& buff,t& field){
    std::string line = "" , label;
    std::getline(buff,line);
    std::istringstream iss(line);
    iss>> label >> field;
}

HiveMind::HiveMind(){}

bool HiveMind::loadSimulationFile(){
    std::ifstream fin(simulationFile);
    if(!fin.is_open()){
        std::cerr<<"Couln't open the file " << simulationFile << "\n";
        return false;
    }
    
    // rows and columns are hardcoded because MAP_SIZE is followed by 2 values
    std::string line = "", label;
    std::getline(fin,line);
    
    std::istringstream iss(line);
    iss >> label >> rowsN >> columnsN;

    readField<size_t>(fin,maxTicksN);
    readField<size_t>(fin,stationsN);
    readField<size_t>(fin,clientsN);
    readField<size_t>(fin,dronesN);
    readField<size_t>(fin,robotsN);
    readField<size_t>(fin,scootersN);
    readField<size_t>(fin,packagesN);
    readField<size_t>(fin,spawnFreqN);

    fin.close();

    if(dronesN + robotsN + scootersN == 0){
        std::cerr<<"No agents specified in the simulation file!\n";
        return false;
    }
    if(maxTicksN == 0){
        std::cerr<<"Max ticks cannot be zero!\n";
        return false;
    }
    if(packagesN == 0){
        std::cerr<<"No packages to deliver specified in the simulation file!\n";
        return false;
    }
    if(spawnFreqN == 0){
        std::cerr<<"Spawn frequency cannot be zero!\n";
        return false;
    }
    if(clientsN == 0){
        std::cerr<<"No clients specified in the simulation file!\n";
        return false;
    }

    // ADD AGENTS TO A LIST
    agentsN = dronesN + scootersN + robotsN;

    for(size_t i = 0; i < dronesN; i++)
        agents.push_back(std::make_unique<Drone>());
        
    for(size_t i = 0; i < robotsN; i++)
        agents.push_back(std::make_unique<Robot>());

    for(size_t i = 0; i < scootersN; i++)
        agents.push_back(std::make_unique<Scooter>());

    return 1;
}

void HiveMind::setMap(std::vector<std::vector<Cell>> _map){
    map = _map;
}

void HiveMind::setClients(std::vector<std::pair<size_t,size_t>> _clients){
    clients = _clients;
}

void HiveMind::setBaseCoords(std::pair<size_t,size_t> _baseCoords){
    baseRow = _baseCoords.first;
    baseCol = _baseCoords.second;
}

void HiveMind::setAgents(std::vector<std::unique_ptr<Agent>> _agents){
    agents = std::move(_agents);
}

std::pair<size_t,size_t> HiveMind::getRandomClient() {
    return clients[getRandomNumber<int>(0,static_cast<int>(clients.size()-1))];
}

void HiveMind::createRandomPackage(size_t tick){
    int randomReward = getRandomNumber<int>(200,800);
    size_t randomDeadline = getRandomNumber<size_t>(10,20);
    std::pair<size_t,size_t> randomClient = getRandomClient();
    std::shared_ptr<Package> pkg = std::make_shared<Package>(randomClient,randomReward,randomDeadline,tick);
    packages.push_back(pkg);

    std::printf("Package created: client(%llu,%llu), reward(%d), deadline(%llu), firstTick(%llu), location(BASE)\n",
        randomClient.first,
        randomClient.second,
        randomReward,
        randomDeadline,
        tick);
}

void HiveMind::assignNextPackage(Agent& agent) {

    if(packages.empty())
        return;

    // Agent dead → no assignment
    if (agent.getState() == AgentState::DEAD)
        return;

    // Agent full → cannot take more packages
    if (agent.getPackages().size() >= agent.getCapacity())
        return;

    std::shared_ptr<Package> packagePtr = packages.front();
    packages.erase(packages.begin());

    packagePtr->agentId = agent.getId();
    agent.getPackages().push_back(packagePtr);

    Package &pkg = *packagePtr;
    // log
    std::printf("Package assigned to agent#%llu at (%llu,%llu) (reward=%llu, deadline=%llu, client=(%llu,%llu))\n",
                pkg.agentId,
                agent.getCoordinates().first,
                agent.getCoordinates().second,
                pkg.reward,
                pkg.deadline,
                pkg.client.first,
                pkg.client.second);
}

constexpr int DIST_WEIGHT = 10;       // weight for distance
constexpr int STATION_WEIGHT = 5;     // weight for recharge stations

void HiveMind::decidePackageAssignment() {
    if(packages.empty())
        return;
        
    int minCost = INT_MAX;
    Agent* selectedAgent = nullptr;

    for (auto& agent : agents) {
        if (agent->getState() == AgentState::DEAD)
            continue;

        int cost = 0;
        int stationCount = 0;
        size_t batteryLeft = agent->getCurrentBattery();
        std::pair<size_t, size_t> agentCoords = agent->getCoordinates();

        // Consider packages already in agent's possession
        if (agent->hasPackages() && agent->getPackages().size() < agent->getCapacity()) {
            for (auto& package : agent->getPackages()) {
                if (package->location == Package::Location::AGENT) {
                    auto [dist, stations] = bfsDistance(map, agentCoords, package->client, *agent);

                    // Ticks needed related to agent's speed
                    int ticksNeeded = static_cast<int>(std::ceil(float(dist) / agent->getSpeed()));

                    // Reduce battery for this path
                    if (batteryLeft < ticksNeeded * agent->getConsumption()) {
                        // Agent needs to stop at stations along the way
                        batteryLeft = agent->getMaxBattery();
                        cost += (ticksNeeded + stations) * DIST_WEIGHT; // extra cost for recharging delay
                    } else {
                        batteryLeft -= ticksNeeded * agent->getConsumption();
                        cost += ticksNeeded * DIST_WEIGHT;
                    }

                    stationCount += stations;
                    agentCoords = package->client;
                }
            }

            // Return to base to pick up new package
            auto [dist, stations] = bfsDistance(map, agentCoords, getBaseCoords(), *agent);
            int ticksNeeded = static_cast<int>(std::ceil(float(dist) / agent->getSpeed()));
            if (batteryLeft < ticksNeeded * agent->getConsumption()) {
                batteryLeft = agent->getMaxBattery();
                cost += (ticksNeeded + stations) * DIST_WEIGHT;
            } else {
                batteryLeft -= ticksNeeded * agent->getConsumption();
                cost += ticksNeeded * DIST_WEIGHT;
            }
            stationCount += stations;
            agentCoords = getBaseCoords();
        }

        // Now consider the new package at base
        auto [dist, stations] = bfsDistance(map, agentCoords, packages.front()->client, *agent);
        int ticksNeeded = static_cast<int>(std::ceil(float(dist) / agent->getSpeed()));
        if (batteryLeft < ticksNeeded * agent->getConsumption()) {
            batteryLeft = agent->getMaxBattery();
            cost += (ticksNeeded + stations) * DIST_WEIGHT;
        } else {
            batteryLeft -= ticksNeeded * agent->getConsumption();
            cost += ticksNeeded * DIST_WEIGHT;
        }
        stationCount += stations;

        // Prefer paths with recharge stations
        cost -= stationCount * STATION_WEIGHT;

        if (cost < minCost) {
            minCost = cost;
            selectedAgent = agent.get();
        }
    }

    if (selectedAgent != nullptr)
        assignNextPackage(*selectedAgent);
}


void HiveMind::printSimulationParameters(){
    std::cout<< "Map size: " << rowsN << " " << columnsN << std::endl;
    std::cout<< "Max ticks: " << maxTicksN << std::endl;
    std::cout<< "Max stations: " << stationsN << std::endl;
    std::cout<< "Clients: " << clientsN << std::endl;
    std::cout<< "Drones : " << dronesN << std::endl;
    std::cout<< "Robots: " << robotsN << std::endl;
    std::cout<< "Scooters: " << scootersN << std::endl;
    std::cout<< "Total packages: " << packagesN << std::endl;
    std::cout<< "Spawn frequency: " << spawnFreqN << std::endl << std::endl;
}