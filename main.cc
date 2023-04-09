//
//  main.cpp
//  cpascpt
//
//  Created by Jba03 on 2023-04-07.
//

#include <iostream>

#include "compile.hh"
#include "interface.hh"

GameInterface gameInterface;

static uint32_t findActor(const char* actorName)
{
    Actor* a = gameInterface.findActor(actorName);
    return a ? a->offset : 0;
}

static uint32_t findSubroutine(const char* actorName, const char* macroName)
{
    Macro* m = gameInterface.findMacro(gameInterface.findActor(actorName), macroName);
    return m ? m->offset : 0;
}

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
    
    // Read source file
    std::ifstream file("tests/test2");
    std::stringstream source;
    source << file.rdbuf();
    
    // Load the game interface
    gameInterface = GameInterface(fixLvl, fixPtr, lvlLvl, lvlPtr);
    
    // Compile!
    CompilerContext compiler(CompilerContext::Target::Target_R3_GC);
    compiler.callbackFindActor = findActor;
    compiler.callbackFindSubroutine = findSubroutine;
    compiler.compile(source.str());
    
    for (Node node : compiler.nodes)
    {
        for (int i = 0; i < (node.depth-1)*4; i++)printf(" ");
        printf("%s: %d (%d)\n", compiler.nodeTypeTable[node.type].c_str(), node.param, node.depth);
    }
    
    //printf("n nodes: %zu\n", compiler.nodes.size());
    
    return 0;
}
