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
#include <iostream>

#pragma mark - Memory

#define ptrReadFn std::function<void (std::fstream &)> 

template <typename T>
class ReadResult
{
public:
    T data;
    unsigned readOffset = 0;
    std::fstream stream;
    
    ReadResult(T type, long readOffset = 0) : data(type), readOffset(readOffset)
    {
//        if (std::is_same<T, pointer>::value) // Read pointer
//        {
//            std::pair<pointer, uint8_t> pair = level->pointers[readOffset - 4];
//            pointer pointer = pair.first;
//            uint8_t fileID = pair.second;
//            data = pointer;
//        }
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
    
    T doAt(Level *level, std::function<void(std::fstream&, uint8_t)>&& callback)
    {
        std::pair<pointer, uint8_t> pair = level->pointers[readOffset];
        pointer pointer = pair.first;
        uint8_t fileID = pair.second;
        
        if (pointer != 0 && fileID < 2) // >= 2: kf, vb
        {
            // Go to the level the pointer points to
            Level* lvl = level->interface->level[fileID];
            // Get the stream handle of the level buffer
            std::fstream& stream = lvl->levelFile;
            // Save position
            auto checkpoint = stream.tellg();
            // Read at pointer
            stream.seekg(pointer);
            callback(stream, fileID);
            // Seek back
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
    uint32_t offset = file.tellg();
    file.read((char*)&data, sizeof(T));
    return ReadResult<T>(data, offset);
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
}

void Level::ReadFillInPointers()
{
    auto checkpoint = pointerFile.tellg();
    pointerFile.seekg(0, pointerFile.end);
    long size = pointerFile.tellg();
    pointerFile.seekg(checkpoint);
    
    uint32_t numFillInPointers = unsigned((size - pointerFile.tellg()) / 16);
    while (numFillInPointers--)
    {
        uint32_t doublePointer = read<uint32_t>(pointerFile).swap();
        uint32_t sourceFile = read<uint32_t>(pointerFile).swap();
        pointer realPointer = read<pointer>(pointerFile).swap();
        uint32_t targetFile = read<uint32_t>(pointerFile).swap();
        
        if (sourceFile < 2 && targetFile < 2)
        {
            //printf("%d (%d) -> %d (%d)\n", doublePointer, sourceFile, realPointer, targetFile);
            this->interface->level[sourceFile]->pointers[doublePointer] = std::make_pair(realPointer, targetFile);
        }
    }
}

template <typename T>
static void ReadIntelligence(Level *level, std::fstream& stream, std::vector<T>& list, bool isMacro = false)
{
    auto checkpoint = stream.tellg();
    
    read<pointer>(stream).doAt(level, ptrReadFn([level, &list, isMacro](std::fstream& listStream) {
        // Save position
        auto savepoint = listStream.tellg();
        // Skip first macro pointer
        listStream.ignore(4);
        // Read macro count
        uint32_t count = isMacro ? read<uint8_t>(listStream).swap() : read<uint32_t>(listStream).swap();
        // Seek back
        listStream.seekg(savepoint);
        // Read the macros
        read<pointer>(listStream).doAt(level, ptrReadFn([level, &list, count, isMacro](std::fstream& typeStream) {
            for (unsigned int i = 0; i < count; i++) {
                
                uint32_t offset = typeStream.tellg();
                
                char* buf = new char[0x100];
                typeStream.read(buf, 0x100);
                std::string name(buf);
                delete[] buf;
                
                name = name.substr(name.find(':') + 1);
                
                T readtype;
                readtype.name = name;
                readtype.offset = offset;
                list.push_back(readtype);
                
                typeStream.ignore(isMacro ? 8 : 12);
            }
        }));
    }));
    
    stream.seekg(long(checkpoint) + 4);
}

void Level::ReadActor(std::fstream& stream, uint8_t fileID)
{
    //auto savepoint = stream.tellg();
    
    Actor actor;
    actor.offset = stream.tellg();
    actor.fileID = fileID;
    
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
                AIModelStream.ignore(0);
                
                std::vector<Behavior> intelligenceList;
                std::vector<Behavior> reflexList;
                std::vector<Macro> macroList;
                
                ReadIntelligence(this, AIModelStream, actor.intelligenceList);
                ReadIntelligence(this, AIModelStream, actor.reflexList);
                AIModelStream.ignore(4); // Skip dsgvars
                ReadIntelligence(this, AIModelStream, actor.macroList, true);
            }));
        }));
    }));
    
    this->interface->actors.push_back(actor);
    
    //stream.seekg(savepoint);
}

static void ReadObjectType(Level *lvl, std::fstream& stream, std::vector<std::string>& names)
{
    auto readtype = [lvl, &names] (std::fstream &elementStream) {
        elementStream.ignore(12);
        
        read<pointer>(elementStream).doAt(lvl, ptrReadFn([lvl, &names](std::fstream& nameStream) {
            std::stringstream sstream;
            
            while (1) {
                char c = read<char>(nameStream);
                if (c < 33 || c > 126) break;
                sstream << std::string(1, c);
            }
            
            std::string name = sstream.str();
            names.push_back(name);
        }));
        
        elementStream.ignore(4);
    };
    
    readtype(stream);
    while (1) {
        if (read<ObjectTypeElement>(stream).doAt(lvl, ptrReadFn([lvl, &names, readtype](std::fstream& elementStream) {
            readtype(elementStream);
        })).next == 0) break;
    }
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
            read<pointer>(levelFile).doAt(this, std::function<void (std::fstream &, uint8_t)> ([this](std::fstream& actorStream, uint8_t fileID) {
                // Start of actor struct
                ReadActor(actorStream, fileID);
            }));
        }
        
        
    }
    else
    {
        // Base + ?
        advance(4 + 4 * 4);
        // Text + ?
        advance(24 + 4 * 60);
        // Texture count (minus fix texture count)
        numTextures = read<uint32_t>(levelFile).swap() - interface->level[0]->numTextures;
        // Skip textures
        advance(2 * numTextures * 4);
        
        //printf("%lld\n", levelFile.tellg().operator long long());
        
        pointer actualWorld = read<pointer>(levelFile).swap();
        pointer dynamicWorld = read<pointer>(levelFile).swap();
        pointer inactiveDynamicWorld = read<pointer>(levelFile).swap();
        pointer fatherSector = read<pointer>(levelFile).swap();
        pointer firstSubmapPosition = read<pointer>(levelFile).swap();
        // Skip until object types
        advance(7 * 4);
        
        // Family names
        read<LinkedList>(levelFile).doAt(this, ptrReadFn([this](std::fstream& stream) {
            ReadObjectType(this, stream, this->interface->familyNames);
        }));
        
        // Model names
        read<LinkedList>(levelFile).doAt(this, ptrReadFn([this](std::fstream& stream) {
            ReadObjectType(this, stream, this->interface->modelNames);
        }));
        
        // Instance names
        read<LinkedList>(levelFile).doAt(this, ptrReadFn([this](std::fstream& stream) {
            ReadObjectType(this, stream, this->interface->instanceNames);
        }));
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
    
    fixLevel->ReadFillInPointers();
    lvlLevel->ReadFillInPointers();
    
    fixLevel->Load();
    lvlLevel->Load();
    
    for (Actor& a : actors)
    {
        a.name = instanceNames.at(a.instanceType);
    }
    
    targetActor = findActor("Rayman");
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
    for (Macro& m : actor->macroList)
        if (m.name == macroName) return &m;
    
    return nullptr;
}

int GameInterface::insertTree(NodeTree& tree)
{
    if (!targetActor) return -1;
    
    // Find the files in which the target actor is located
    std::fstream& levelStream = level[targetActor->fileID]->levelFile;
    std::fstream& pointerStream = level[targetActor->fileID]->pointerFile;
    
    levelStream.seekg(0, levelStream.end);
    long levelStreamSize = levelStream.tellg();
    levelStream.seekg(0, levelStream.beg);
    
    pointerStream.seekg(0, pointerStream.end);
    long pointerStreamSize = pointerStream.tellg();
    pointerStream.seekg(0, pointerStream.beg);
    
    // Seek to the end of the level
    levelStream.seekg(0, levelStream.end);
    // Script marker begin
    levelStream << "cpascpt.begin" << '\0' << '\0' << '\0';
    
    long textRegionOffset = levelStream.tellg();
    // Allocate size for the text region
    for (int i = 0; i < tree.textRegionSize(); i++) levelStream << '\0';
    
    for (Node node : tree.nodes)
    {
        uint32_t param = 0;
        
        switch (node.type)
        {
            case NodeType::String:
            {
                auto save = levelStream.tellg();
                levelStream.seekg(textRegionOffset);
                param = textRegionOffset;
                std::string text = std::any_cast<std::string>(node.param);
                levelStream << text;
                textRegionOffset += text.length() + 4 - (text.length() % 4);
                levelStream.seekg(save);
                break;
            }
                
            case NodeType::Real:
            {
                float f = std::any_cast<float>(node.param);
                param = *(uint32_t*)&f;
                break;
            }
                
            default:
                param = std::any_cast<uint32_t>(node.param);
                break;
        }
        
        param = swap32(param);
        
        uint8_t padding[3] = {0, 0, 0};
        levelStream.write((char*)&param, 4);
        levelStream.write((char*)&padding, 3);
        levelStream.write((char*)&node.type, 1);
        levelStream.write((char*)&padding, 2);
        levelStream.write((char*)&node.depth, 1);
        levelStream.write((char*)&padding, 1);
    }
    
    // Script marker end
    levelStream << "cpascpt.end" << '\0' << '\0' << '\0' << '\0' << '\0';
    
    levelStream.close();
    
    return 0;
}
