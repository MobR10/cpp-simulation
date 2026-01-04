#pragma once

#include <utility>

struct Package{
    std::pair<size_t,size_t> client;
    size_t reward;
    size_t deadline;
    size_t firstTick;
    enum class Location{
        BASE,
        AGENT
    }location;
    size_t agentId = 0;

    Package();
    Package(std::pair<size_t,size_t> _client, size_t _reward, size_t _deadline, size_t _firstTick, Location _location = Location::BASE): 
    client(_client),
    reward(_reward),
    deadline(_deadline),
    firstTick(_firstTick),
    location(_location){};

    // std::pair<size_t,size_t> getCoordinates() const { return coordinates; }
    // size_t getReward() const { return reward; }
    // size_t getDeadline() const { return deadline; }
    // size_t getFirstTick() const { return firstTick; }

    // void setCoordinates(std::pair<size_t,size_t> _coordinates) { coordinates = _coordinates; }
    // void setReward(size_t _reward) { reward = _reward; }
    // void setDeadline(size_t _deadline) { deadline = _deadline; }
    // void setFirstTick(size_t _firstTick) { firstTick = _firstTick; }

};