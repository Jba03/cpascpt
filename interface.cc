//
//  interface.cpp
//  cpascpt
//
//  Created by Jba03 on 2023-04-07.
//

#include "interface.hh"

#include <fstream>
#include <sstream>
#include <memory>

#pragma mark - Memory

#define ptrReadFn std::function<void (std::fstream &)> 

template <typename T>
class ReadResult
{
public:
    T data;
    unsigned readOffset = 0;
    ReadResult(T type, long readOffset = 0) : data(type), readOffset(readOffset) { }

    operator T() const { return data; }
    T swap()
    {
        switch (sizeof(T))
        {
            case 1: return data;
            case 2: return swap16(data);
            case 4: return swap32(data);
            default: return data;
        }
    }
    
    T doAt(Level *level, std::function<void(std::fstream&, uint8_t)>&& callback)
    {
        std::pair<pointer, uint8_t> pair = level->pointers[readOffset - 4];
        pointer pointer = pair.first;
        uint8_t fileID = pair.second;
        
        if (pointer != 0 && fileID < 2) // >= 3: kf, vb
        {
            // Go to the level the pointer points to
            Level* lvl = level->interface->level[fileID];
            // Get the stream handle of the level buffer
            std::fstream& stream = lvl->levelFile;
            // Save position
            auto checkpoint = stream.tellg();
            // Seek to pointer location
            stream.seekg(pointer);
            // Let the user decide.
            callback(stream, fileID);
            // Go back to checkpoint
            stream.seekg(checkpoint);
        }
        
        return data;
    }
    
    T doAt(Level *level, std::function<void(std::fstream&)>&& callback)
    {
        return doAt(level, std::function<void (std::fstream &, uint8_t)> ([callback](std::fstream& stream, uint8_t) { callback(stream); }));
    }
    
    T* dynamic()
    {
        T *dyn = new T{};
        memcpy(dyn, &data, sizeof(T));
        return dyn;
    }
};

template <typename T>
static ReadResult<T> read(std::fstream& file)
{
    T data {};
    file.read((char*)&data, sizeof(T));
    return ReadResult<T>(data, file.tellg());
}

#pragma mark - Level

Level::Level(GameInterface* interface, std::fstream& lvl, std::fstream& ptr, bool isFix) : levelFile(lvl), pointerFile(ptr), isFix(isFix)
{
    this->interface = interface;
    
    uint32_t numPointers = read<uint32_t>(ptr).swap();
    while (numPointers--)
    {
        uint32_t fileID = read<uint32_t>(ptr).swap();
        doublepointer doublePointer = read<doublepointer>(ptr).swap() + 4;
        // Seek to the pointed location
        lvl.seekg(doublePointer);
        // Read the pointer at the location
        pointer resultingPointer = read<pointer>(lvl).swap() + 4;
        
        pointers[doublePointer] = std::make_pair(resultingPointer, fileID);
    }
    
    lvl.seekg(0);
    
    
//    ReadAt(0, this, std::function<void (std::fstream &)> ([](std::fstream& stream) {
//
//    }));
}

void Level::ReadActor(std::fstream& stream)
{
    //auto savepoint = stream.tellg();
    
    Actor actor;
    
    stream.ignore(4); // Skip 3DData
    read<pointer>(stream).doAt(this, ptrReadFn([this, &actor](std::fstream& stdGameStream) {
        // Inside stdgame struct
        actor.familyType = read<uint32_t>(stdGameStream).swap();
        actor.modelType = read<uint32_t>(stdGameStream).swap();
        actor.instanceType = read<uint32_t>(stdGameStream).swap();        
    }));
    
    stream.ignore(4); // Skip dynamics
    read<pointer>(stream).doAt(this, ptrReadFn([this, &actor](std::fstream& brainStream) {
        // Inside brain struct
        read<pointer>(brainStream).doAt(this, ptrReadFn([this, &actor](std::fstream& mindStream) {
            // Inside mind struct
            read<pointer>(mindStream).doAt(this, ptrReadFn([this, &actor](std::fstream& AIModelStream) {
                // Inside AIModel struct
                // Skip intelligence, reflex and dsgvars
                read<pointer>(AIModelStream).doAt(this, ptrReadFn([this, &actor](std::fstream& macroListStream) {
                    // Save position
                    auto savepoint = macroListStream.tellg();
                    // Skip first macro pointer
                    macroListStream.ignore(4);
                    // Read macro count
                    uint32_t numMacros = read<uint32_t>(macroListStream).swap();
                    // Seek back
                    macroListStream.seekg(savepoint);
                    // Read the macros
                    read<pointer>(macroListStream).doAt(this, ptrReadFn([this, &actor, numMacros](std::fstream& macroStream) {
                        for (unsigned int i = 0; i < numMacros; i++) {
                            
                            char* buf = new char[0x100];
                            macroStream.read(buf, 0x100);
                            std::string name(buf);
                            delete[] buf;
                            
                            Macro macro;
                            macro.name = name;
                            actor.macros.push_back(macro);
                        }
                    }));
                }));
            }));
        }));
    }));
    
    this->interface->actors.push_back(actor);
    
    //stream.seekg(savepoint);
}

void Level::Load()
{
    if (isFix)
    {
        // Base + ? + matrix + localization
        advance(4 * 6);
        // Text
        advance(20);
        
        uint32_t levelNameCount = read<uint32_t>(levelFile).swap();
        uint32_t demoNameCount = read<uint32_t>(levelFile).swap();
        
        // Demo save names
        advance(12 * demoNameCount);
        // Demo level names
        advance(12 * demoNameCount);
        // Level names + name of first level
        advance(30 * levelNameCount + 30);
        // Language
        advance(10);
        // Texture count
        numTextures = read<uint32_t>(levelFile).swap();
        // Skip textures
        advance(numTextures * 4);
        // Skip menu textures
        advance(read<uint32_t>(levelFile).swap() * 4);
        // Skip memory channels
        advance(numTextures * 4);
        // Skip input structure (for now)
        advance(0x12E0 + 0x8 + 0x418 + 0xE8);
        
        uint32_t numActors = read<uint32_t>(levelFile).swap();
        for (unsigned int n = 0; n < numActors; n++)
        {
            read<pointer>(levelFile).doAt(this, ptrReadFn([this](std::fstream& actorStream) {
                // Start of actor struct
                ReadActor(actorStream);
            }));
        }
        
        
    }
    else
    {
        
    }
}

void Level::advance(int bytes)
{
    levelFile.ignore(bytes);
}

void Level::seek(long offset)
{
    levelFile.seekg(offset);
}

GameInterface::GameInterface(std::fstream& fix,
                             std::fstream& fix_ptr,
                             std::fstream& lvl,
                             std::fstream& lvl_ptr)
{
    Level* fixLevel = new Level(this, fix, fix_ptr);
    Level* lvlLevel = new Level(this, lvl, lvl_ptr, false);

    level.push_back(fixLevel);
    level.push_back(lvlLevel);
    
    fixLevel->Load();
    lvlLevel->Load();
}

Actor* GameInterface::findActor(std::string name)
{
    for (Actor& a : actors)
        if (a.name == name) return &a;
    
    return nullptr;
}

Macro* GameInterface::findMacro(Actor* actor, std::string macroName)
{
    if (!actor) return nullptr;
    for (Macro& m : actor->macros)
        if (m.name == macroName) return &m;
    
    return nullptr;
}
