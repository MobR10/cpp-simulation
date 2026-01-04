#pragma once

#include <vector>
#include <utility>

#include "types.h"
#include "agents/agents.h"
typedef std::pair<size_t,size_t> Pair;

std::vector<Pair> aStar(const std::vector<std::vector<Cell>>& map, Pair start, Pair end, Agent& agent);