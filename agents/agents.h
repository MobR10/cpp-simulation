#pragma once

#include "../types.h"
#include "../hivemind.h"
#include "package.h"

#include <string>
#include <vector>
#include <memory>
#include <iostream>
#include <queue>

// class IAgent{
//     public:
//         virtual void move() = 0;
//         virtual ~IAgent(){};
// };

class HiveMind;

class Agent{
    protected:
        std::string name;
        static size_t numberOfAgents;
        size_t id;
        bool targetBase = false;
        std::pair<size_t,size_t> coordinates;
        AgentState state = AgentState::IDLE;
        char symbol;
        TerrainType terrain;
        size_t speed, maxBattery,currentBattery, consumption, cost, capacity;
        std::vector<std::shared_ptr<Package>> packages;
        std::vector<std::pair<size_t,size_t>> currentPath;

        void logMessage(const std::string& message);
    public:
        Agent(char _symbol,TerrainType _terrain, size_t _speed, size_t _maxBattery, size_t _consumption, size_t _cost, size_t _capacity);
        virtual void tick(const std::vector<std::vector<Cell>>& map, HiveMind& HiveMind,int& profit, size_t currentTick, size_t& delivered, size_t& deadAgents, size_t& dropped);
        void decideNextPath(const std::vector<std::vector<Cell>>& map, HiveMind& hiveMind);
        bool tryDelivery(int& profit, size_t currentTick,size_t& delivered);
        void dropPackages(int& profit,size_t& dropped);
        virtual ~Agent(){};

        void takePackages();

        std::vector<std::shared_ptr<Package>>& getPackages() { return packages; }

        // Getters
        std::string getName() const { return name; }
        char getSymbol() const { return symbol; }
        TerrainType getTerrain() const { return terrain; }
        size_t getId() const { return id; }
        size_t getSpeed() const { return speed; }
        size_t getMaxBattery() const { return maxBattery; }
        size_t getCurrentBattery() const { return currentBattery; }
        size_t getConsumption() const { return consumption; }
        size_t getCost() const { return cost; }
        size_t getCapacity() const { return capacity; }
        std::pair<size_t,size_t> getCoordinates() { return coordinates; }
        AgentState getState() const { return state; }
        const std::vector<std::pair<size_t,size_t>>& getCurrentPath() const { return currentPath; }
        bool getTargetBase() const { return targetBase; }

        // Setters
        void setSymbol(char s) { symbol = s; }
        void setTerrain(TerrainType t) { terrain = t; }
        void setSpeed(size_t s) { speed = s; }
        void setMaxBattery(size_t m) { maxBattery = m; }
        void setCurrentBattery(size_t c) { currentBattery = c; }
        void setConsumption(size_t c) { consumption = c; }
        void setCost(size_t c) { cost = c; }
        void setCoordinates(std::pair<size_t,size_t> _coordinates) { coordinates = _coordinates; }
        void setState(AgentState _state) { state = _state; }

        void setCurrentPath(std::vector<std::pair<size_t,size_t>> _path) { currentPath = _path; }

        bool at(std::pair<size_t,size_t> _coordinates);

        bool canMove() const;

        bool hasPath() const;

        void setTargetBase(bool goToBase) { targetBase = goToBase ; };

        bool isPathEmpty();

        bool hasPackages();
};

// class AgentOperator{
//     private:
//         IAgent * agent;

//     public:
//         AgentOperator();
//         AgentOperator(IAgent* _agent);
//         void runAgent();
//         void setAgentType(IAgent* _agent);
//         ~AgentOperator();
// };

class Drone: /*public IAgent,*/ public Agent{

    public:
        Drone();
        // void move();
};

class Robot: /*public IAgent,*/ public Agent{

    public:
        Robot();
        // void move();
};

class Scooter: /*public IAgent,*/ public Agent{
    
    public:
        Scooter();
        // void move();
};