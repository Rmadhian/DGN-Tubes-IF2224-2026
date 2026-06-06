#ifndef AST_PARSER_H
#define AST_PARSER_H

#include <iostream>
#include <string>
#include <vector>
#include "semantic.h"

class ASTParser {
public:
    ASTParser(const std::string& fileContent);
    void parse();
    
    SymbolTable getSymbolTable() const { return st; }
    ProgramNode* getASTRoot() const { return root; }
    bool isError() const { return error; }

private:
    std::string content;
    SymbolTable st;
    ProgramNode* root;
    bool error;

    std::vector<std::string> splitLines(const std::string& str);
    std::string trim(const std::string& str);
    std::vector<std::string> splitWhitespace(const std::string& str);


    void parseDecoratedAST(const std::vector<std::string>& lines, size_t& i);
    void buildSymbolTableFromTree(); // Derive symbol table dari tree tanpa tab/btab/atab
    
    ASTNode* parseASTNode(const std::string& line);
    ObjClass strToObjClass(const std::string& str);

    // Tracking variabel yang ditemukan di tree
    std::vector<std::string> varDeclNames;
};

#endif
