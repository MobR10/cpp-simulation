#pragma once

#include <unordered_map>
#include <string>


constexpr int deadAgent = -500;
constexpr int undelivered = -200;
constexpr int deliveredLate = -50;

enum class AgentState{
    IDLE,
    MOVING,
    CHARGING,
    DEAD
};

enum class Cell{
    ROAD,
    WALL,
    BASE,
    STATION,
    CLIENT
};

inline std::unordered_map<Cell,char> cellChar = {
        {Cell::ROAD,'.'},
        {Cell::WALL,'#'},
        {Cell::BASE,'B'},
        {Cell::STATION,'S'},
        {Cell::CLIENT,'D'}
};

enum class SimulationParameterKeys{
    ROWS,
    COLUMNS,
    MAX_TICKS,
    MAX_STATIONS,
    CLIENTS_COUNT,
    DRONES,
    ROBOTS,
    SCOOTERS,
    TOTAL_PACKAGES,
    SPAWN_FREQUENCY
};

enum class TerrainType{
    AIR,
    GROUND
};

inline std::unordered_map<TerrainType,std::string> terrainToString ={
    {TerrainType::AIR,"AER"},
    {TerrainType::GROUND,"PAMANT"}
};

inline std::unordered_map<AgentState,std::string> agentStateToString ={
    {AgentState::IDLE,"IDLE"},
    {AgentState::MOVING,"MOVING"},
    {AgentState::CHARGING,"CHARGING"},
    {AgentState::DEAD,"DEAD"}
};

#define mapFileName "map.txt"
#define simulationFile "simulation_setup.txt"