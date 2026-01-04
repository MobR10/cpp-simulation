#include "agents.h"
#include "../types.h"

#include <iostream>

Robot::Robot(): Agent('R',TerrainType::GROUND,1,300,2,1,4) {
    name = "ROBOT";
}
