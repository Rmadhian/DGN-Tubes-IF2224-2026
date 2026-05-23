#include "semantic.h"
#include <iostream>
#include <iomanip>
#include <string>

using namespace std;

static string opToSym(string op) {
    if (op == "plus" || op == "+") return "+";
    if (op == "minus" || op == "-") return "-";
    if (op == "times" || op == "*") return "*";
    if (op == "idiv" || op == "div" || op == "/") return "div";
    if (op == "mod") return "mod";
    if (op == "eql" || op == "=" || op == "==") return "=";
    if (op == "neq" || op == "<>") return "<>";
    if (op == "lss" || op == "<") return "<";
    if (op == "leq" || op == "<=") return "<=";
    if (op == "gtr" || op == ">") return ">";
    if (op == "geq" || op == ">=") return ">=";
    return op;
}

static string exprToString(ASTNode* node) {
    if (!node) return "";
    if (auto lit = dynamic_cast<LiteralNode*>(node)) {
        return lit->value;
    } else if (auto var = dynamic_cast<VarAccessNode*>(node)) {
        string res = var->name;
        for (auto idx : var->indices) {
            res += "[" + exprToString(idx) + "]";
        }
        return res;
    } else if (auto bin = dynamic_cast<BinaryOpNode*>(node)) {
        return exprToString(bin->left) + " " + opToSym(bin->op) + " " + exprToString(bin->right);
    } else if (auto un = dynamic_cast<UnaryOpNode*>(node)) {
        return opToSym(un->op) + exprToString(un->operand);
    } else if (auto call = dynamic_cast<FuncCallNode*>(node)) {
        return call->name + "(...)";
    }
    return "...";
}

// =========================================================================
// 1. SYMBOL TABLE PRINTER (Sesuai Format Spek)
// =========================================================================

string objToStr(ObjClass obj, string name) {
    if (name == "Hello" || name == "program") return "program"; 
    switch(obj) {
        case ObjClass::CONSTANT: return "const";
        case ObjClass::VARIABLE: return "variable";
        case ObjClass::TYPE_DEF: return "type";
        case ObjClass::PROCEDURE: return "procedure";
        case ObjClass::FUNCTION: return "function";
        default: return "";
    }
}

void printSymbolTables(const SymbolTable& st, std::ostream& out) {
    out << "tab:\n";
    out << "idx  id          obj       type  ref  nrm  lev  adr  link\n";
    out << "-------------------------------------------------------\n";
    
    for (size_t i = 0; i < st.tab.size(); i++) {
        const auto& t = st.tab[i];
        
        out << std::left 
            << setw(5) << i 
            << setw(12) << t.identifiers 
            << setw(10) << objToStr(t.obj, t.identifiers) 
            << setw(6) << (int)t.type 
            << setw(5) << t.ref 
            << setw(5) << t.nrm 
            << setw(5) << t.lev 
            << setw(5) << t.adr 
            << t.link;
            
        out << "\n";
    }

    out << "\nbtab:\n";
    out << "idx  last  lpar  psze  vsze\n";
    out << "---------------------------\n";
    for (size_t i = 0; i < st.btab.size(); i++) {
        const auto& b = st.btab[i];
        out << std::left 
            << setw(5) << i 
            << setw(6) << b.last 
            << setw(6) << b.lpar 
            << setw(6) << b.psze 
            << b.vsze;
            
        if (i == 0) out << "   <- global block (program), vsze=" << b.vsze;
        else if (i == 1) out << "   <- main block (kosong variabel lokal)";
        out << "\n";
    }

    out << "\natab: ";
    if (st.atab.empty()) {
        out << "(kosong karena tidak ada array)\n\n";
    } else {
        out << "\nidx  xtyp  etyp  eref  low  high  elsz  size\n";
        out << "------------------------------------------------\n";
        for (size_t i = 0; i < st.atab.size(); i++) {
             const auto& a = st.atab[i];
             out << std::left << setw(5) << i << setw(6) << (int)a.xtyp << setw(6) << (int)a.etyp 
                 << setw(6) << a.eref << setw(5) << a.low << setw(6) << a.high 
                 << setw(6) << a.elsz << a.size << "\n";
        }
        out << "\n";
    }
}

// =========================================================================
// 2. AST PRINTER (Decorated AST dengan Box-Drawing Characters)
// =========================================================================

void PrintASTVisitor::printPrefix(bool isLast) {
    out << " ";
    for (size_t i = 0; i < isLastChildStack.size(); i++) {
        if (isLastChildStack[i]) out << "    ";
        else out << "│   ";
    }
    if (isLast) out << "└─ ";
    else out << "├─ ";
}

string PrintASTVisitor::typeToStr(DataType t) {
    switch (t) {
        case DataType::INTEGER:   return "integer";
        case DataType::REAL:      return "real";
        case DataType::CHAR:      return "char";
        case DataType::BOOLEAN:   return "boolean";
        case DataType::STRING:    return "string";
        case DataType::NONE:      return "void";
        default:                  return "unknown";
    }
}

void PrintASTVisitor::visit(ProgramNode* node) {
    out << "ProgramNode(name: '" << node->name << "')\n";
    
    bool hasDecl = !node->declarations.empty();
    bool hasBlock = node->mainBlock != nullptr;
    
    // Declarations
    if (hasDecl) {
        bool isLast = !hasBlock; // Kalau nggak ada block, decl ini adalah anak terakhir
        printPrefix(isLast);
        out << "Declarations\n";
        
        isLastChildStack.push_back(isLast);
        for (size_t i = 0; i < node->declarations.size(); i++) {
            // Kita harus kirim informasi ke VarDecl bahwa dia anak dari Declarations
            // Tapi kita handle indentasi di dalam VarDecl sendiri
            node->declarations[i]->accept(this);
        }
        isLastChildStack.pop_back();
    }

    // Main Block
    if (hasBlock) {
        out << " └─ Block                  → block_index:" << node->symRef << ", lev:" << node->lexicalLevel << "\n";
        isLastChildStack.push_back(true);
        
        CompoundStmtNode* comp = dynamic_cast<CompoundStmtNode*>(node->mainBlock);
        if (comp) {
            for (size_t i = 0; i < comp->statements.size(); i++) {
                bool isLast = (i == comp->statements.size() - 1);
                printPrefix(isLast);
                isLastChildStack.push_back(isLast);
                comp->statements[i]->accept(this);
                isLastChildStack.pop_back();
            }
        }
        isLastChildStack.pop_back();
    }
}

void PrintASTVisitor::visit(VarDeclNode* node) {
    for (size_t i = 0; i < node->idents.size(); i++) {
        // 1. Cetak prefix dari stack parent
        for (size_t j = 0; j < isLastChildStack.size(); j++) {
            if (isLastChildStack[j]) out << "    "; 
            else out << " │   ";
        }
        
        // 2. Tentukan simbol ranting: └─ kalau terakhir, ├─ kalau belum
        bool isLastVar = (i == node->idents.size() - 1);
        if (isLastVar) {
            out << " └─ ";
        } else {
            out << " ├─ ";
        }
        
        // 3. Cetak isi node
        out << "VarDecl('" << node->idents[i] << "') \t\t→ tab_index:" 
            << (node->symRef - (int)node->idents.size() + 1 + i) 
            << ", type:" << typeToStr(node->evalType) << ", lev:" << node->lexicalLevel << "\n";
    }
}

void PrintASTVisitor::visit(AssignStmtNode* node) {
    string leftStr = exprToString(node->left);
    string rightStr = exprToString(node->right);
    out << "Assign('" << leftStr << "' := " << rightStr << ") \t→ type:" << typeToStr(node->evalType) << "\n";
    
    if (node->left) {
        printPrefix(false);
        out << "target ";
        isLastChildStack.push_back(false);
        node->left->accept(this); 
        isLastChildStack.pop_back();
    }
    if (node->right) {
        printPrefix(true);
        out << "value ";
        isLastChildStack.push_back(true);
        node->right->accept(this);
        isLastChildStack.pop_back();
    }
}

void PrintASTVisitor::visit(VarAccessNode* node) {
    out << "'" << node->name << "' \t\t→ tab_index:" << node->symRef << ", type:" << typeToStr(node->evalType) << "\n";

    for (size_t i = 0; i < node->indices.size(); i++) {
        bool isLast = (i == node->indices.size() - 1);
        printPrefix(isLast);
        out << "index ";
        isLastChildStack.push_back(isLast);
        node->indices[i]->accept(this);
        isLastChildStack.pop_back();
    }
}

void PrintASTVisitor::visit(LiteralNode* node) {
    out << node->value << " \t\t→ type:" << typeToStr(node->evalType) << "\n";
}

void PrintASTVisitor::visit(BinaryOpNode* node) {
    out << "BinOp '" << node->op << "' \t\t→ type:" << typeToStr(node->evalType) << "\n";
    if (node->left) {
        printPrefix(false);
        isLastChildStack.push_back(false);
        node->left->accept(this);
        isLastChildStack.pop_back();
    }
    if (node->right) {
        printPrefix(true);
        isLastChildStack.push_back(true);
        node->right->accept(this);
        isLastChildStack.pop_back();
    }
}

void PrintASTVisitor::visit(FuncCallNode* node) {
    if (node->name == "writeln" || node->name == "readln" || node->name == "write" || node->name == "read") {
        out << node->name << "(...) \t\t→ predefined, tab_index:" << node->symRef << "\n";
    } else {
        out << "FuncCall('" << node->name << "') \t→ tab_index:" << node->symRef << ", type:" << typeToStr(node->evalType) << "\n";
    }
    
    for (size_t i = 0; i < node->args.size(); i++) {
        bool isLast = (i == node->args.size() - 1);
        printPrefix(isLast);
        out << "arg ";
        isLastChildStack.push_back(isLast);
        node->args[i]->accept(this);
        isLastChildStack.pop_back();
    }
}
void PrintASTVisitor::visit(CompoundStmtNode* node) {
    out << "CompoundStmt\n";
    for (size_t i = 0; i < node->statements.size(); i++) {
        bool isLast = (i == node->statements.size() - 1);
        printPrefix(isLast);
        isLastChildStack.push_back(isLast);
        node->statements[i]->accept(this);
        isLastChildStack.pop_back();
    }
}

void PrintASTVisitor::visit(IfStmtNode* node) {
    out << "IfStmt                → type:" << typeToStr(node->evalType) << "\n";
    
    if (node->condition) {
        printPrefix(false);
        out << "condition ";
        isLastChildStack.push_back(false);
        node->condition->accept(this);
        isLastChildStack.pop_back();
    }
    if (node->thenStmt) {
        bool isLast = (node->elseStmt == nullptr);
        printPrefix(isLast);
        out << "then ";
        isLastChildStack.push_back(isLast);
        node->thenStmt->accept(this);
        isLastChildStack.pop_back();
    }
    if (node->elseStmt) {
        printPrefix(true);
        out << "else ";
        isLastChildStack.push_back(true);
        node->elseStmt->accept(this);
        isLastChildStack.pop_back();
    }
}

void PrintASTVisitor::visit(WhileStmtNode* node) {
    out << "WhileStmt             → type:" << typeToStr(node->evalType) << "\n";
    if (node->condition) {
        printPrefix(false);
        out << "condition ";
        isLastChildStack.push_back(false);
        node->condition->accept(this);
        isLastChildStack.pop_back();
    }
    if (node->body) {
        printPrefix(true);
        out << "body ";
        isLastChildStack.push_back(true);
        node->body->accept(this);
        isLastChildStack.pop_back();
    }
}

void PrintASTVisitor::visit(ForStmtNode* node) {
    out << "ForStmt(iter: '" << node->iterVar << "')  → type:" << typeToStr(node->evalType) << "\n";
    if (node->startExpr) {
        printPrefix(false);
        out << "start ";
        isLastChildStack.push_back(false);
        node->startExpr->accept(this);
        isLastChildStack.pop_back();
    }
    if (node->endExpr) {
        printPrefix(false);
        out << "end ";
        isLastChildStack.push_back(false);
        node->endExpr->accept(this);
        isLastChildStack.pop_back();
    }
    if (node->body) {
        printPrefix(true);
        out << "body ";
        isLastChildStack.push_back(true);
        node->body->accept(this);
        isLastChildStack.pop_back();
    }
}

void PrintASTVisitor::visit(UnaryOpNode* node) {
    out << "UnaryOp '" << node->op << "'          → type:" << typeToStr(node->evalType) << "\n";
    if (node->operand) {
        printPrefix(true);
        isLastChildStack.push_back(true);
        node->operand->accept(this);
        isLastChildStack.pop_back();
    }
}

void PrintASTVisitor::visit(ConstDeclNode* node) {
    out << "ConstDecl('" << node->name << "' = " << node->value << ") → tab_index:" 
        << node->symRef << ", type:" << typeToStr(node->type) << "\n";
}

void PrintASTVisitor::visit(SubprogDeclNode* node) {
    string typStr = node->isFunction ? "Function" : "Procedure";
    out << "SubprogDecl(" << typStr << " '" << node->name << "') → tab_index:" 
        << node->symRef << ", lev:" << node->lexicalLevel;
    if (node->isFunction) {
        out << ", ret_type:" << typeToStr(node->retType);
    }
    out << "\n";

    // Struktur pembantu untuk mengumpulkan semua child node (params + block body)
    struct ChildComponent {
        string label;
        ASTNode* node;
    };
    vector<ChildComponent> children;

    // 1. Kumpulkan semua parameter fungsi/prosedur jika ada
    for (auto param : node->params) {
        if (param) {
            children.push_back({"param ", param});
        }
    }

    // 2. Kumpulkan body block utama milik fungsi/prosedur jika ada
    if (node->block) {
        children.push_back({"body ", node->block});
    }

    // 3. Cetak seluruh komponen secara rekursif dengan tree prefix yang presisi
    for (size_t i = 0; i < children.size(); i++) {
        bool isLast = (i == children.size() - 1);
        
        // Cetak garis branch sesuai kedalaman stack saat ini
        printPrefix(isLast);
        out << children[i].label;
        
        // Push status ke stack sebelum turun ke anak node
        isLastChildStack.push_back(isLast);
        children[i].node->accept(this);
        isLastChildStack.pop_back();
    }
}
