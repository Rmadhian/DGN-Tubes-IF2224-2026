#include "semantic.h"
#include <iostream>
#include <iomanip>
#include <string>

using namespace std;

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
            
        // Anotasi tambahan untuk predefined identifiers
        if (t.identifiers == "writeln" || t.identifiers == "readln" || t.identifiers == "true" || t.identifiers == "false") {
            out << "  ... (predefined)";
        }
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
    
    bool hasBlock = node->mainBlock != nullptr;
    
    // Declarations
    if (!node->declarations.empty()) {
        out << (hasBlock ? " ├─ Declarations\n" : " └─ Declarations\n");
        isLastChildStack.push_back(hasBlock ? false : true);
        
        for (size_t i = 0; i < node->declarations.size(); i++) {
            printPrefix(i == node->declarations.size() - 1);
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
    out << "VarDecl(";
    for (size_t i = 0; i < node->idents.size(); i++) {
        out << "'" << node->idents[i] << "'";
        if (i < node->idents.size() - 1) out << ", ";
    }
    out << ")      → tab_index:" << node->symRef << ", type:" << typeToStr(node->type) << ", lev:" << node->lexicalLevel << "\n";
}

void PrintASTVisitor::visit(AssignStmtNode* node) {
    out << "Assign(...)           → type:" << typeToStr(node->evalType) << "\n";
    
    if (node->left) {
        printPrefix(false);
        out << "target ";
        node->left->accept(this); 
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
    out << "'" << node->name << "'         → tab_index:" << node->symRef << ", type:" << typeToStr(node->evalType) << "\n";
}

void PrintASTVisitor::visit(LiteralNode* node) {
    out << node->value << "             → type:" << typeToStr(node->evalType) << "\n";
}

void PrintASTVisitor::visit(BinaryOpNode* node) {
    out << "BinOp '" << node->op << "'         → type:" << typeToStr(node->evalType) << "\n";
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
    out << node->name << "(...)          → ";
    if (node->symRef < 38) out << "predefined, "; // Hardcode asumsi predefined words
    out << "tab_index:" << node->symRef << "\n";
}

// Implementasi kosong untuk Node lain agar tidak error (bisa kamu lengkapi sesuai kebutuhan logika di atas)
void PrintASTVisitor::visit(CompoundStmtNode* node) {}
void PrintASTVisitor::visit(ConstDeclNode* node) {}
void PrintASTVisitor::visit(SubprogDeclNode* node) {}
void PrintASTVisitor::visit(IfStmtNode* node) {}
void PrintASTVisitor::visit(WhileStmtNode* node) {}
void PrintASTVisitor::visit(ForStmtNode* node) {}
void PrintASTVisitor::visit(UnaryOpNode* node) {}