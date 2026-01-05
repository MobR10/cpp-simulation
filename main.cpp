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

    const double deltaTime = 0.00833; // 8.33 ms per tick (~120 FPS)

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

        for(size_t i = 0; i < hiveMind.getPackages().size(); i++){
            hiveMind.decidePackageAssignment();
        }

        for (auto& agent : agents){
            if(agent->getState() == AgentState::DEAD)
                continue;
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
    
    // check if the simulation ran out of ticks time
    // if positive, then reset the assigned packages

    for(auto& agent : agents){
        if(agent->getState() == AgentState::DEAD)
            continue;
        for(size_t i = 0; i < agent->getPackages().size(); i++){
            auto package = agent->getPackages().at(i);
            if(package->location == Package::Location::BASE){
                package->agentId = 0;
                hiveMind.getPackages().push_back(package);
                agent->getPackages().erase(agent->getPackages().begin() + i);
                i--;
            }
        }
    }

    std::FILE* resultFile = std::fopen("simulation.txt","w");

    std::fprintf(resultFile,"Delivered packages: %llu\n",delivered);
    std::printf("Delivered packages: %llu\n",delivered);

    std::fprintf(resultFile,"Dropped packages: %llu\n",dropped);
    std::printf("Dropped packages: %llu\n",dropped);

    bool printDeadAgents = false;
    bool printAliveAgents = false;

    for(auto& agent : agents){
        if(agent->getState() == AgentState::DEAD){
            if(!printDeadAgents){
                fprintf(resultFile,"Dead Agents:\n");
                std::cout<<"Dead Agents:\n";
                printDeadAgents = true;
            }

            fprintf(resultFile,"Agent#%llu (%s) at (%llu,%llu)",
                    agent->getId(),
                    agent->getName().c_str(),
                    agent->getCoordinates().first,
                    agent->getCoordinates().second);
            std::printf("Agent#%llu (%s) at (%llu,%llu)",agent->getId(),
                    agent->getName().c_str(),
                    agent->getCoordinates().first,
                    agent->getCoordinates().second);

            if(agent->getPackages().size() > 0){
                fprintf(resultFile," has %llu undelivered packages.\n",agent->getPackages().size());
                std::printf(" has %llu undelivered packages.\n",agent->getPackages().size());
            }else{
                fprintf(resultFile," has no undelivered packages.\n");
                std::printf(" has no undelivered packages.\n");
            }
        }
        else{
            if(!printAliveAgents){
                fprintf(resultFile,"Alive Agents:\n");
                std::cout<<"Alive Agents:\n";
                printAliveAgents = true;
            }
            fprintf(resultFile,"Agent#%llu (%s) at (%llu,%llu)",
                    agent->getId(),
                    agent->getName().c_str(),
                    agent->getCoordinates().first,
                    agent->getCoordinates().second);
            std::printf("Agent#%llu (%s) at (%llu,%llu)",agent->getId(),
                    agent->getName().c_str(),
                    agent->getCoordinates().first,
                    agent->getCoordinates().second);

            if(agent->getPackages().size() > 0){
                fprintf(resultFile," has %llu undelivered packages.\n",agent->getPackages().size());
                std::printf(" has %llu undelivered packages.\n",agent->getPackages().size());
            }else{
                fprintf(resultFile," has no undelivered packages.\n");
                std::printf(" has no undelivered packages.\n");
            }
        }
    }

    if(hiveMind.getPackages().size() > 0){
        fprintf(resultFile,"Found %llu undelivered packages at the base.\n",hiveMind.getPackages().size());
        std::printf("Found %llu undelivered packages at base.\n",hiveMind.getPackages().size());
        profit += undelivered * static_cast<int>(hiveMind.getPackages().size());
    }

    std::fprintf(resultFile,"Final Profit: %d\n",profit);
    std::fclose(resultFile);
    std::cout<<"Profit: "<< profit << std::endl;
}