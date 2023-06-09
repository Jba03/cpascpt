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

#include "nodetree.hh"

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
    void makeNode(NodeType type, std::any param)
    {
        Node nd;
        nd.type = type;
        nd.depth = nodetree.depth;
        nd.param = param;
        
        if (callbackEmitNode)
            callbackEmitNode(nd.type, std::any_cast<uint32_t>(nd.param), nd.depth);
        
        nodetree.add(nd);
    }
    
    void shiftDepth(int s)
    {
        nodetree.depth += s;
    }
    
    void compile(std::string source);
    void loadTables();
    
    // Callback to find a subroutine by name. Returned is the address of the actor, 0 if none.
    uint32_t (*callbackFindSubroutine)(const char* actorName, const char* subroutineName) = nullptr;
    // Callback to find an actor by name. Returned is the address of the subroutine, 0 if none.
    uint32_t (*callbackFindActor)(const char* actorName) = nullptr;
    // Callback to be executed when a node is emitted from the compiler.
    void (*callbackEmitNode)(uint8_t type, uint32_t param, uint8_t depth) = nullptr;
    
    Target target;
    Options options;
    NodeTree nodetree;
    
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

#pragma mark - Compiler interoperability

#if WIN32
#   define DLLEXPORT __declspec(dllexport)
#else
#   define DLLEXPORT
#endif

// Create a new compiler context
DLLEXPORT CompilerContext* CPAScriptCompilerCreate(CompilerContext::Target target);
// Register callback for finding actor offets
DLLEXPORT void CPAScriptCompilerFindActorCallback(CompilerContext* compiler, uint32_t (*callback)(const char*));
// Register callback for finding macro offsets
DLLEXPORT void CPAScriptCompilerFindMacroCallback(CompilerContext* compiler, uint32_t (*callback)(const char*, const char*));
// Register callback for when the compiler emits a new node
DLLEXPORT void CPAScriptCompilerEmitNodeCallback(CompilerContext* compiler, void (*callback)(uint8_t, uint32_t, uint8_t));
// Compile source string
DLLEXPORT int CPAScriptCompilerCompile(CompilerContext* compiler, const char* source);
// Destroy compiler context
DLLEXPORT void CPAScriptCompilerDestroy(CompilerContext *c);

#endif /* compile_hh */
