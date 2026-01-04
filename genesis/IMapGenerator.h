#pragma once

#include "../hivemind.h"
#include "../types.h"

#include <fstream>
#include <string>
#include <vector>

// ========= STRATEGY INTERFACE =========
class IMapGenerator{

    public:
    virtual void load() = 0;
    virtual ~IMapGenerator() = default;
};

// ========= STRATEGY CONTEXT =========
class MapGenerator{
    IMapGenerator *strategy;

    public:
    MapGenerator();
    MapGenerator(IMapGenerator* _strategy);
    ~MapGenerator();
    void setStrategy(IMapGenerator* strategy);
    void runStrategy();
};

// ========= STRATEGIES =========

// ========= FILE MAP LOADER =========
class FileMapLoader: public IMapGenerator{

    public:
    FileMapLoader();
    void load();
};

// ========= ProceduralMapGenerator =========
class ProceduralMapGenerator: public IMapGenerator{

    private:
    HiveMind& hiveMind;

    public:
        ProceduralMapGenerator(HiveMind& _hiveMind);
        void load();
        bool isMapValid(std::vector<std::vector<Cell>>& map);
        
};
