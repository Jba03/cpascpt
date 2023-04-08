//
//  interface.cpp
//  cpascpt
//
//  Created by Jba03 on 2023-04-07.
//

#include "interface.hh"

#include <fstream>
#include <memory>

#define pointerRead(streamName, code) std::function<void (std::istream &)> ([](std::fstream& streamName) { code; }))

template <typename T>
class ReadResult
{
public:
    T data;
    long readOffset = 0;
    ReadResult(T type, long readOffset) : data(type), readOffset(readOffset) {
        
        
        int a;
    }

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
    
    T doAt(Level *level, std::function<void(std::fstream&)>&& callback)
    {
        std::pair<pointer, uint8_t> pair = level->pointers[readOffset];
        pointer pointer = pair.first;
        uint8_t fileID = pair.second;
        
        if (fileID < 2) // >= 3: kf, vb
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
            callback(stream);
            // Go back to checkpoint
            stream.seekg(checkpoint);
        }
        
        return data;
    }
};

template <typename T>
static ReadResult<T> read(std::fstream& file)
{
    T data;
    file.read((char*)&data, sizeof(T));
    return ReadResult<T>(data, file.tellg());
}

static void ReadAt(const pointer p, Level* level, std::function<void(std::fstream&)>&& callback)
{
    std::pair<pointer, uint8_t> data = level->pointers[p];
    pointer pointer = data.first;
    uint8_t fileID = data.second;
    
    if (fileID < 2) // >= 3: kf, vb
    {
        // Go to the level the pointer points to
        Level* lvl = level->interface->level[data.second];
        // Get the file handle of the level file
        std::fstream& stream = lvl->levelFile;
        // Save position
        auto checkpoint = stream.tellg();
        // Seek to pointer location
        stream.seekg(pointer);
        // Let the user decide.
        callback(stream);
        // Go back to checkpoint
        stream.seekg(checkpoint);
    }
}

Level::Level(GameInterface* interface, std::fstream& lvl, std::fstream& ptr, bool isFix) : levelFile(lvl), pointerFile(ptr), isFix(isFix)
{
    this->interface = interface;
    
    uint32_t numPointers = read<uint32_t>(ptr).swap();
    while (numPointers--)
    {
        uint32_t fileID = read<uint32_t>(ptr).swap();
        doublepointer doublePointer = read<doublepointer>(ptr).swap();
        // Seek to the pointed location
        lvl.seekg(doublePointer + 4);
        // Read the pointer at the location
        pointer resultingPointer = read<pointer>(lvl).swap();
        
        pointers[doublePointer + 4] = std::make_pair(resultingPointer, fileID);
    }
    
    
    
//    ReadAt(0, this, std::function<void (std::fstream &)> ([](std::fstream& stream) {
//
//    }));
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
        
    }
    else
    {
        
    }
}

void Level::advance(int bytes)
{
    levelFile.ignore(bytes);
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
