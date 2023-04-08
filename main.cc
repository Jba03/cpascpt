//
//  main.cpp
//  cpascpt
//
//  Created by Jba03 on 2023-04-07.
//

#include <iostream>

#include "compile.hh"
#include "interface.hh"

int main(int argc, const char * argv[])
{
    // TODO: maybe read these into memory before initializing the interface.
    std::fstream fixLvl(argv[1], std::ios_base::binary | std::ios_base::in);
    std::fstream fixPtr(argv[2], std::ios_base::binary | std::ios_base::in);
    std::fstream lvlLvl(argv[3], std::ios_base::binary | std::ios_base::in);
    std::fstream lvlPtr(argv[4], std::ios_base::binary | std::ios_base::in);
    
    if (!fixLvl) exit(-1);
    if (!fixPtr) exit(-1);
    if (!lvlLvl) exit(-1);
    if (!lvlPtr) exit(-1);
    
    GameInterface interface(fixLvl, fixPtr, lvlLvl, lvlPtr);
    
    std::ifstream file("tests/test2");
    
    std::stringstream source;
    source << file.rdbuf();
    
    CompilerContext compiler(CompilerContext::Target::Target_R3_GC);
    
    compiler.compile(source.str());
    
    printf("n nodes: %zu\n", compiler.nodes.size());
    
    return 0;
}
