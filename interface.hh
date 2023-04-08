//
//  interface.hpp
//  cpascpt
//
//  Created by Jba03 on 2023-04-07.
//

#ifndef interface_hh
#define interface_hh

#include <map>
#include <unordered_map>
#include <vector>
#include <fstream>

#define swap16(data) \
    ((((data) >> 8) & 0x00FF) | (((data) << 8) & 0xFF00))

#define swap32(data) \
    ((((data) >> 24) & 0x000000FF) | (((data) >>  8) & 0x0000FF00) | \
    ( ((data) <<  8) & 0x00FF0000) | (((data) << 24) & 0xFF000000) )

#define host_byteorder_16(data) swap16(data)
#define host_byteorder_32(data) swap32(data)
#define game_byteorder_16(data) swap16(data)
#define game_byteorder_32(data) swap32(data)
#define readonly const

typedef uint32_t pointer;
typedef uint32_t doublepointer;

struct Level;
struct GameInterface;

struct Macro
{
    std::string name;
    uint32_t offset;
};

//
//struct MacroList
//{
//    readonly pointer macros;
//    readonly uint8_t numMacros;
//    readonly uint8_t padding[3];
//};
//
//struct AIModel
//{
//    readonly pointer intelligenceBehaviorList;
//    readonly pointer reflexBehaviorList;
//    readonly pointer dsgVar;
//    readonly pointer macroList;
//    readonly uint8_t secondPass;
//    readonly uint8_t padding[3];
//};
//
//struct Mind
//{
//    readonly pointer AIModel;
//    readonly pointer intelligence;
//    readonly pointer reflex;
//    readonly pointer dsgMem;
//    readonly uint8_t runIntelligence;
//    readonly uint8_t padding[3];
//};
//
struct Brain
{
    readonly pointer mind;
    readonly pointer lastNoCollideMaterial;
    readonly uint8_t warnMechanics;
    readonly uint8_t activeDuringTransition;
    readonly uint8_t padding[2];
    
    
};

struct Actor
{
    uint32_t familyType;
    uint32_t modelType;
    uint32_t instanceType;
    
    std::vector<Macro> macros;
    std::string name;
    
    uint32_t offset;
};

struct Level
{
    GameInterface* interface;
    std::fstream& levelFile;
    std::fstream& pointerFile;
    std::unordered_map<pointer, std::pair<pointer, uint8_t /* file */> >pointers;
    bool isFix;
    
    int numTextures = 0;
    
    void ReadActor(std::fstream& stream);
    
    Level(GameInterface* interface, std::fstream& lvl, std::fstream& ptr, bool isFix = true);
    void Load();
    void advance(int bytes);
    void seek(long offset);
};

struct GameInterface
{
    // [0] = fixed memory
    // [1] = level memory
    std::vector<Level*> level;
    std::vector<Actor> actors;
    
    GameInterface() {}
    GameInterface(std::fstream& fix,
                  std::fstream& fix_ptr,
                  std::fstream& lvl,
                  std::fstream& lvl_ptr);
    
    Actor* findActor(std::string name);
    Macro* findMacro(Actor* actor, std::string macroName);
};

#endif /* interface_hh */
