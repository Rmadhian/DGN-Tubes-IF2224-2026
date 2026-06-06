#ifndef ICG_VISITOR_H
#define ICG_VISITOR_H

#include <vector>
#include <string>
#include "tac_instruction.h"
#include "ast_builder.h" 

class ICGVisitor : public ASTVisitor {
private:
    // Array utama untuk menyimpan daftar instruksi ICG.
    std::vector<Instruction> instructions;

    // Symbol table dari semantic analysis, untuk mencari alamat variabel.
    SymbolTable st;

    // String pool untuk menyimpan literal string (index dipakai oleh LIT)
    std::vector<std::string> stringPool;

public:
    ICGVisitor() {}

    const std::vector<Instruction>& getInstructions() const {
        return instructions;
    }

    const std::vector<std::string>& getStringPool() const {
        return stringPool;
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

    // Helper pendispatch node AST.

    // Mendispatch sebuah node AST ke method visit yang sesuai.
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

    // Menangani pembuatan kode ICG untuk deklarasi tingkat program dan subprogram.
    void visit(ProgramNode* node) override;
    void visit(VarDeclNode* node) override;
    void visit(ConstDeclNode* node) override;
    void visit(SubprogDeclNode* node) override;

    // Menangani pembuatan kode ICG untuk ekspresi dan perintah dasar (load, store, operator aritmatika, cetak).
    void visit(LiteralNode* node) override;
    void visit(VarAccessNode* node) override;
    void visit(BinaryOpNode* node) override;
    void visit(UnaryOpNode* node) override;
    void visit(FuncCallNode* node) override;
    void visit(AssignStmtNode* node) override;
    void visit(WriteStatementNode* node) override; // Menangani WRT dan WRTLN

    // Menangani pembuatan kode ICG untuk control flow (percabangan dan perulangan).
    void visit(CompoundStmtNode* node) override;
    void visit(IfStmtNode* node) override;
    void visit(WhileStmtNode* node) override;
    void visit(ForStmtNode* node) override;
};

#endif // ICG_VISITOR_H