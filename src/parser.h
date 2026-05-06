#ifndef PARSER_H
#define PARSER_H

#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include "lexer.h" 

using namespace std;

// ---------------------------------------------------------
// STRUKTUR NODE PARSE TREE
// ---------------------------------------------------------
struct ParseTreeNode {
    string label;      // Nama non-terminal (ex: "<expression>") atau tipe token terminal (ex: "ident")
    string value;      // Isi dari token jika ada (ex: "angka1", "10", "+")
    vector<ParseTreeNode*> children; // Anak-anak dari node ini

    ParseTreeNode(string l, string v = "") : label(l), value(v) {}
    
    // Destructor buat ngebersihin memori rekursif (penting biar ga memory leak)
    ~ParseTreeNode() {
        for (auto child : children) {
            delete child;
        }
    }
};

// ---------------------------------------------------------
// KELAS PARSER UTAMA
// ---------------------------------------------------------
class Parser {
private:
    vector<Token> tokens;
    int currentTokenIndex;
    bool hasError; // Flag penanda kalau terjadi syntax error

    // =========================================================
    // CORE & EXPRESSION
    // =========================================================
    Token currentToken();
    Token peek(int offset = 1);
    void advance();
    ParseTreeNode* match(TokenType expectedType);
    void reportError(string expected, string found);

public:
    Parser(const vector<Token>& tokens);
    
    // =========================================================
    // CORE & EXPRESSION
    // =========================================================
    ParseTreeNode* parse(); // Entry point untuk mulai parsing
    void printTree(ParseTreeNode* node, ofstream& outFile, string indent = "", bool isLast = true, bool isRoot = true);
    bool isError() const { return hasError; }

    // =========================================================
    // TOP-LEVEL & DATA TYPES
    // =========================================================
    ParseTreeNode* parseProgram();
    ParseTreeNode* parseProgramHeader();
    ParseTreeNode* parseDeclarationPart();
    ParseTreeNode* parseConstDeclaration();
    ParseTreeNode* parseConstant();
    ParseTreeNode* parseTypeDeclaration();
    ParseTreeNode* parseType();
    ParseTreeNode* parseArrayType();
    ParseTreeNode* parseRange();
    ParseTreeNode* parseEnumerated();
    ParseTreeNode* parseRecordType();
    ParseTreeNode* parseFieldList();
    ParseTreeNode* parseFieldPart();
    ParseTreeNode* parseVarDeclaration();
    ParseTreeNode* parseIdentifierList();

    // =========================================================
    // BLOCK, STATEMENT DASAR, & SUBPROGRAMS
    // =========================================================
    ParseTreeNode* parseBlock();
    ParseTreeNode* parseCompoundStatement();
    ParseTreeNode* parseStatementList();
    ParseTreeNode* parseStatement();
    ParseTreeNode* parseAssignmentStatement();
    ParseTreeNode* parseVariable();
    ParseTreeNode* parseComponentVariable();
    ParseTreeNode* parseIndexList();
    ParseTreeNode* parseSubprogramDeclaration();
    ParseTreeNode* parseProcedureDeclaration();
    ParseTreeNode* parseFunctionDeclaration();
    ParseTreeNode* parseFormalParameterList();
    ParseTreeNode* parseParameterGroup();
    ParseTreeNode* parseProcedureFunctionCall();
    ParseTreeNode* parseParameterList();

    // =========================================================
    // CORE & EXPRESSION
    // =========================================================
    ParseTreeNode* parseIfStatement();
    ParseTreeNode* parseCaseStatement();
    ParseTreeNode* parseCaseBlock();
    ParseTreeNode* parseWhileStatement();
    ParseTreeNode* parseRepeatStatement();
    ParseTreeNode* parseForStatement();

    ParseTreeNode* parseExpression();
    ParseTreeNode* parseSimpleExpression();
    ParseTreeNode* parseTerm();
    ParseTreeNode* parseFactor();
};

#endif