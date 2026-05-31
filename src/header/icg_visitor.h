#ifndef ICG_VISITOR_H
#define ICG_VISITOR_H

#include <vector>
#include "tac_instruction.h"
#include "ast_builder.h" // Asumsi file yang menyimpan deklarasi interface ASTVisitor dan Node-Node AST kalian

class ICGVisitor : public ASTVisitor {
private:
    // ============================================================================
    // (Penyimpanan Utama)
    // Semua method visit() akan menambahkan (push_back) instruksi ke array ini.
    // ============================================================================
    std::vector<Instruction> instructions;

public:
    // Mengambil hasil akhir instruksi untuk di-pass ke Interpreter
    const std::vector<Instruction>& getInstructions() const {
        return instructions;
    }

    // Set symbol table dari hasil semantic analysis
    void setSymbolTable(const SymbolTable& symTab) {
        this->st = symTab;
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