#ifndef PARSER_H
#define PARSER_H

#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include "lexer.h" 

using namespace std;

struct ParseTreeNode {
    string label;
    string value;
    vector<ParseTreeNode*> children;
    ParseTreeNode(string l, string v = "") : label(l), value(v) {}
    ~ParseTreeNode() {
        for (auto child : children) {
            delete child;
        }
    }
};


class Parser {
private:
    vector<Token> tokens;
    int currentTokenIndex;
    bool hasError;
    Token currentToken();
    Token peek(int offset = 1);
    void advance();
    ParseTreeNode* match(TokenType expectedType);
    void reportError(string expected, string found);

public:
    Parser(const vector<Token>& tokens);
    ParseTreeNode* parse();
    void printTree(ParseTreeNode* node, ofstream& outFile, string indent = "", bool isLast = true, bool isRoot = true);
    bool isError() const { return hasError; }

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