#ifndef ICG_VISITOR_H
#define ICG_VISITOR_H

#include <vector>
#include <string>
#include "tac_instruction.h"
#include "ast_builder.h" 

class WriteStatementNode : public ASTNode {
public:
    ASTNode* expression = nullptr; 
    bool newline = true;          
    void accept(SemanticVisitor* visitor) override {}
};

class ASTVisitor {
public:
    virtual ~ASTVisitor() {}

    virtual void visit(ProgramNode* node) = 0;
    virtual void visit(VarDeclNode* node) = 0;
    virtual void visit(ConstDeclNode* node) = 0;
    virtual void visit(SubprogDeclNode* node) = 0;

    virtual void visit(CompoundStmtNode* node) = 0;
    virtual void visit(AssignStmtNode* node) = 0;
    virtual void visit(IfStmtNode* node) = 0;
    virtual void visit(WhileStmtNode* node) = 0;
    virtual void visit(ForStmtNode* node) = 0;

    virtual void visit(BinaryOpNode* node) = 0;
    virtual void visit(UnaryOpNode* node) = 0;
    virtual void visit(LiteralNode* node) = 0;
    virtual void visit(VarAccessNode* node) = 0;
    virtual void visit(FuncCallNode* node) = 0;
    virtual void visit(WriteStatementNode* node) = 0;
};

class ICGVisitor : public ASTVisitor {
private:
    // ============================================================================
    // (Penyimpanan Utama)
    // Semua method visit() akan menambahkan (push_back) instruksi ke array ini.
    // ============================================================================
    std::vector<Instruction> instructions;

    // Symbol table hasil semantic analysis (Milestone 3). Dipakai untuk
    // mencari alamat memori variabel. Boleh null kalau belum di-pass.
    SymbolTable* symtab;

public:
    // Default tetap ada supaya kompatibel; idealnya di-pass symbol table
    // dari hasil SemanticAnalyzer agar alamat variabel bisa di-resolve.
    ICGVisitor(SymbolTable* st = nullptr) : symtab(st) {}

    // Mengambil hasil akhir instruksi untuk di-pass ke Interpreter
    const std::vector<Instruction>& getInstructions() const {
        return instructions;
    }

    // ------------------------------------------------------------------------
    // Helper bersama
    // ------------------------------------------------------------------------

    // Dispatch sebuah node AST ke method visit() yang sesuai. Dipakai untuk
    // menelusuri anak-anak node (pengganti accept() versi ICG).
    void dispatch(ASTNode* node) {
        if (node == nullptr) return;

        if (auto p = dynamic_cast<ProgramNode*>(node))        { visit(p); return; }
        if (auto p = dynamic_cast<VarDeclNode*>(node))        { visit(p); return; }
        if (auto p = dynamic_cast<ConstDeclNode*>(node))      { visit(p); return; }
        if (auto p = dynamic_cast<SubprogDeclNode*>(node))    { visit(p); return; }
        if (auto p = dynamic_cast<CompoundStmtNode*>(node))   { visit(p); return; }
        if (auto p = dynamic_cast<AssignStmtNode*>(node))     { visit(p); return; }
        if (auto p = dynamic_cast<IfStmtNode*>(node))         { visit(p); return; }
        if (auto p = dynamic_cast<WhileStmtNode*>(node))      { visit(p); return; }
        if (auto p = dynamic_cast<ForStmtNode*>(node))        { visit(p); return; }
        if (auto p = dynamic_cast<BinaryOpNode*>(node))       { visit(p); return; }
        if (auto p = dynamic_cast<UnaryOpNode*>(node))        { visit(p); return; }
        if (auto p = dynamic_cast<LiteralNode*>(node))        { visit(p); return; }
        if (auto p = dynamic_cast<VarAccessNode*>(node))      { visit(p); return; }
        if (auto p = dynamic_cast<WriteStatementNode*>(node)) { visit(p); return; }
        if (auto p = dynamic_cast<FuncCallNode*>(node))       { visit(p); return; }
    }

    // Mencari alamat memori sebuah variabel berdasarkan namanya.
    // Slot 0,1,2 dipakai Static Link, Dynamic Link, Return Address (lihat spek),
    // jadi variabel mulai dari alamat 3.
    int addressOf(const std::string& name) {
        if (symtab == nullptr) return -1;
        TabEntry* e = symtab->lookupTab(name);
        if (e == nullptr) return -1;

        // Kalau icg_decl (Dongun) sudah mengisi alamatnya, langsung pakai
        if (e->adr >= 3) return e->adr;

        // Fallback: hitung offset dari urutan deklarasi variabel (mulai dari 3)
        int offset = 3;
        for (size_t i = 33; i < symtab->tab.size(); i++) {
            if (symtab->tab[i].obj == ObjClass::VARIABLE && symtab->tab[i].lev == e->lev) {
                if (symtab->tab[i].identifiers == name) return offset;
                offset++;
            }
        }
        return e->adr;
    }

    // ============================================================================
    // DONGUN (Top-Level & Deklarasi)
    // Fokus di: icg_decl.cpp
    // Tugas: Bikin inisialisasi memori awal (INT m), menyiapkan alokasi fungsi (CAL, RET).
    // ============================================================================
    void visit(ProgramNode* node) override;
    void visit(VarDeclNode* node) override;
    void visit(ConstDeclNode* node) override;
    void visit(SubprogDeclNode* node) override;

    // ============================================================================
    // RAMA (Ekspresi, Assignment & Output)
    // Fokus di: icg_expr.cpp
    // Tugas: Load literal (LIT), load variabel (LOD), store nilai (STO), operasi 
    // matematika/logika (OPR 1-12), dan cetak layar (OPR 13 & 14).
    // ============================================================================
    void visit(LiteralNode* node) override;
    void visit(VarAccessNode* node) override;
    void visit(BinaryOpNode* node) override;
    void visit(UnaryOpNode* node) override;
    void visit(FuncCallNode* node) override;
    void visit(AssignStmtNode* node) override;
    void visit(WriteStatementNode* node) override; // Menangani WRT dan WRTLN

    // ============================================================================
    // NELSON (Statement & Control Flow)
    // Fokus di: icg_stmt.cpp
    // Tugas: Menangani lompatan (JMP, JPC), mengatur blok eksekusi, meratakan (flattening) 
    // percabangan dan perulangan.
    // ============================================================================
    void visit(CompoundStmtNode* node) override;
    void visit(IfStmtNode* node) override;
    void visit(WhileStmtNode* node) override;
    void visit(ForStmtNode* node) override;
};

#endif // ICG_VISITOR_H