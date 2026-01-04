#include "hivemind.h"
#include "types.h"

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

void HiveMind::loadSimulationFile(){
    std::ifstream fin(simulationFile);
    if(!fin.is_open()){
        std::cerr<<"Couln't open the file " << simulationFile << "\n";
        return;
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

    // ADD AGENTS TO A LIST
    agentsN = dronesN + scootersN + robotsN;

    for(size_t i = 0; i < dronesN; i++)
        agents.push_back(std::make_unique<Drone>());
        
    for(size_t i = 0; i < robotsN; i++)
        agents.push_back(std::make_unique<Robot>());

    for(size_t i = 0; i < scootersN; i++)
        agents.push_back(std::make_unique<Scooter>());
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