#ifndef SEMANTIC_H
#define SEMANTIC_H

#include <iostream>
#include <vector>
#include <string>
#include <memory>

using namespace std;

// =========================================================
// 1. ENUMERASI UNTUK SEMANTIK
// =========================================================

// Kelas Objek (obj di tabel tab)
enum class ObjClass { 
    CONSTANT, VARIABLE, TYPE_DEF, PROCEDURE, FUNCTION
};

// Tipe Data Dasar (type di tabel tab)
// NOTYPE = belum diketahui/belum dianalisis, NONE = void (statement/procedure)
enum class DataType {
    NOTYPE, NONE, INTEGER, REAL, CHAR, BOOLEAN, STRING, ARRAY, RECORD
};

// =========================================================
// 2. STRUKTUR SYMBOL TABLE
// =========================================================

// atab: Array Table
struct ATabEntry {
    int arrays;     // Indeks entri
    DataType xtyp;  // Tipe indeks
    DataType etyp;  // Tipe elemen
    int eref;       // Referensi ke atab/btab jika elemen komposit
    int low;        // Batas bawah
    int high;       // Batas atas
    int elsz;       // Ukuran elemen
    int size;       // Total ukuran array
};

// btab: Block Table (Untuk Prosedur, Fungsi, Record)
struct BTabEntry {
    int blocks;     // Indeks entri
    int last;       // Indeks identifier terakhir di scope ini (link ke tab)
    int lpar;       // Indeks parameter terakhir (link ke tab)
    int psze;       // Total ukuran parameter (byte)
    int vsze;       // Total ukuran variabel lokal (byte)
};

// tab: Main Symbol Table
struct TabEntry {
    string identifiers; // Nama ident
    int link;           // Link ke ident sebelumnya di scope yg sama
    ObjClass obj;       // Class: Var, Const, dll
    DataType type;      // Tipe: Int, Real, dll
    int ref;            // Pointer ke atab (jika array) atau btab (jika proc/rec)
    int nrm;            // 1 = Normal, 0 = By-Reference (Var Parameter)
    int lev;            // Lexical level (0: Global, 1: Lokal, dst)
    int adr;            // Address/Offset/Value
};

// =========================================================
// 3. SYMBOL TABLE MANAGER
// =========================================================

class SymbolTable {
public:
    vector<TabEntry> tab;
    vector<BTabEntry> btab;
    vector<ATabEntry> atab;
    int currentLevel;

    SymbolTable() { currentLevel = 0; }

    // Manajemen Scope
    void pushScope() { currentLevel++; }
    void popScope() { currentLevel--; }

    // Fungsi Utama Tabel tab
    TabEntry* lookupTab(string name);      
    TabEntry* lookupLocalTab(string name); 
    int insertTab(string name, ObjClass obj, DataType type, int ref = 0, int lev = -1, int adr = 0);

    // Fungsi Tabel atab (Array)
    int insertATab(DataType xtyp, DataType etyp, int eref, int low, int high, int elsz);
    ATabEntry* getATab(int index);

    // Fungsi Tabel btab (Block/Record)
    int insertBTab();
    void updateBTab(int bIndex, int last, int lpar, int psze, int vsze);
    BTabEntry* getBTab(int index);

    // Inisialisasi Predefined (Integer, Real, True, False, dll)
    void initPredefined();
};

// =========================================================
// 4. BASE CLASS AST & VISITOR INTERFACE
// =========================================================

class SemanticVisitor;

class ASTNode {
public:
    // Decorated AST Annotations
    DataType evalType;  // Tipe hasil evaluasi
    int symRef;         // Link ke index tabel tab
    int lexicalLevel;   // Level scope saat ini

    ASTNode() : evalType(DataType::NOTYPE), symRef(-1), lexicalLevel(0) {}
    virtual ~ASTNode() {}
    virtual void accept(SemanticVisitor* visitor) = 0;
};

// =========================================================
// 5. DAFTAR AST NODES
// =========================================================

// ---------------------------------------------------------
// DONGUN (Modul Deklarasi & Program Root)
// ---------------------------------------------------------
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
    int ref; // Jika array/record
    void accept(SemanticVisitor* visitor) override;
};

class ConstDeclNode : public ASTNode {
public:
    string name;
    DataType type;
    string value;
    void accept(SemanticVisitor* visitor) override;
};

// Gabungan Procedure & Function
class SubprogDeclNode : public ASTNode {
public:
    string name;
    bool isFunction; 
    DataType retType; // Kalau procedure, isi DataType::NOTYPE
    vector<ASTNode*> params;
    ASTNode* block;
    void accept(SemanticVisitor* visitor) override;
};

// ---------------------------------------------------------
// NELSON (Modul Statement & Control Flow)
// ---------------------------------------------------------
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

// ---------------------------------------------------------
// RAMA (Modul Ekspresi & Pengecekan Tipe Data)
// ---------------------------------------------------------
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
    vector<ASTNode*> indices; // Untuk pemanggilan array
    string fieldName;         // Untuk pemanggilan record
    void accept(SemanticVisitor* visitor) override;
};

class FuncCallNode : public ASTNode {
public:
    string name;
    vector<ASTNode*> args;
    void accept(SemanticVisitor* visitor) override;
};

// =========================================================
// 6. SEMANTIC VISITOR INTERFACE
// =========================================================

class SemanticVisitor {
public:
    SymbolTable st;

    // --- Dongun ---
    virtual void visit(ProgramNode* node) = 0;
    virtual void visit(VarDeclNode* node) = 0;
    virtual void visit(ConstDeclNode* node) = 0;
    virtual void visit(SubprogDeclNode* node) = 0;

    // --- Nelson ---
    virtual void visit(CompoundStmtNode* node) = 0;
    virtual void visit(AssignStmtNode* node) = 0;
    virtual void visit(IfStmtNode* node) = 0;
    virtual void visit(WhileStmtNode* node) = 0;
    virtual void visit(ForStmtNode* node) = 0;

    // --- Rama ---
    virtual void visit(BinaryOpNode* node) = 0;
    virtual void visit(UnaryOpNode* node) = 0;
    virtual void visit(LiteralNode* node) = 0;
    virtual void visit(VarAccessNode* node) = 0;
    virtual void visit(FuncCallNode* node) = 0;
};

// =========================================================
// 7. SEMANTIC ANALYZER (Implementasi Visitor)
// =========================================================
//

class SemanticAnalyzer : public SemanticVisitor {
public:
    // --- Dongun (semantic_decl.cpp) ---
    void visit(ProgramNode* node) override;
    void visit(VarDeclNode* node) override;
    void visit(ConstDeclNode* node) override;
    void visit(SubprogDeclNode* node) override;

    // --- Nelson (semantic_stmt.cpp) ---
    void visit(CompoundStmtNode* node) override;
    void visit(AssignStmtNode* node) override;
    void visit(IfStmtNode* node) override;
    void visit(WhileStmtNode* node) override;
    void visit(ForStmtNode* node) override;

    // --- Rama (semantic_expr.cpp) ---
    void visit(BinaryOpNode* node) override;
    void visit(UnaryOpNode* node) override;
    void visit(LiteralNode* node) override;
    void visit(VarAccessNode* node) override;
    void visit(FuncCallNode* node) override;
};

#endif // SEMANTIC_H
