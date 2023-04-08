//
//  compile.hh
//  cpascpt
//
//  Created by Jba03 on 2023-04-07.
//

#ifndef compile_hh
#define compile_hh

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <any>

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
    NodeType type;
    std::any param;
    unsigned depth;
};

struct CompilerContext
{
    enum Target
    {
        Target_R3_GC,
        Target_R3_PC,
    };
    
    enum Options
    {
        IgnoreAllErrors,
    };
    
    CompilerContext(Target t, Options opt = {})
    {
        target = t;
        options = opt;
    }
    
    // Appends a new node to the tree.
    template <typename T>
    void makeNode(NodeType type, T param)
    {
        Node nd;
        nd.type = type;
        nd.depth = currentDepth;
        nd.param = param;
        
        nodes.push_back(nd);
    }
    
    void shiftDepth(int s)
    {
        currentDepth += s;
    }
    
    void compile(std::string source);
    void loadTables();
    
    // Callback to find a subroutine by name. Returned is an offset, by the user correctly set.
    uint32_t (*callbackFindSubroutine)(const char* actorName, const char* subroutineName);
    // Callback to find an actor by name. Returned is an offset, by the user correctly set.
    //uint32_t (*callbackFindSubroutine)(const char* actorName, const char* subroutineName);
    
    Target target;
    Options options;
    
    std::vector<Node> nodes;
    int currentDepth = 1;
    
    // Table
    std::vector<std::string> nodeTypeTable;
    std::vector<std::string> keywordTable;
    std::vector<std::string> operatorTable;
    std::vector<std::string> functionTable;
    std::vector<std::string> procedureTable;
    std::vector<std::string> conditionTable;
    std::vector<std::string> fieldTable;
    std::vector<std::string> metaActionTable;;
};

#endif /* compile_hh */
