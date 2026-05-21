#ifndef SEMANTIC_H
#define SEMANTIC_H

#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <algorithm>

using namespace std;

// =========================================================
// Enumerasi Semantik
// =========================================================

enum class ObjClass { 
    CONSTANT, VARIABLE, TYPE_DEF, PROCEDURE, FUNCTION
};

// NOTYPE = belum ditentukan, NONE = void (return type prosedur)
enum class DataType {
    NOTYPE, NONE, INTEGER, REAL, CHAR, BOOLEAN, STRING, ARRAY, RECORD
};

// =========================================================
// Struktur Symbol Table (tab, atab, btab)
// =========================================================

// Array Table: menyimpan metadata tipe array
struct ATabEntry {
    int arrays;     // Indeks entri
    DataType xtyp;  // Tipe indeks array (harus ordinal, bukan Real)
    DataType etyp;  // Tipe elemen array
    int eref;       // Referensi ke atab/btab jika elemen komposit
    int low;        // Batas bawah indeks
    int high;       // Batas atas indeks
    int elsz;       // Ukuran per elemen (byte/unit memori)
    int size;       // Total ukuran array = (high-low+1)*elsz
};

// Block Table: metadata scope prosedur, fungsi, dan record
struct BTabEntry {
    int blocks;     // Indeks entri block
    int last;       // Indeks identifier terakhir yang dideklarasikan di block ini
    int lpar;       // Indeks parameter terakhir prosedur/fungsi (0 jika record)
    int psze;       // Total ukuran parameter (byte/unit memori)
    int vsze;       // Total ukuran variabel lokal (byte/unit memori)
};

// Main Symbol Table: satu entri per identifier
struct TabEntry {
    string identifiers;  // Nama identifier
    int link;            // Pointer ke identifier sebelumnya dalam scope yang sama
    ObjClass obj;        // Kelas objek (CONSTANT, VARIABLE, TYPE_DEF, PROCEDURE, FUNCTION)
    DataType type;       // Tipe dasar identifier
    int ref;             // Indeks ke atab (array) atau btab (procedure/record)
    int nrm;             // 1 = pass-by-value (normal), 0 = pass-by-reference (VAR param)
    int lev;             // Lexical level (0 = global, 1 = dalam prosedur, dst.)
    int adr;             // Offset/nilai/alamat (tergantung jenis objek)
};

// =========================================================
// Symbol Table Manager
// =========================================================

class SymbolTable {
public:
    vector<TabEntry>  tab;    // Main identifier table (indeks dimulai dari 0)
    vector<BTabEntry> btab;   // Block table
    vector<ATabEntry> atab;   // Array table
    int currentLevel;         // Lexical level saat ini

    SymbolTable() { currentLevel = 0; }

    // Manajemen scope
    void pushScope() { currentLevel++; }
    void popScope()  { currentLevel--; }

    // Operasi pada tab
    TabEntry* lookupTab(const string& name);       // Lookup dari dalam ke luar scope
    TabEntry* lookupLocalTab(const string& name);  // Lookup di scope lokal saja
    TabEntry* getTab(int index);
    int insertTab(const string& name, ObjClass obj, DataType type,
                  int ref = 0, int lev = -1, int adr = 0, int nrm = 1);

    // Operasi pada atab
    int insertATab(DataType xtyp, DataType etyp, int eref, int low, int high, int elsz);
    ATabEntry* getATab(int index);

    // Operasi pada btab
    int insertBTab();
    void updateBTab(int bIndex, int last, int lpar, int psze, int vsze);
    BTabEntry* getBTab(int index);

    // Inisialisasi predefined identifiers (tipe dasar + konstanta boolean)
    void initPredefined();

    // Helper untuk output symbol table
    string dataTypeName(DataType t) const;
    string objClassName(ObjClass o) const;
    void printTab(ostream& out = cout)  const;
    void printBTab(ostream& out = cout) const;
    void printATab(ostream& out = cout) const;
};

// =========================================================
// Forward declarations
// =========================================================

class SemanticVisitor;

// =========================================================
// Base Class AST Node
// =========================================================

class ASTNode {
public:
    DataType evalType;   // Tipe hasil evaluasi node
    int symRef;          // Indeks ke tab (symbol table reference)
    int lexicalLevel;    // Lexical level tempat node berada

    ASTNode() : evalType(DataType::NOTYPE), symRef(-1), lexicalLevel(0) {}
    virtual ~ASTNode() {}
    virtual void accept(SemanticVisitor* visitor) = 0;

    // Helper untuk pretty-print decorated AST
    virtual void print(ostream& out, int indent = 0, const SymbolTable* st = nullptr) const = 0;

public:
    // Utility: print indentasi
    static void printIndent(ostream& out, int indent) {
        for (int i = 0; i < indent; i++) out << "  ";
    }
    // Utility: DataType ke string (untuk output)
    static string dtStr(DataType t) {
        switch(t) {
            case DataType::INTEGER: return "Integer";
            case DataType::REAL:    return "Real";
            case DataType::CHAR:    return "Char";
            case DataType::BOOLEAN: return "Boolean";
            case DataType::STRING:  return "String";
            case DataType::ARRAY:   return "Array";
            case DataType::RECORD:  return "Record";
            case DataType::NONE:    return "None";
            default:                return "Unknown";
        }
    }
};

// =========================================================
// AST Node Definitions
// =========================================================

// --- Program Root ---

class ProgramNode : public ASTNode {
public:
    string name;
    vector<ASTNode*> declarations;
    ASTNode* mainBlock;

    ProgramNode() : mainBlock(nullptr) {}
    void accept(SemanticVisitor* visitor) override;
    void print(ostream& out, int indent = 0, const SymbolTable* st = nullptr) const override;
};

// --- Deklarasi ---

class VarDeclNode : public ASTNode {
public:
    vector<string> idents;      // Daftar nama variabel
    DataType type;              // Tipe variabel
    int ref;                    // Indeks atab (jika array)
    DataType elementType;       // Tipe elemen (jika array)
    DataType indexType;         // Tipe indeks (jika array)
    int lowBound;               // Batas bawah (jika array)
    int highBound;              // Batas atas (jika array)

    VarDeclNode()
        : type(DataType::NOTYPE), ref(0),
          elementType(DataType::NOTYPE),
          indexType(DataType::INTEGER),
          lowBound(0), highBound(0) {}

    void accept(SemanticVisitor* visitor) override;
    void print(ostream& out, int indent = 0, const SymbolTable* st = nullptr) const override;
};

class ConstDeclNode : public ASTNode {
public:
    string name;        // Nama konstanta
    DataType type;      // Tipe konstanta
    string value;       // Nilai konstanta (sebagai string)

    ConstDeclNode() : type(DataType::NOTYPE) {}
    void accept(SemanticVisitor* visitor) override;
    void print(ostream& out, int indent = 0, const SymbolTable* st = nullptr) const override;
};

class SubprogDeclNode : public ASTNode {
public:
    string name;
    bool isFunction;           // true = function, false = procedure
    DataType retType;          // Tipe kembalian (NONE untuk procedure)
    vector<ASTNode*> params;   // Parameter (sebagai VarDeclNode)
    ASTNode* block;            // Compound statement body

    SubprogDeclNode() : isFunction(false), retType(DataType::NONE), block(nullptr) {}
    void accept(SemanticVisitor* visitor) override;
    void print(ostream& out, int indent = 0, const SymbolTable* st = nullptr) const override;
};

// --- Statement & Control Flow ---

class CompoundStmtNode : public ASTNode {
public:
    vector<ASTNode*> statements;
    void accept(SemanticVisitor* visitor) override;
    void print(ostream& out, int indent = 0, const SymbolTable* st = nullptr) const override;
};

class AssignStmtNode : public ASTNode {
public:
    ASTNode* left;    // Variabel tujuan
    ASTNode* right;   // Ekspresi nilai
    AssignStmtNode() : left(nullptr), right(nullptr) {}
    void accept(SemanticVisitor* visitor) override;
    void print(ostream& out, int indent = 0, const SymbolTable* st = nullptr) const override;
};

class IfStmtNode : public ASTNode {
public:
    ASTNode* condition;
    ASTNode* thenStmt;
    ASTNode* elseStmt;  // nullptr jika tidak ada else
    IfStmtNode() : condition(nullptr), thenStmt(nullptr), elseStmt(nullptr) {}
    void accept(SemanticVisitor* visitor) override;
    void print(ostream& out, int indent = 0, const SymbolTable* st = nullptr) const override;
};

class WhileStmtNode : public ASTNode {
public:
    ASTNode* condition;
    ASTNode* body;
    WhileStmtNode() : condition(nullptr), body(nullptr) {}
    void accept(SemanticVisitor* visitor) override;
    void print(ostream& out, int indent = 0, const SymbolTable* st = nullptr) const override;
};

class ForStmtNode : public ASTNode {
public:
    string iterVar;      // Nama variabel iterator
    ASTNode* startExpr;
    ASTNode* endExpr;
    bool isDownto;       // true = downto, false = to
    ASTNode* body;
    ForStmtNode() : startExpr(nullptr), endExpr(nullptr), isDownto(false), body(nullptr) {}
    void accept(SemanticVisitor* visitor) override;
    void print(ostream& out, int indent = 0, const SymbolTable* st = nullptr) const override;
};

// --- Ekspresi ---

class BinaryOpNode : public ASTNode {
public:
    ASTNode* left;
    string op;
    ASTNode* right;
    BinaryOpNode() : left(nullptr), right(nullptr) {}
    void accept(SemanticVisitor* visitor) override;
    void print(ostream& out, int indent = 0, const SymbolTable* st = nullptr) const override;
};

class UnaryOpNode : public ASTNode {
public:
    string op;
    ASTNode* operand;
    UnaryOpNode() : operand(nullptr) {}
    void accept(SemanticVisitor* visitor) override;
    void print(ostream& out, int indent = 0, const SymbolTable* st = nullptr) const override;
};

class LiteralNode : public ASTNode {
public:
    string value;
    DataType literalType;
    LiteralNode() : literalType(DataType::NOTYPE) {}
    void accept(SemanticVisitor* visitor) override;
    void print(ostream& out, int indent = 0, const SymbolTable* st = nullptr) const override;
};

class VarAccessNode : public ASTNode {
public:
    string name;
    vector<ASTNode*> indices;  // Subscript array (bisa kosong)
    string fieldName;          // Akses field record (bisa kosong)
    void accept(SemanticVisitor* visitor) override;
    void print(ostream& out, int indent = 0, const SymbolTable* st = nullptr) const override;
};

class FuncCallNode : public ASTNode {
public:
    string name;
    vector<ASTNode*> args;
    void accept(SemanticVisitor* visitor) override;
    void print(ostream& out, int indent = 0, const SymbolTable* st = nullptr) const override;
};

// =========================================================
// Semantic Visitor Interface
// =========================================================

class SemanticVisitor {
public:
    SymbolTable st;
    bool hasError;

    SemanticVisitor() : hasError(false) {}
    virtual ~SemanticVisitor() {}

    // --- Deklarasi ---
    virtual void visit(ProgramNode*      node) = 0;
    virtual void visit(VarDeclNode*      node) = 0;
    virtual void visit(ConstDeclNode*    node) = 0;
    virtual void visit(SubprogDeclNode*  node) = 0;

    // --- Statement ---
    virtual void visit(CompoundStmtNode* node) = 0;
    virtual void visit(AssignStmtNode*   node) = 0;
    virtual void visit(IfStmtNode*       node) = 0;
    virtual void visit(WhileStmtNode*    node) = 0;
    virtual void visit(ForStmtNode*      node) = 0;

    // --- Ekspresi ---
    virtual void visit(BinaryOpNode*     node) = 0;
    virtual void visit(UnaryOpNode*      node) = 0;
    virtual void visit(LiteralNode*      node) = 0;
    virtual void visit(VarAccessNode*    node) = 0;
    virtual void visit(FuncCallNode*     node) = 0;

public:
    // Helper: catat error semantik
    void semanticError(const string& msg) {
        cerr << "[Semantic Error] " << msg << endl;
        hasError = true;
    }
    // Helper: catat warning semantik
    void semanticWarning(const string& msg) {
        cerr << "[Semantic Warning] " << msg << endl;
    }
};

// =========================================================
// Concrete Semantic Analyzer (implements Visitor)
// =========================================================

class SemanticAnalyzer : public SemanticVisitor {
public:
    // --- Deklarasi ---
    void visit(ProgramNode*      node) override;
    void visit(VarDeclNode*      node) override;
    void visit(ConstDeclNode*    node) override;
    void visit(SubprogDeclNode*  node) override;

    // --- Statement ---
    void visit(CompoundStmtNode* node) override;
    void visit(AssignStmtNode*   node) override;
    void visit(IfStmtNode*       node) override;
    void visit(WhileStmtNode*    node) override;
    void visit(ForStmtNode*      node) override;

    // --- Ekspresi ---
    void visit(BinaryOpNode*     node) override;
    void visit(UnaryOpNode*      node) override;
    void visit(LiteralNode*      node) override;
    void visit(VarAccessNode*    node) override;
    void visit(FuncCallNode*     node) override;

private:
    // --- Type Compatibility Helpers ---

    // Cek apakah tipe T2 assignment-compatible dengan T1
    // (T1 Real & T2 Integer diperbolehkan, sama tipe diperbolehkan)
    bool isAssignmentCompatible(DataType t1, DataType t2) const;

    // Cek apakah tipe adalah ordinal (valid untuk iterator for-loop)
    bool isOrdinalType(DataType t) const;

    // Cek apakah tipe numerik (Integer atau Real)
    bool isNumericType(DataType t) const;

    // Cek apakah tipe comparable dengan operator relasional
    bool isComparableType(DataType t) const;

    // Hitung tipe hasil operasi aritmatika (integer+real promotion)
    DataType arithmeticResultType(DataType l, DataType r) const;
};

#endif // SEMANTIC_H