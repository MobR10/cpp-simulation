#include "genesis/IMapGenerator.h"
#include "agents/agents.h"
#include "astar.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <memory>
#include <utility>

#include <chrono>
#include <thread>

using namespace std;

void print(std::vector<std::pair<size_t,size_t>>& path, size_t id){
    cout<<"Path for agent #" << id << ": ";
    for(size_t i=0; i < path.size(); i++){
        printf("(%llu,%llu) -> ",path.at(i).first,path.at(i).second);
    }
}

#define debug 1

const AgentState CHARGING = AgentState::CHARGING;
const AgentState DEAD = AgentState::DEAD;
const AgentState IDLE = AgentState::IDLE;
const AgentState MOVING = AgentState::MOVING;

int main(){
   
    HiveMind hiveMind;
    hiveMind.loadSimulationFile();

    MapGenerator generator(new ProceduralMapGenerator(hiveMind));
    generator.runStrategy();

    const std::vector<std::vector<Cell>>& map = hiveMind.getMap();
    const std::vector<std::pair<size_t,size_t>>& clients = hiveMind.getClients();
    const std::pair<size_t,size_t> baseCoords = hiveMind.getBaseCoords();
    
    std::vector<std::unique_ptr<Agent>>& agents = hiveMind.getAgents();

    #if debug == 1
    cout<<"Number of packages is: " << hiveMind.getPackagesN() << endl;
    cout <<"Number of clients is: " << clients.size() << endl << endl;
    cout <<"Clients are : ";
    for(size_t i = 0; i < clients.size(); i ++)
        cout<< '(' << clients.at(i).first << ',' << clients.at(i).second << "), ";
    cout << endl << endl;
    cout <<"Base coords: " << baseCoords.first << "," << baseCoords.second << endl << endl;
    #endif

    const double deltaTime = 0.00833; // 16 ms per tick (~60 FPS)

    int profit = 0;
    size_t deadAgents = 0;

    size_t spawnedPackages = 0;
    size_t delivered = 0;
    size_t dropped = 0;

    for (size_t tick = 1; tick <= hiveMind.getMaxTicksN() && delivered + dropped < hiveMind.getPackagesN() && deadAgents < hiveMind.getAgentsN(); tick++) {
        std::cout<<"Tick number "<< tick << std::endl;
        auto start = std::chrono::high_resolution_clock::now();

        if(tick % hiveMind.getSpawnFreqN() == 0 && spawnedPackages < hiveMind.getPackagesN()){
            hiveMind.createRandomPackage(tick);
            spawnedPackages++;
        }

        for (auto& agent : agents){
            if(agent->getState() == AgentState::DEAD)
                continue;

            hiveMind.assignNextPackage(*agent);
        
            agent->tick(map,hiveMind,profit,tick,delivered,deadAgents,dropped);
        }

        cout<<"Profit: " << profit << endl;

        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = end - start;
            
        if (elapsed.count() < deltaTime) {
            std::this_thread::sleep_for(std::chrono::duration<double>(deltaTime - elapsed.count()));
        }
        std::cout<< std::endl;
    }

    for(auto& agent : agents){
        if(agent->getPackages().size() > 0){
                std::printf("Found %llu undelivered packages on a %s.\n",agent->getPackages().size(),agent->getName().c_str());
            }
        if(agent->getState() == AgentState::DEAD){
            std::printf("Found a dead %s\n",agent->getName().c_str());
        }
    }

    std::cout<<"Profit: "<< profit << std::endl;
}