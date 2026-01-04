#include "agents.h"
#include "../types.h"

#include <iostream>

Scooter::Scooter(): Agent('S',TerrainType::GROUND,2,200,5,4,2) {
    name = "SCOOTER";
}
