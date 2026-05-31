#ifndef SEMANTIC_H
#define SEMANTIC_H

#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <algorithm>

using namespace std;

// Enumerasi Semantik

enum class ObjClass { 
    CONSTANT, VARIABLE, TYPE_DEF, PROCEDURE, FUNCTION
};

// NOTYPE = belum ditentukan, NONE = void
enum class DataType {
    NOTYPE, NONE, INTEGER, REAL, CHAR, BOOLEAN, STRING, ARRAY, RECORD, SUBRANGE, ENUMERATED
};

// Struktur Symbol Table

// Array Table: menyimpan metadata tipe array
struct ATabEntry {
    int arrays;     // Indeks entri
    DataType xtyp;  // Tipe indeks
    DataType etyp;  // Tipe elemen
    int eref;       // Referensi ke atab/btab jika elemen komposit
    int low;        // Batas bawah
    int high;       // Batas atas
    int elsz;       // Ukuran per elemen
    int size;       // Total ukuran array
};

// Block Table: metadata scope prosedur, fungsi, dan record
struct BTabEntry {
    int blocks;     // Indeks entri
    int last;       // Indeks identifier terakhir di scope ini
    int lpar;       // Indeks parameter terakhir
    int psze;       // Ukuran total parameter (byte)
    int vsze;       // Ukuran total variabel lokal (byte)
};

// Main Symbol Table: satu entri per identifier
struct TabEntry {
    string identifiers;
    int link;           // Link ke identifier sebelumnya di scope yang sama
    ObjClass obj;
    DataType type;
    int ref;            // Indeks ke atab (array) atau btab (proc/rec)
    int nrm;            // 1 = pass-by-value, 0 = pass-by-reference
    int lev;            // Lexical level (0 = global)
    int adr;            // Alamat/offset/nilai
};

// Symbol Table Manager

class SymbolTable {
public:
    vector<TabEntry> tab;
    vector<BTabEntry> btab;
    vector<ATabEntry> atab;
    int currentLevel;
    vector<int> activeBlocks;

    SymbolTable() { currentLevel = 0; }

    void pushScope() { currentLevel++; }
    void popScope() { currentLevel--; }

    // Operasi pada tab
    TabEntry* lookupTab(string name);      
    TabEntry* lookupLocalTab(string name); 
    TabEntry* getTab(int index); 
    int insertTab(string name, ObjClass obj, DataType type, int ref = 0, int lev = -1, int adr = 0);

    // Operasi pada atab
    int insertATab(DataType xtyp, DataType etyp, int eref, int low, int high, int elsz);
    ATabEntry* getATab(int index);

    // Operasi pada btab
    int insertBTab();
    void updateBTab(int bIndex, int last, int lpar, int psze, int vsze);
    BTabEntry* getBTab(int index);

    void initPredefined();
};

// Forward declaration
class SemanticVisitor;
class WriteStatementNode;

// Base Class AST & Visitor Interface
class ASTNode {
public:
    DataType evalType; 
    int symRef;        
    int lexicalLevel;

    ASTNode() : evalType(DataType::NOTYPE), symRef(-1), lexicalLevel(0) {}
    virtual ~ASTNode() {}
    virtual void accept(SemanticVisitor* visitor) = 0;
};

// AST Node Definitions

// Deklarasi & Program Root 

class ProgramNode : public ASTNode {
public:
    string name;
    vector<ASTNode*> declarations;
    ASTNode* mainBlock;
    void accept(SemanticVisitor* visitor) override;
};

class VarDeclNode : public ASTNode {
public:
    vector<string> idents;
    DataType type;
    int ref;
    DataType elementType = DataType::NOTYPE;
    int lowBound = 0;
    int highBound = 0;
    void accept(SemanticVisitor* visitor) override;
};

class ConstDeclNode : public ASTNode {
public:
    string name;
    DataType type;
    string value;
    void accept(SemanticVisitor* visitor) override;
};

class SubprogDeclNode : public ASTNode {
public:
    string name;
    bool isFunction; 
    DataType retType;
    vector<ASTNode*> params;
    ASTNode* block;
    void accept(SemanticVisitor* visitor) override;
};

// Statement & Control Flow

class CompoundStmtNode : public ASTNode {
public:
    vector<ASTNode*> statements;
    void accept(SemanticVisitor* visitor) override;
};

class AssignStmtNode : public ASTNode {
public:
    ASTNode* left;
    ASTNode* right;
    void accept(SemanticVisitor* visitor) override;
};

class IfStmtNode : public ASTNode {
public:
    ASTNode* condition;
    ASTNode* thenStmt;
    ASTNode* elseStmt;
    void accept(SemanticVisitor* visitor) override;
};

class WhileStmtNode : public ASTNode {
public:
    ASTNode* condition;
    ASTNode* body;
    void accept(SemanticVisitor* visitor) override;
};

class ForStmtNode : public ASTNode {
public:
    string iterVar;
    ASTNode* startExpr;
    ASTNode* endExpr;
    bool isDownto;
    ASTNode* body;
    void accept(SemanticVisitor* visitor) override;
};

// Ekspresi & Type Checking

class BinaryOpNode : public ASTNode {
public:
    ASTNode* left;
    string op;
    ASTNode* right;
    void accept(SemanticVisitor* visitor) override;
};

class UnaryOpNode : public ASTNode {
public:
    string op;
    ASTNode* operand;
    void accept(SemanticVisitor* visitor) override;
};

class LiteralNode : public ASTNode {
public:
    string value;
    DataType literalType;
    void accept(SemanticVisitor* visitor) override;
};

class VarAccessNode : public ASTNode {
public:
    string name;
    vector<ASTNode*> indices; // Subscript array
    string fieldName;         // Akses field record
    void accept(SemanticVisitor* visitor) override;
};

class FuncCallNode : public ASTNode {
public:
    string name;
    vector<ASTNode*> args;
    void accept(SemanticVisitor* visitor) override;
};

// Node khusus untuk statement write/writeln (dipakai ICG supaya bisa bedain dari FuncCall biasa)
class WriteStatementNode : public ASTNode {
public:
    bool hasNewline; // true = writeln, false = write
    vector<ASTNode*> args;
    void accept(SemanticVisitor* visitor) override;
};

// Semantic Visitor Interface
class SemanticVisitor {
public:
    SymbolTable st;

    // Deklarasi
    virtual void visit(ProgramNode* node) = 0;
    virtual void visit(VarDeclNode* node) = 0;
    virtual void visit(ConstDeclNode* node) = 0;
    virtual void visit(SubprogDeclNode* node) = 0;

    // Statement
    virtual void visit(CompoundStmtNode* node) = 0;
    virtual void visit(AssignStmtNode* node) = 0;
    virtual void visit(IfStmtNode* node) = 0;
    virtual void visit(WhileStmtNode* node) = 0;
    virtual void visit(ForStmtNode* node) = 0;

    // Ekspresi
    virtual void visit(BinaryOpNode* node) = 0;
    virtual void visit(UnaryOpNode* node) = 0;
    virtual void visit(LiteralNode* node) = 0;
    virtual void visit(VarAccessNode* node) = 0;
    virtual void visit(FuncCallNode* node) = 0;

    // Output statement (default kosong biar SemanticAnalyzer/PrintAST gak perlu implement)
    virtual void visit(WriteStatementNode* node) {}
};

class SemanticAnalyzer : public SemanticVisitor {
public:
    void visit(ProgramNode* node) override;
    void visit(VarDeclNode* node) override;
    void visit(ConstDeclNode* node) override;
    void visit(SubprogDeclNode* node) override;

    void visit(CompoundStmtNode* node) override;
    void visit(AssignStmtNode* node) override;
    void visit(IfStmtNode* node) override;
    void visit(WhileStmtNode* node) override;
    void visit(ForStmtNode* node) override;

    void visit(BinaryOpNode* node) override;
    void visit(UnaryOpNode* node) override;
    void visit(LiteralNode* node) override;
    void visit(VarAccessNode* node) override;
    void visit(FuncCallNode* node) override;
};

class PrintASTVisitor : public SemanticVisitor {
private:
    std::ostream& out;
    std::vector<bool> isLastChildStack;
    
    void printPrefix(bool isLast);
    std::string typeToStr(DataType t);

public:
    PrintASTVisitor(std::ostream& outputStream) : out(outputStream) {}

    void visit(ProgramNode* node) override;
    void visit(VarDeclNode* node) override;
    void visit(ConstDeclNode* node) override;
    void visit(SubprogDeclNode* node) override;

    void visit(CompoundStmtNode* node) override;
    void visit(AssignStmtNode* node) override;
    void visit(IfStmtNode* node) override;
    void visit(WhileStmtNode* node) override;
    void visit(ForStmtNode* node) override;

    void visit(BinaryOpNode* node) override;
    void visit(UnaryOpNode* node) override;
    void visit(LiteralNode* node) override;
    void visit(VarAccessNode* node) override;
    void visit(FuncCallNode* node) override;
};

// Deklarasi fungsi cetak Symbol Table
void printSymbolTables(const SymbolTable& st, std::ostream& out);

#endif // SEMANTIC_H
