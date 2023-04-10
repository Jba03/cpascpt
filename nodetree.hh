//
//  nodetree.hh
//  cpascpt
//
//  Created by Jba03 on 2023-04-10.
//

#ifndef nodetree_hh
#define nodetree_hh

#include <any>
#include <cstdint>
#include <vector>

enum NodeType
{
    KeyWord            = 0,
    Condition          = 1,
    Operator           = 2,
    Function           = 3,
    Procedure          = 4,
    MetaAction         = 5,
    BeginMacro         = 6,
    BeginMacro2        = 7,
    EndMacro           = 8,
    Field              = 9,
    DsgVarRef          = 10,
    DsgVarRef2         = 11,
    Constant           = 12,
    Real               = 13,
    Button             = 14,
    ConstantVector     = 15,
    Vector             = 16,
    Mask               = 17,
    ModuleRef          = 18,
    DsgVarId           = 19,
    String             = 20,
    LipsSynchroRef     = 21,
    FamilyRef          = 22,
    ActorRef           = 23,
    ActionRef          = 24,
    SuperObjectRef     = 25,
    Unknown            = 26,
    WayPointRef        = 27,
    TextRef            = 28,
    ComportRef         = 29,
    ModuleRef2         = 30,
    SoundEventRef      = 31,
    ObjectTableRef     = 32,
    GameMaterialRef    = 33,
    VisualMaterial     = 34,
    ParticleGenerator  = 35,
    ModelRef           = 36,
    ModelRef2          = 37,
    CustomBits         = 38,
    Caps               = 39,
    Unknown2           = 40,
    SubRoutine         = 41,
    Null               = 42,
    Null2              = 43,
    GraphRef           = 44,
};

struct Node
{
    uint8_t type;
    uint8_t depth;
    std::any param;
};

struct NodeTree
{
    std::vector<Node> nodes;
    int depth = 1;
    
    void add(Node node)
    {
        nodes.push_back(node);
    }
    
    void clear()
    {
        nodes.clear();
    }
    
    unsigned length()
    {
        return nodes.size();
    }
    
    void print(std::vector<std::string>& nodeTypes)
    {
        for (Node node : nodes)
        {
            for (int i = 0; i < (node.depth - 1) * 4; i++)
                printf(" ");
            
            switch (node.type)
            {
                case NodeType::String:
                    printf("%s: %s (%d)\n", nodeTypes[node.type].c_str(), std::any_cast<std::string>(node.param).c_str(), node.depth);
                    break;
                    
                default:
                    printf("%s: %d (%d)\n", nodeTypes[node.type].c_str(), std::any_cast<uint32_t>(node.param), node.depth);
            }
        }
    }
    
    void write(std::fstream& stream)
    {
        for (Node node : nodes)
        {
            uint8_t type = node.type;
            uint8_t depth = node.depth;
            uint32_t param = 0;
            uint8_t padding[3] = {0, 0, 0};
            
            if (type != NodeType::String)
            {
                param = std::any_cast<uint32_t>(node.param);
            }
            
            stream.write((char*)&param, 4);
            stream.write((char*)&padding, 3);
            stream.write((char*)&type, 1);
            stream.write((char*)&padding, 2);
            stream.write((char*)&depth, 1);
            stream.write((char*)&padding, 1);
        }
    }
};

#endif /* nodetree_hh */
