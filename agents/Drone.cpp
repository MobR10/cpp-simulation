#include "agents.h"
#include "../types.h"

#include <iostream>

Drone::Drone(): Agent('^',TerrainType::AIR,3,100,10,15,1){
    name = "DRONE";
}
