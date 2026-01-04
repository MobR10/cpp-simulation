#pragma once

#include <vector>
#include <queue>
#include <memory>
#include <utility>
#include <random>
#include "types.h"
#include "agents/agents.h"
#include "agents/package.h"

class Agent;

class HiveMind{

    size_t 
    rowsN = 0,
    columnsN = 0,
    maxTicksN = 0,
    stationsN = 0,
    dronesN = 0,
    clientsN = 0,
    robotsN = 0,
    scootersN = 0,
    packagesN = 0,
    spawnFreqN = 0,
    agentsN = 0;

    std::vector<std::vector<Cell>> map;
    std::vector<std::pair<size_t,size_t>> clients;
    std::vector<std::unique_ptr<Agent>> agents;
    std::vector<std::shared_ptr<Package>> packages;
    size_t baseRow, baseCol;

    public:
        HiveMind();
        void loadSimulationFile();

        size_t getRowsN() const { return rowsN; }
        size_t getColumnsN() const { return columnsN; }
        size_t getMaxTicksN() const { return maxTicksN; }
        size_t getStationsN() const { return stationsN; }
        size_t getDronesN() const { return dronesN; }
        size_t getClientsN() const { return clientsN; }
        size_t getRobotsN() const { return robotsN; }
        size_t getScootersN() const { return scootersN; }
        size_t getPackagesN() const { return packagesN; }
        size_t getSpawnFreqN() const { return spawnFreqN; }
        size_t getAgentsN() const { return agentsN; }

        std::vector<std::shared_ptr<Package>>& getPackages() { return packages; }

        const std::vector<std::vector<Cell>>& getMap(){ return map; }
        const std::vector<std::pair<size_t,size_t>>& getClients(){ return clients; }
        const std::pair<size_t,size_t> getBaseCoords(){ return {baseRow, baseCol}; }
        std::vector<std::unique_ptr<Agent>>& getAgents() { return agents; }

        
        void setMap(std::vector<std::vector<Cell>> _map);
        void setClients(std::vector<std::pair<size_t,size_t>> _clients);
        void setBaseCoords(std::pair<size_t,size_t> _baseCoords);
        void setAgents(std::vector<std::unique_ptr<Agent>> _agents);

        std::pair<size_t,size_t> getRandomClient();

        void createRandomPackage(size_t tick);

        void assignNextPackage(Agent& agent);


        void printSimulationParameters();


};
