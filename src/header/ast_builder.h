#ifndef AST_BUILDER_H
#define AST_BUILDER_H

#include "parser.h"
#include "semantic.h"
#include <vector>
#include <string>

using namespace std;

class ASTBuilder {
public:
    // Entry point: konversi parse tree <program> menjadi AST
    ProgramNode* build(ParseTreeNode* parseTreeRoot);

private:
    // Deklarasi
    void buildDeclarations(ParseTreeNode* declPart, vector<ASTNode*>& declList);
    void buildVarDeclaration(ParseTreeNode* varDecl, vector<ASTNode*>& declList);
    void buildConstDeclaration(ParseTreeNode* constDecl, vector<ASTNode*>& declList);
    void buildSubprogramDeclaration(ParseTreeNode* subprogDecl, vector<ASTNode*>& declList);

    // Statement
    CompoundStmtNode* buildCompoundStatement(ParseTreeNode* compStmt);
    ASTNode* buildStatement(ParseTreeNode* stmt);
    AssignStmtNode* buildAssignment(ParseTreeNode* assignStmt);
    IfStmtNode* buildIf(ParseTreeNode* ifStmt);
    WhileStmtNode* buildWhile(ParseTreeNode* whileStmt);
    ForStmtNode* buildFor(ParseTreeNode* forStmt);
    FuncCallNode* buildProcedureFunctionCall(ParseTreeNode* callStmt);
    
    // Ekspresi
    ASTNode* buildExpression(ParseTreeNode* expr);
    ASTNode* buildSimpleExpression(ParseTreeNode* simpleExpr);
    ASTNode* buildTerm(ParseTreeNode* term);
    ASTNode* buildFactor(ParseTreeNode* factor);
    
    // Helper
    VarAccessNode* buildVariable(ParseTreeNode* varNode);
    DataType stringToDataType(string typeStr);
    DataType resolveType(ParseTreeNode* typeNode);
    vector<string> extractIdentifierList(ParseTreeNode* idList);
};

#endif