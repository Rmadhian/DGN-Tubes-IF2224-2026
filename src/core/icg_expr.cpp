#include "icg_visitor.h"
#include <iostream>

// ICG untuk Ekspresi, Assignment, dan Output (Bagian Rama)

// LiteralNode -> LIT 0 value (push literal ke stack)
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
    } else {
        // fallback: coba convert ke int
        try { val = std::stoi(node->value); } catch (...) { val = 0; }
    }
    instructions.push_back(Instruction(OpCode::LIT, 0, val));
}

// VarAccessNode -> LOD level address (load value dari memory)
void ICGVisitor::visit(VarAccessNode* node) {
    int tabIdx = node->symRef;
    if (tabIdx >= 0 && tabIdx < (int)st.tab.size()) {
        const auto& entry = st.tab[tabIdx];
        if (entry.obj == ObjClass::CONSTANT) {
            instructions.push_back(Instruction(OpCode::LIT, 0, entry.adr));
        } else {
            instructions.push_back(Instruction(OpCode::LOD, node->lexicalLevel, entry.adr));
        }
    } else {
        std::cerr << "ICG Error: symbol reference invalid untuk '" << node->name << "'" << std::endl;
    }
}

// BinaryOpNode -> visit kiri, visit kanan, lalu OPR sesuai operator
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
    else {
        std::cerr << "ICG Error: operator '" << op << "' belum di-handle" << std::endl;
        return;
    }

    instructions.push_back(Instruction(OpCode::OPR, 0, oprCode));
}

// UnaryOpNode -> visit operand, lalu OPR 1 (NEG) kalau minus
void ICGVisitor::visit(UnaryOpNode* node) {
    node->operand->accept(this);

    if (node->op == "minus" || node->op == "-") {
        instructions.push_back(Instruction(OpCode::OPR, 0, (int)OprCode::NEG));
    }
    // kalau plus, gak perlu instruksi tambahan (unary + itu no-op)
}

// FuncCallNode -> kalau write/writeln, handle output; sisanya CAL
void ICGVisitor::visit(FuncCallNode* node) {
    if (node->name == "writeln") {
        // Tiap argumen: evaluate ekspresi, lalu cetak
        for (size_t i = 0; i < node->args.size(); i++) {
            node->args[i]->accept(this);
            if (i < node->args.size() - 1) {
                instructions.push_back(Instruction(OpCode::OPR, 0, (int)OprCode::WRT));
            } else {
                instructions.push_back(Instruction(OpCode::OPR, 0, (int)OprCode::WRTLN));
            }
        }
        // writeln tanpa argumen = newline doang
        if (node->args.empty()) {
            instructions.push_back(Instruction(OpCode::LIT, 0, 0));
            instructions.push_back(Instruction(OpCode::OPR, 0, (int)OprCode::WRTLN));
        }
    } else if (node->name == "write") {
        for (auto* arg : node->args) {
            arg->accept(this);
            instructions.push_back(Instruction(OpCode::OPR, 0, (int)OprCode::WRT));
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
                instructions.push_back(Instruction(OpCode::CAL, node->lexicalLevel, st.tab[i].adr));
                return;
            }
        }
        std::cerr << "ICG Error: fungsi/prosedur '" << node->name << "' gak ketemu" << std::endl;
    }
}

// AssignStmtNode -> evaluate ruas kanan, lalu STO ke address ruas kiri
void ICGVisitor::visit(AssignStmtNode* node) {
    // Evaluasi ekspresi di ruas kanan, hasilnya bakal di-push ke stack
    node->right->accept(this);

    // Ambil address variabel target (ruas kiri)
    VarAccessNode* target = dynamic_cast<VarAccessNode*>(node->left);
    if (target) {
        int tabIdx = target->symRef;
        if (tabIdx >= 0 && tabIdx < (int)st.tab.size()) {
            int addr = st.tab[tabIdx].adr;
            instructions.push_back(Instruction(OpCode::STO, target->lexicalLevel, addr));
        }
    } else {
        std::cerr << "ICG Error: target assignment bukan VarAccessNode" << std::endl;
    }
}

// WriteStatementNode -> evaluate tiap argumen lalu OPR WRT/WRTLN
void ICGVisitor::visit(WriteStatementNode* node) {
    for (size_t i = 0; i < node->args.size(); i++) {
        node->args[i]->accept(this);
        // Argumen terakhir di writeln pake WRTLN, sisanya WRT
        if (node->hasNewline && i == node->args.size() - 1) {
            instructions.push_back(Instruction(OpCode::OPR, 0, (int)OprCode::WRTLN));
        } else {
            instructions.push_back(Instruction(OpCode::OPR, 0, (int)OprCode::WRT));
        }
    }
    // writeln() tanpa argumen = cetak newline aja
    if (node->args.empty() && node->hasNewline) {
        instructions.push_back(Instruction(OpCode::LIT, 0, 0));
        instructions.push_back(Instruction(OpCode::OPR, 0, (int)OprCode::WRTLN));
    }
}