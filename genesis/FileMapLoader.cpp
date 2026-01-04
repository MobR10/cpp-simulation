#include "IMapGenerator.h"

#include <fstream>
#include <iostream>
#include <string>


FileMapLoader::FileMapLoader(){}

void FileMapLoader::load(){
    std::ifstream fin(mapFileName);
    if(!fin.is_open()){
        std::cerr<<"Couln't open the file " << mapFileName << "\n";
        return;
    }
    std::string contents(
    (std::istreambuf_iterator<char>(fin)), std::istreambuf_iterator<char>());
    std::cout<<contents;
}