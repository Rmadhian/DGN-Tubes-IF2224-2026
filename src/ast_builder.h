#ifndef AST_BUILDER_H
#define AST_BUILDER_H

#include "parser.h"
#include "semantic.h"
#include <vector>
#include <string>

using namespace std;

class ASTBuilder {
public:
    // Fungsi utama untuk mengubah ParseTreeNode root (<program>) menjadi ProgramNode AST
    ProgramNode* build(ParseTreeNode* parseTreeRoot);

private:
    // --- Pembentuk Deklarasi ---
    void buildDeclarations(ParseTreeNode* declPart, vector<ASTNode*>& declList);
    void buildVarDeclaration(ParseTreeNode* varDecl, vector<ASTNode*>& declList);
    void buildConstDeclaration(ParseTreeNode* constDecl, vector<ASTNode*>& declList);
    void buildSubprogramDeclaration(ParseTreeNode* subprogDecl, vector<ASTNode*>& declList);

    // --- Pembentuk Statement ---
    CompoundStmtNode* buildCompoundStatement(ParseTreeNode* compStmt);
    ASTNode* buildStatement(ParseTreeNode* stmt);
    AssignStmtNode* buildAssignment(ParseTreeNode* assignStmt);
    IfStmtNode* buildIf(ParseTreeNode* ifStmt);
    WhileStmtNode* buildWhile(ParseTreeNode* whileStmt);
    ForStmtNode* buildFor(ParseTreeNode* forStmt);
    FuncCallNode* buildProcedureFunctionCall(ParseTreeNode* callStmt);
    
    // --- Pembentuk Ekspresi ---
    ASTNode* buildExpression(ParseTreeNode* expr);
    ASTNode* buildSimpleExpression(ParseTreeNode* simpleExpr);
    ASTNode* buildTerm(ParseTreeNode* term);
    ASTNode* buildFactor(ParseTreeNode* factor);
    
    // --- Helper / Evaluator Dasar ---
    VarAccessNode* buildVariable(ParseTreeNode* varNode);
    DataType stringToDataType(string typeStr);
    vector<string> extractIdentifierList(ParseTreeNode* idList);
};

#endif