//
//  main.cpp
//  cpascpt
//
//  Created by Jba03 on 2023-04-07.
//

#include <iostream>
#include <filesystem>

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
    if (argc < 4)
    {
        printf("usage: cpascpt [fix.lvl] [*.lvl] [sourcefile]\n");
        return -1;
    }
    
    std::filesystem::path fixPath = std::filesystem::path(argv[1]).replace_extension("");
    std::filesystem::path levelPath = std::filesystem::path(argv[2]).replace_extension("");
    std::filesystem::path sourcePath = std::filesystem::path(argv[3]).remove_filename();
    std::filesystem::path sourceFileName = std::filesystem::path(argv[3]).filename();
    
    
    // TODO: maybe read these into memory before initializing the interface.
    std::fstream fixLvl(fixPath.string() + ".lvl", std::ios_base::binary | std::ios_base::in | std::ios_base::out);
    std::fstream fixPtr(fixPath.string() + ".ptr", std::ios_base::binary | std::ios_base::in | std::ios_base::out);
    std::fstream lvlLvl(levelPath.string() + ".lvl", std::ios_base::binary | std::ios_base::in | std::ios_base::out);
    std::fstream lvlPtr(levelPath.string() + ".ptr", std::ios_base::binary | std::ios_base::in | std::ios_base::out);
    
    if (!fixLvl.is_open() || !fixPtr.is_open())
    {
        fprintf(stderr, "failed to open %s: %s\n", (fixPath.string() + (fixLvl.is_open() ? "ptr" : "lvl")).c_str(), strerror(errno));
        return -1;
    }
    
    if (!lvlLvl.is_open() || !lvlPtr.is_open())
    {
        fprintf(stderr, "failed to open %s: %s\n", (levelPath.string() + (lvlLvl.is_open() ? "ptr" : "lvl")).c_str(), strerror(errno));
        return -1;
    }
    
    // Read source file
    std::ifstream file(argv[3]);
    std::stringstream source;
    source << file.rdbuf();
    
    // Load the game interface
    gameInterface = GameInterface(fixLvl, fixPtr, lvlLvl, lvlPtr);
    
    // Compile!
    CompilerContext compiler(CompilerContext::Target::Target_R3_GC);
    compiler.callbackFindActor = findActor;
    compiler.callbackFindSubroutine = findSubroutine;
    compiler.compile(source.str());
    
    compiler.nodetree.print(compiler.nodeTypeTable);
    
    std::fstream binary(sourcePath.string() + sourceFileName.string() + ".bin", std::ios_base::out | std::ios_base::binary);
    compiler.nodetree.write(binary);
    
    gameInterface.insertTree(compiler.nodetree);
    
    return 0;
}
