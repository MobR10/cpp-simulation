#include "IMapGenerator.h"

// CONSTRUCTORS
MapGenerator::MapGenerator(): strategy(nullptr){}
MapGenerator::MapGenerator(IMapGenerator* _strategy): strategy(_strategy){}

// METHODS
void MapGenerator::setStrategy(IMapGenerator* _strategy){
    if(strategy) delete strategy;
    this->strategy = _strategy;
}

void MapGenerator::runStrategy(){
    this->strategy->load();
}

MapGenerator::~MapGenerator(){
    if(strategy) delete strategy;
}