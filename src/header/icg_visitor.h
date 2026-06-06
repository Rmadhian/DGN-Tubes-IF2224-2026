#ifndef ICG_VISITOR_H
#define ICG_VISITOR_H

#include <vector>
#include <string>
#include "tac_instruction.h"
#include "ast_builder.h" 

class ICGVisitor : public ASTVisitor {
private:
    // ============================================================================
    // (Penyimpanan Utama)
    // Semua method visit() akan menambahkan (push_back) instruksi ke array ini.
    // ============================================================================
    std::vector<Instruction> instructions;

    // Symbol table hasil semantic analysis (Milestone 3). Dipakai untuk
    // mencari alamat memori variabel.
    SymbolTable st;

public:
    ICGVisitor() {}

    // Mengambil hasil akhir instruksi untuk di-pass ke Interpreter
    const std::vector<Instruction>& getInstructions() const {
        return instructions;
    }

    // Inisialisasi Symbol Table dari Semantic Analyzer
    void setSymbolTable(const SymbolTable& symTab) {
        this->st = symTab;
        
        // Sesuaikan alamat variabel (fallback jika kosong)
        int offset = 3;
        for (size_t i = 33; i < st.tab.size(); i++) {
            if (st.tab[i].obj == ObjClass::VARIABLE) {
                if (st.tab[i].adr < 3) {
                    st.tab[i].adr = offset; // Fallback jika adr belum diatur
                }
                offset = st.tab[i].adr + 1;
            }
        }
    }

    // ------------------------------------------------------------------------
    // Helper bersama (Bagian Nelson)
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

    // Mencari TabEntry sebuah variabel berdasarkan namanya.
    TabEntry* getTabEntry(const std::string& name) {
        return st.lookupTab(name);
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