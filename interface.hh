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

struct Macro
{
    readonly int8_t name[0x100];
    readonly pointer initialScript;
    readonly pointer currentScript;
};

struct SubroutineList
{
    readonly pointer macros;
    readonly uint8_t numMacros;
    readonly uint8_t padding[3];
};

struct AIModel
{
    readonly pointer intelligenceBehaviorList;
    readonly pointer reflexBehaviorList;
    readonly pointer dsgVar;
    readonly pointer macroList;
    readonly uint8_t secondPass;
    readonly uint8_t padding[3];
};

struct Mind
{
    readonly pointer AIModel;
    readonly pointer intelligence;
    readonly pointer reflex;
    readonly pointer dsgMem;
    readonly uint8_t runIntelligence;
    readonly uint8_t padding[3];
};

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
    readonly pointer p3DData;
    readonly pointer stdGame;
    readonly pointer dynam;
    readonly pointer brain;
    readonly pointer cineInfo;
    readonly pointer collSet;
    readonly pointer msWay;
    readonly pointer msLight;
    readonly pointer sectorInfo;
    readonly pointer micro;
    readonly pointer msSound;
};

struct GameInterface;

struct Level
{
    GameInterface* interface;
    std::fstream& levelFile;
    std::fstream& pointerFile;
    std::unordered_map<pointer, std::pair<pointer, uint8_t /* file */> >pointers;
    bool isFix;
    
    
    Level(GameInterface* interface, std::fstream& lvl, std::fstream& ptr, bool isFix = true);
    void Load();
    void advance(int bytes);
};

struct GameInterface
{
    // [0] = fixed memory
    // [1] = level memory
    std::vector<Level*> level;
    
    GameInterface(std::fstream& fix,
                  std::fstream& fix_ptr,
                  std::fstream& lvl,
                  std::fstream& lvl_ptr);
};

#endif /* interface_hh */
