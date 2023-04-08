//
//  compile.cpp
//  cpascpt
//
//  Created by Jba03 on 2023-04-07.
//

#include "compile.hh"
#include "types-r3.hh"

#include <antlr4-runtime.h>

#include "GenericLexer.h"
#include "GenericParser.h"
#include "GenericBaseListener.h"

using namespace antlr4;
using namespace antlr4::tree;

class TreeShapeListener : public GenericBaseListener
{
private:
    CompilerContext* compiler;
    bool dotAccess = false;
    std::string errorString = "No error";
    
    void fail(ParserRuleContext* ctx, std::string reason)
    {
        std::stringstream stream;
        stream << "Line " << ctx->start->getLine() << ":" << ctx->start->getCharPositionInLine() << ": " << reason << "\n";
        errorString = stream.str();
        
        if (!(compiler->options & CompilerContext::IgnoreAllErrors))
        {
            std::cerr << errorString;
            exit(-1);
        }
    }
    
    long tableFindSymbol(std::vector<std::string> table, std::string symbol)
    {
        auto iter = std::find(table.begin(), table.end(), symbol);
        if (iter != table.end()) return iter - table.begin();
        return -1;
    }
    
    void* findSubroutine(std::string name)
    {
        return NULL;
    }
    
    bool isVectorOp()
    {
        return false;
    }
    
public:
    
    void setCompiler(CompilerContext *c)
    {
        compiler = c;
    }
    
    std::string getErrorMessage()
    {
        return errorString;
    }
    
    void enterStatement(GenericParser::StatementContext * ctx) override { }
    void exitStatement(GenericParser::StatementContext * ctx) override { }

    void enterStatementList(GenericParser::StatementListContext * ctx) override { }
    void exitStatementList(GenericParser::StatementListContext * ctx) override { }

    void enterBlock(GenericParser::BlockContext * ctx) override { }
    void exitBlock(GenericParser::BlockContext * ctx) override { }

    void enterComment(GenericParser::CommentContext * ctx) override { }
    void exitComment(GenericParser::CommentContext * ctx) override { }

    void enterIfStatement(GenericParser::IfStatementContext * ctx) override
    {
        if (!ctx->ifCondition()) fail(ctx, "Missing condition in if-statement");
        compiler->makeNode(NodeType::KeyWord, 0 /* If */);
        compiler->shiftDepth(+1);
    }
    
    void exitIfStatement(GenericParser::IfStatementContext * ctx) override
    {
        compiler->shiftDepth(-1);
    }
    
    void exitIfCondition(GenericParser::IfConditionContext * ctx) override
    {
        compiler->shiftDepth(-1);
        compiler->makeNode(NodeType::KeyWord, 16 /* Then */);
        compiler->shiftDepth(+1);
    }

    void enterElseStatement(GenericParser::ElseStatementContext * ctx) override
    {
        compiler->shiftDepth(-1);
        compiler->makeNode(NodeType::KeyWord, 17 /* Else */);
        compiler->shiftDepth(+1);
    }

    void enterExpressionStatement(GenericParser::ExpressionStatementContext * ctx) override { }
    void exitExpressionStatement(GenericParser::ExpressionStatementContext * ctx) override { }

    void enterFunctionCall(GenericParser::FunctionCallContext * ctx) override
    {
        GenericParser::FunctionNameContext *nameCtx = ctx->functionName();
        if (!nameCtx)
            fail(ctx, "Missing callable method name");
        
        std::string name = nameCtx->getText();
        std::cout << name << std::endl;
        
        long functionIndex = tableFindSymbol(compiler->functionTable, name);
        long procedureIndex = tableFindSymbol(compiler->procedureTable, name);
        long conditionIndex = tableFindSymbol(compiler->conditionTable, name);
        long metaActionIndex = tableFindSymbol(compiler->metaActionTable, name);
        
        void* subroutine = NULL;
        
        if      (functionIndex   >= 0) compiler->makeNode(NodeType::Function, functionIndex);
        else if (procedureIndex  >= 0) compiler->makeNode(NodeType::Procedure, procedureIndex);
        else if (conditionIndex  >= 0) compiler->makeNode(NodeType::Condition, conditionIndex);
        else if (metaActionIndex >= 0) compiler->makeNode(NodeType::MetaAction, metaActionIndex);
        else if ((subroutine = findSubroutine(name))) compiler->makeNode(NodeType::SubRoutine, /*subroutine->offset*/ 0);
        else fail(ctx, "No such callable method '" + name + "' found");
        // Make sure only procedures can be called within actor reference access.
        if (this->dotAccess && functionIndex   >= 0) fail(ctx, "Cannot call function '" + name + "' on actor reference");
        if (this->dotAccess && conditionIndex  >= 0) fail(ctx, "Cannot call condition '" + name + "' on actor reference");
        if (this->dotAccess && metaActionIndex >= 0) fail(ctx, "Cannot call meta-action '" + name + "' on actor reference");
        //if (this->dotAccess && subroutine >= 0) fail(ctx, "Cannot call meta-action '" + name + "' on actor reference");
        
        compiler->shiftDepth(+1);
    }
    
    void exitFunctionCall(GenericParser::FunctionCallContext * ctx) override
    {
        compiler->shiftDepth(-1);
    }
    
    void enterSingleExpression(GenericParser::SingleExpressionContext * ctx) override
    {
        // Operator
        GenericParser::ArithmeticOperatorContext *arithmeticOperator = ctx->arithmeticOperator();
        GenericParser::LogicalOperatorContext *logicalOperator = ctx->logicalOperator();
        GenericParser::ComparisonOperatorContext *comparisonOperator = ctx->comparisonOperator();
        GenericParser::UnaryOperatorContext *unaryOperator = ctx->unaryOperator();
        GenericParser::AssignmentOperatorContext *assignmentOperator = ctx->assignmentOperator();
        // Object/reference access
        GenericParser::FieldAccessOperatorContext *field = ctx->fieldAccessOperator();
        GenericParser::VectorComponentContext *vectorComponent = ctx->vectorComponent();
        GenericParser::LiteralContext *literal = ctx->literal();
        
        if (arithmeticOperator)
        {
            std::string op = arithmeticOperator->getText();
            if (op == "+") compiler->makeNode(NodeType::Operator, isVectorOp() ? 17 : 0);
            else if (op == "-") compiler->makeNode(NodeType::Operator, isVectorOp() ? 18 : 1);
            else if (op == "*") compiler->makeNode(NodeType::Operator, isVectorOp() ? 20 : 2);
            else if (op == "/") compiler->makeNode(NodeType::Operator, isVectorOp() ? 21 : 3);
            else if (op == "%")
            {
                if (isVectorOp())
                    fail(ctx, "Modulo operation '%' cannot be performed on a vector operand");
                else
                    compiler->makeNode(NodeType::Operator, 5);
            }
            else fail(ctx, "Invalid arithmetic operator '" + op + "'");
            
            compiler->shiftDepth(+1);
        }
        
        if (comparisonOperator)
        {
            if (isVectorOp()) fail(ctx, "Attempt to perform comparison on vector operand");
            
            std::string op = comparisonOperator->getText();
            if      (op == "==") compiler->makeNode(NodeType::Condition, 4);
            else if (op == "!=") compiler->makeNode(NodeType::Condition, 5);
            else if (op ==  "<") compiler->makeNode(NodeType::Condition, 6);
            else if (op ==  ">") compiler->makeNode(NodeType::Condition, 7);
            else if (op == "<=") compiler->makeNode(NodeType::Condition, 8);
            else if (op == ">=") compiler->makeNode(NodeType::Condition, 9);
            else fail(ctx, "Invalid comparison operator '" + op + "'");
            
            compiler->shiftDepth(+1);
        }
        
        if (logicalOperator)
        {
            if (isVectorOp()) fail(ctx, "Attempt to perform logical operation on vector operand");
            
            std::string op = logicalOperator->getText();
            if (op == "&&") compiler->makeNode(NodeType::Condition, 0);
            else if (op == "||") compiler->makeNode(NodeType::Condition, 1);
            // `NOT` operator (2) is specified as a unary operator further down.
            else if (op ==  "^") compiler->makeNode(NodeType::Condition, 3);
            else fail(ctx, "Invalid logical operator '" + op + "'");
            
            compiler->shiftDepth(+1);
        }
        
        if (unaryOperator)
        {
            std::string op = unaryOperator->getText();
            if      (op == "+") /* Do nothing */;
            else if (op == "-") compiler->makeNode(NodeType::Operator, isVectorOp() ? 19 : 4);
            else if (op == "!")
            {
                if (isVectorOp()) fail(ctx, "Attempt to perform logical operation on vector operand");
                compiler->makeNode(NodeType::Condition, 2 /* ! */);
            }
            else fail(ctx, "Invalid unary operator '" + op + "'");
            
            compiler->shiftDepth(+1);
        }
        
        if (assignmentOperator)
        {
            std::string op = assignmentOperator->getText();
            if      (op ==  "=") compiler->makeNode(NodeType::Operator, 12);
            else if (op == "+=") compiler->makeNode(NodeType::Operator, 6);
            else if (op == "-=") compiler->makeNode(NodeType::Operator, 7);
            else if (op == "*=") compiler->makeNode(NodeType::Operator, 8);
            else if (op == "/=") compiler->makeNode(NodeType::Operator, 9);
            else fail(ctx, "Invalid assignment operator '" + op + "'");
            
            compiler->shiftDepth(+1);
        }
        
        if (field)
        {
            if (field->getText() != ".") fail(ctx, "Invalid field access");
            compiler->makeNode(NodeType::Operator, 13 /* . */);
            compiler->shiftDepth(+1);
            this->dotAccess = true;
        }
        
        if (vectorComponent)
        {
            std::string component = vectorComponent->getText();
            if      (component == "X" || component == "x") compiler->makeNode(NodeType::Operator, 14 /* .X */);
            else if (component == "Y" || component == "y") compiler->makeNode(NodeType::Operator, 15 /* .Y */);
            else if (component == "Z" || component == "z") compiler->makeNode(NodeType::Operator, 16 /* .Z */);
            else fail(ctx, "Invalid vector component '" + component + "'");
            //compiler->shiftDepth(+1);
        }
        
    }
    
    void exitSingleExpression(GenericParser::SingleExpressionContext * ctx) override
    {
        if (ctx->arithmeticOperator()
        ||  ctx->comparisonOperator()
        ||  ctx->logicalOperator()
        ||  ctx->unaryOperator()
        ||  ctx->assignmentOperator()
        ||  ctx->fieldAccessOperator()) compiler->shiftDepth(-1);
        if (ctx->fieldAccessOperator()) this->dotAccess = false;
    }

    void enterFieldAccessOperator(GenericParser::FieldAccessOperatorContext * ctx) override { }
    void exitFieldAccessOperator(GenericParser::FieldAccessOperatorContext * ctx) override { }

    void enterUnaryOperator(GenericParser::UnaryOperatorContext * ctx) override { }
    void exitUnaryOperator(GenericParser::UnaryOperatorContext * ctx) override { }

    void enterArithmeticOperator(GenericParser::ArithmeticOperatorContext * ctx) override { }
    void exitArithmeticOperator(GenericParser::ArithmeticOperatorContext * ctx) override { }

    void enterComparisonOperator(GenericParser::ComparisonOperatorContext * ctx) override { }
    void exitComparisonOperator(GenericParser::ComparisonOperatorContext * ctx) override { }

    void enterLogicalOperator(GenericParser::LogicalOperatorContext * ctx) override { }
    void exitLogicalOperator(GenericParser::LogicalOperatorContext * ctx) override { }

    void enterAssignmentOperator(GenericParser::AssignmentOperatorContext * ctx) override { }
    void exitAssignmentOperator(GenericParser::AssignmentOperatorContext * ctx) override { }

    void enterVector(GenericParser::VectorContext * ctx) override
    {
        // Presume the vector is always non-constant.
        compiler->makeNode(NodeType::Vector, 0);
        compiler->shiftDepth(+1);
    }
    
    void exitVector(GenericParser::VectorContext * ctx) override
    {
        compiler->shiftDepth(-1);
    }

    void enterVectorName(GenericParser::VectorNameContext * ctx) override { }
    void exitVectorName(GenericParser::VectorNameContext * ctx) override { }

    void enterVectorComponent(GenericParser::VectorComponentContext * ctx) override { }
    void exitVectorComponent(GenericParser::VectorComponentContext * ctx) override { }

    void enterDsgVar(GenericParser::DsgVarContext * ctx) override { }
    void exitDsgVar(GenericParser::DsgVarContext * ctx) override { }

    void enterDsgVarIdentifier(GenericParser::DsgVarIdentifierContext * ctx) override { }
    void exitDsgVarIdentifier(GenericParser::DsgVarIdentifierContext * ctx) override { }

    void enterActorReference(GenericParser::ActorReferenceContext * ctx) override
    {
        std::string name = ctx->getText();
        printf("actor: %s\n", name.c_str());
    }
    
    void exitActorReference(GenericParser::ActorReferenceContext * ctx) override { }

    void enterLiteral(GenericParser::LiteralContext * ctx) override { }
    void exitLiteral(GenericParser::LiteralContext * ctx) override { }

    void enterNumericLiteral(GenericParser::NumericLiteralContext * ctx) override { }
    void exitNumericLiteral(GenericParser::NumericLiteralContext * ctx) override { }

    void enterReservedWord(GenericParser::ReservedWordContext * ctx) override { }
    void exitReservedWord(GenericParser::ReservedWordContext * ctx) override { }

    void enterKeyword(GenericParser::KeywordContext * ctx) override { }
    void exitKeyword(GenericParser::KeywordContext * ctx) override { }

    void enterField(GenericParser::FieldContext * ctx) override
    {
        
    }
    
    void exitField(GenericParser::FieldContext * ctx) override { }

    void enterEveryRule(antlr4::ParserRuleContext * ctx) override { }
    void exitEveryRule(antlr4::ParserRuleContext * ctx) override { }
    void visitTerminal(antlr4::tree::TerminalNode * /*node*/) override { }
    void visitErrorNode(antlr4::tree::ErrorNode * /*node*/) override { }
};

void CompilerContext::loadTables()
{
    if (target == Target_R3_GC || target == Target_R3_PC)
    {
        nodeTypeTable = R3NodeTypes;
        keywordTable = R3Keywords;
        operatorTable = R3Operators;
        functionTable = R3Functions;
        procedureTable = R3Procedures;
        conditionTable = R3Conditions;
        fieldTable = R3Fields;
        metaActionTable = R3MetaActions;
        
        if (target == Target_R3_GC) // Gamecube adds a function with ID=219.
            procedureTable.insert(procedureTable.begin() + 219, "FixePositionPersoGamecubeExclusive");
    }
}

void CompilerContext::compile(std::string source)
{
    ANTLRInputStream input(source);
    
    GenericLexer lexer(&input);
    // Remove error listeners, as the lexer most likely
    // will generate lots of unnecessary warnings otherwise.
    lexer.removeErrorListeners();
        
    TreeShapeListener listener;
    CommonTokenStream tokens(&lexer);
    GenericParser parser(&tokens);

    listener.setCompiler(this);
    this->loadTables();
    
    tree::ParseTree *tree = parser.source();
    tree::ParseTreeWalker::DEFAULT.walk(&listener, tree);
}
