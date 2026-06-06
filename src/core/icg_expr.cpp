#include "icg_visitor.h"
#include <iostream>

// ICG ekspresi, assignment, dan I/O
// LiteralNode: Load literal ke stack
void ICGVisitor::visit(LiteralNode* node) {
    int val = 0;
    if (node->literalType == DataType::INTEGER) {
        val = std::stoi(node->value);
    } else if (node->literalType == DataType::BOOLEAN) {
        // true = 1, false = 0
        val = (node->value == "true" || node->value == "1") ? 1 : 0;
    } else if (node->literalType == DataType::CHAR) {
        // ambil ASCII code dari karakter pertama
        val = (node->value.length() > 0) ? (int)node->value[0] : 0;
    } else if (node->literalType == DataType::STRING) {
        // simpan string di pool, push index-nya ke stack
        std::string s = node->value;
        if (s.length() >= 2 && s.front() == '\'' && s.back() == '\'') {
            s = s.substr(1, s.length() - 2);
        }
        stringPool.push_back(s);
        val = stringPool.size() - 1;
    } else {
        // fallback: coba convert ke int
        try { val = std::stoi(node->value); } catch (...) { val = 0; }
    }
    instructions.push_back(Instruction(OpCode::LIT, 0, val));
}

// VarAccessNode: Load variabel dari memori
void ICGVisitor::visit(VarAccessNode* node) {
    int tabIdx = node->symRef;
    if (tabIdx >= 0 && tabIdx < (int)st.tab.size()) {
        const auto& entry = st.tab[tabIdx];
        if (entry.obj == ObjClass::CONSTANT) {
            instructions.push_back(Instruction(OpCode::LIT, 0, entry.adr));
        } else {
            int levelDiff = node->lexicalLevel - entry.lev;
            instructions.push_back(Instruction(OpCode::LOD, levelDiff, entry.adr));
        }
    } else {
        std::cerr << "ICG Error: symbol reference invalid untuk '" << node->name << "'" << std::endl;
    }
}

// BinaryOpNode: Evaluasi operand kiri, kanan, lalu operasi
void ICGVisitor::visit(BinaryOpNode* node) {
    // Telusuri subtree kiri dulu, baru kanan (postorder)
    node->left->accept(this);
    node->right->accept(this);

    // Mapping operator string ke OPR code
    std::string op = node->op;
    int oprCode = 0;

    if (op == "plus" || op == "+") {oprCode = (int)OprCode::ADD;} // 2
    else if (op == "minus" || op == "-")    oprCode = (int)OprCode::SUB;  // 3
    else if (op == "times" || op == "*")    oprCode = (int)OprCode::MUL;  // 4
    else if (op == "idiv" || op == "div" || op == "/")  oprCode = (int)OprCode::DIV;  // 5
    else if (op == "imod" || op == "mod")   oprCode = (int)OprCode::MOD;  // 6
    else if (op == "eql" || op == "=" || op == "==")    oprCode = (int)OprCode::EQL;  // 7
    else if (op == "neq" || op == "<>")     oprCode = (int)OprCode::NEQ;  // 8
    else if (op == "lss" || op == "<")      oprCode = (int)OprCode::LSS;  // 9
    else if (op == "geq" || op == ">=")     oprCode = (int)OprCode::GEQ;  // 10
    else if (op == "gtr" || op == ">")      oprCode = (int)OprCode::GTR;  // 11
    else if (op == "leq" || op == "<=")     oprCode = (int)OprCode::LEQ;  // 12
    else if (op == "and" || op == "andsy")  oprCode = (int)OprCode::MUL;  // AND = perkalian (1*1=1, sisanya 0)
    else if (op == "or" || op == "orsy") {
        // OR = a+b, lalu jadikan 1 kalau > 0.
        // Tapi kita bisa ubah implementasinya jadi instruksi manual, 
        // karena Arion OPR gak punya native OR, kita pakai ADD lalu cek GTR 0
        instructions.push_back(Instruction(OpCode::OPR, 0, (int)OprCode::ADD));
        instructions.push_back(Instruction(OpCode::LIT, 0, 0));
        instructions.push_back(Instruction(OpCode::OPR, 0, (int)OprCode::GTR));
        return;
    }
    else {
        std::cerr << "ICG Error: operator '" << op << "' belum di-handle" << std::endl;
        return;
    }

    instructions.push_back(Instruction(OpCode::OPR, 0, oprCode));
}

// UnaryOpNode: Operasi tunggal (NEG, NOT)
void ICGVisitor::visit(UnaryOpNode* node) {
    node->operand->accept(this);

    if (node->op == "minus" || node->op == "-") {
        instructions.push_back(Instruction(OpCode::OPR, 0, (int)OprCode::NEG));
    }
    else if (node->op == "not") {
        // NOT: a == 0 (pakai EQL terhadap 0)
        instructions.push_back(Instruction(OpCode::LIT, 0, 0));
        instructions.push_back(Instruction(OpCode::OPR, 0, (int)OprCode::EQL));
    }
    // kalau plus, gak perlu instruksi tambahan (unary + itu no-op)
}

// FuncCallNode: Menangani I/O atau pemanggilan fungsi user
void ICGVisitor::visit(FuncCallNode* node) {
    if (node->name == "writeln" || node->name == "write") {
        if (node->args.empty() && node->name == "writeln") {
            stringPool.push_back("");
            instructions.push_back(Instruction(OpCode::LIT, 0, stringPool.size() - 1));
            instructions.push_back(Instruction(OpCode::OPR, 1, (int)OprCode::WRTLN));
            return;
        }

        // Evaluasi semua argumen lalu cetak
        for (size_t i = 0; i < node->args.size(); i++) {
            auto* arg = node->args[i];
            arg->accept(this);
            // Deteksi tipe: l=1 untuk string, l=0 untuk lainnya
            int lFlag = 0;
            if (auto lit = dynamic_cast<LiteralNode*>(arg)) {
                if (lit->literalType == DataType::STRING) lFlag = 1;
            }
            if (i == node->args.size() - 1 && node->name == "writeln") {
                instructions.push_back(Instruction(OpCode::OPR, lFlag, (int)OprCode::WRTLN));
            } else {
                instructions.push_back(Instruction(OpCode::OPR, lFlag, (int)OprCode::WRT));
            }
        }
    } else if (node->name == "readln" || node->name == "read") {
        // ICG baca input, lalu STO ke variabel
        for (auto* arg : node->args) {
            if (node->name == "readln") {
                instructions.push_back(Instruction(OpCode::OPR, 0, (int)OprCode::READLN));
            } else {
                instructions.push_back(Instruction(OpCode::OPR, 0, (int)OprCode::READ));
            }
            // STO nilai yang baru di-push oleh READ ke address argumen
            VarAccessNode* varArg = dynamic_cast<VarAccessNode*>(arg);
            if (varArg) {
                int tabIdx = varArg->symRef;
                if (tabIdx >= 0 && tabIdx < (int)st.tab.size()) {
                    int levelDiff = varArg->lexicalLevel - st.tab[tabIdx].lev;
                    instructions.push_back(Instruction(OpCode::STO, levelDiff, st.tab[tabIdx].adr));
                }
            }
        }
    } else {
        // Fungsi/prosedur user-defined: push argumen, lalu CAL
        for (auto* arg : node->args) {
            arg->accept(this);
        }
        // Cari alamat fungsi di symbol table
        for (int i = 33; i < (int)st.tab.size(); i++) {
            if (st.tab[i].identifiers == node->name &&
                (st.tab[i].obj == ObjClass::FUNCTION || st.tab[i].obj == ObjClass::PROCEDURE)) {
                int levelDiff = node->lexicalLevel - st.tab[i].lev;
                instructions.push_back(Instruction(OpCode::CAL, levelDiff, st.tab[i].adr, node->args.size()));
                return;
            }
        }
        std::cerr << "ICG Error: fungsi/prosedur '" << node->name << "' gak ketemu" << std::endl;
    }
}

// AssignStmtNode: Evaluasi ruas kanan lalu simpan ke variabel target
void ICGVisitor::visit(AssignStmtNode* node) {
    // Evaluasi ekspresi di ruas kanan, hasilnya bakal di-push ke stack
    node->right->accept(this);

    // Ambil address variabel target (ruas kiri)
    VarAccessNode* target = dynamic_cast<VarAccessNode*>(node->left);
    if (target) {
        int tabIdx = target->symRef;
        if (tabIdx >= 0 && tabIdx < (int)st.tab.size()) {
            int addr = st.tab[tabIdx].adr;
            int levelDiff = target->lexicalLevel - st.tab[tabIdx].lev;
            instructions.push_back(Instruction(OpCode::STO, levelDiff, addr));
        }
    } else {
        std::cerr << "ICG Error: target assignment bukan VarAccessNode" << std::endl;
    }
}

// WriteStatementNode: Cetak nilai argumen
void ICGVisitor::visit(WriteStatementNode* node) {
    if (node->args.empty() && node->hasNewline) {
        // empty writeln
        stringPool.push_back("");
        instructions.push_back(Instruction(OpCode::LIT, 0, stringPool.size() - 1));
        instructions.push_back(Instruction(OpCode::OPR, 1, (int)OprCode::WRTLN));
        return;
    }

    for (size_t i = 0; i < node->args.size(); i++) {
        auto* arg = node->args[i];
        arg->accept(this);
        int lFlag = 0;
        if (auto lit = dynamic_cast<LiteralNode*>(arg)) {
            if (lit->literalType == DataType::STRING) lFlag = 1;
        }
        
        if (i == node->args.size() - 1 && node->hasNewline) {
            instructions.push_back(Instruction(OpCode::OPR, lFlag, (int)OprCode::WRTLN));
        } else {
            instructions.push_back(Instruction(OpCode::OPR, lFlag, (int)OprCode::WRT));
        }
    }
}