#ifndef AST_BUILDER_H
#define AST_BUILDER_H

#include "parser.h"
#include "semantic.h"
#include <vector>
#include <string>

using namespace std;

// =========================================================
// ASTBuilder: Konversi Parse Tree → AST
// Menggunakan Syntax-Directed Translation (L-Attributed Grammar)
// =========================================================

class ASTBuilder {
public:
    // Entry point: konversi parse tree <program> menjadi AST
    ProgramNode* build(ParseTreeNode* parseTreeRoot);

private:
    // --- Deklarasi ---
    void buildDeclarations(ParseTreeNode* declPart, vector<ASTNode*>& declList);
    void buildVarDeclaration(ParseTreeNode* varDecl, vector<ASTNode*>& declList);
    void buildConstDeclaration(ParseTreeNode* constDecl, vector<ASTNode*>& declList);
    void buildSubprogramDeclaration(ParseTreeNode* subprogDecl, vector<ASTNode*>& declList);
    void buildProcedureDeclaration(ParseTreeNode* procNode, SubprogDeclNode* funcNode);
    void buildFunctionDeclaration(ParseTreeNode* funcDeclNode, SubprogDeclNode* funcNode);
    void buildFormalParams(ParseTreeNode* paramList, vector<ASTNode*>& params);
    void buildParameterGroup(ParseTreeNode* paramGroup, vector<ASTNode*>& params);
    void buildBlock(ParseTreeNode* blockNode, SubprogDeclNode* funcNode);

    // Helper tipe
    void buildTypeInfo(ParseTreeNode* typeNode, VarDeclNode* vNode);
    void scanArrayType(ParseTreeNode* node, VarDeclNode* vNode);
    void extractRangeBounds(ParseTreeNode* rangeNode, int& low, int& high);

    // --- Statement ---
    CompoundStmtNode* buildCompoundStatement(ParseTreeNode* compStmt);
    ASTNode* buildStatement(ParseTreeNode* stmt);
    AssignStmtNode* buildAssignment(ParseTreeNode* assignStmt);
    IfStmtNode* buildIf(ParseTreeNode* ifStmt);
    WhileStmtNode* buildWhile(ParseTreeNode* whileStmt);
    ForStmtNode* buildFor(ParseTreeNode* forStmt);
    ASTNode* buildRepeat(ParseTreeNode* repeatStmt);
    FuncCallNode* buildProcedureFunctionCall(ParseTreeNode* callStmt);

    // --- Ekspresi ---
    ASTNode* buildExpression(ParseTreeNode* expr);
    ASTNode* buildSimpleExpression(ParseTreeNode* simpleExpr);
    ASTNode* buildTerm(ParseTreeNode* term);
    ASTNode* buildFactor(ParseTreeNode* factor);

    // --- Helper ---
    VarAccessNode* buildVariable(ParseTreeNode* varNode);
    DataType stringToDataType(const string& typeStr);
    vector<string> extractIdentifierList(ParseTreeNode* idList);
};

#endif // AST_BUILDER_H