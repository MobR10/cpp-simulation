#include "IMapGenerator.h"
#include "../types.h"
#include "../hivemind.h"
#include "../agents/agents.h"

#include <iostream>
#include <vector>
#include <memory>
#include <random>
#include <algorithm>
#include <queue>

ProceduralMapGenerator::ProceduralMapGenerator(HiveMind& _hiveMind): hiveMind(_hiveMind){}

void ProceduralMapGenerator::load(){

    std::vector<std::vector<Cell>> map;
    size_t iterations = 0;
    size_t rows = hiveMind.getRowsN();
    size_t cols = hiveMind.getColumnsN();

    std::cout<< "Generating map:" << std::endl;
    do{
    iterations ++;
    // std::cout<< "Try #"<< iterations << std::endl;
    std::vector<Cell> cells;

    // 1 base
    cells.push_back(Cell::BASE);

    // Stations
    for(size_t i = 0; i < hiveMind.getStationsN(); i++)
    cells.push_back(Cell::STATION);

    // Clients
    for(size_t i = 0; i < hiveMind.getClientsN(); i++)
    cells.push_back(Cell::CLIENT);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(0, 1); // 0 = ROAD, 1 = WALL

    while(cells.size() < size_t(rows * cols)) {
        cells.push_back(dist(gen) == 0 ? Cell::ROAD : Cell::WALL);
    }

    std::shuffle(cells.begin(), cells.end(), gen);
    
    map = std::vector<std::vector<Cell>>(rows, std::vector<Cell>(cols));

    for(size_t i = 0; i < rows; i++) {
        for(size_t j = 0; j < cols; j++) {
            map[i][j] = cells[i * cols + j];
        }
    }
    }while(!isMapValid(map));

    std::cout<< "Valid on try #"<< iterations << std::endl;

    // save map
    hiveMind.setMap(map);

    // save base and clients coordinates
    // also initialize each agent's coordinates to the base
    std::vector<std::pair<size_t,size_t>> clients;

    for(size_t i = 0; i < rows; i++)
        for(size_t j = 0; j < cols; j++)
            if(map[i][j] == Cell::BASE){
                hiveMind.setBaseCoords({i,j});

                // initialize agents coords
                for(size_t k = 0; k < hiveMind.getAgents().size(); k++)
                    hiveMind.getAgents().at(k)->setCoordinates({i,j});
            }
            else if(map[i][j] == Cell::CLIENT){
                    clients.push_back({i,j});
            }
    hiveMind.setClients(clients);
    
    std::ofstream fout(mapFileName);

    if(!fout.is_open()){
         std::cerr<<"Couln't create the file " << mapFileName << "\n";
         return;
    }
    
    for(size_t i = 0; i < rows; i++) {
        for(size_t j = 0; j < cols; j++) {
            fout<<cellChar.at(map[i][j]);
            if(j < cols-1)
                fout<<" ";
        }
        if(i < rows - 1)
            fout<<"\n";
    }
}

bool ProceduralMapGenerator::isMapValid(std::vector<std::vector<Cell>>& map){
    int rows = hiveMind.getRowsN();
    int cols = hiveMind.getColumnsN();

    int brow = -1, bcol = -1;
    for(int i = 0; i < rows; i++)
        for(int j = 0; j < cols; j++)
            if(map[i][j] == Cell::BASE){
                brow = i;
                bcol = j;
                j = cols;
                i = rows;
            }

    std::vector<std::vector<bool>> visited(rows,std::vector<bool>(cols,false));
    std::queue<std::pair<int,int>> q;

    q.push({brow,bcol});
    visited[brow][bcol] = true;

    int dr[] = {-1,1,0,0};
    int dc[] = {0,0,-1,1};

    while(!q.empty()){
        auto [row,col] = q.front();
        q.pop();
        
        for(int i = 0; i < 4; ++i) {
            int nr = row + dr[i], nc = col + dc[i];
            if(nr >= 0 && nr < rows && nc >= 0 && nc < cols
                && !visited[nr][nc] && map[nr][nc] != Cell::WALL) {
                visited[nr][nc] = true;
                q.push({nr,nc});
            }
        }
    }

    for(int i = 0; i < rows; i++)
        for(int j = 0; j< cols; j++)
            if(
                (map[i][j] == Cell::STATION ||
                map[i][j] == Cell::CLIENT) &&
                visited[i][j] == false    
            )
                return false;

    return true;
}