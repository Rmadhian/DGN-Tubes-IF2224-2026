#include "semantic.h"
#include <iostream>

using namespace std;

// =========================================================
// Accept dispatch untuk node statement
// =========================================================

void CompoundStmtNode::accept(SemanticVisitor* v) { v->visit(this); }
void AssignStmtNode::accept(SemanticVisitor* v)   { v->visit(this); }
void IfStmtNode::accept(SemanticVisitor* v)       { v->visit(this); }
void WhileStmtNode::accept(SemanticVisitor* v)    { v->visit(this); }
void ForStmtNode::accept(SemanticVisitor* v)      { v->visit(this); }

// =========================================================
// Print AST untuk node statement
// =========================================================

void CompoundStmtNode::print(ostream& out, int indent, const SymbolTable* st) const {
    printIndent(out, indent);
    out << "CompoundStatement → block_index:" << symRef
        << ", lev:" << lexicalLevel << "\n";
    for (auto* s : statements)
        if (s) s->print(out, indent + 1, st);
}

void AssignStmtNode::print(ostream& out, int indent, const SymbolTable* st) const {
    printIndent(out, indent);
    out << "Assign → type:" << dtStr(evalType) << "\n";
    if (left) {
        printIndent(out, indent + 1);
        out << "target:\n";
        left->print(out, indent + 2, st);
    }
    if (right) {
        printIndent(out, indent + 1);
        out << "value:\n";
        right->print(out, indent + 2, st);
    }
}

void IfStmtNode::print(ostream& out, int indent, const SymbolTable* st) const {
    printIndent(out, indent);
    out << "IfStatement\n";
    if (condition) {
        printIndent(out, indent + 1);
        out << "condition:\n";
        condition->print(out, indent + 2, st);
    }
    if (thenStmt) {
        printIndent(out, indent + 1);
        out << "then:\n";
        thenStmt->print(out, indent + 2, st);
    }
    if (elseStmt) {
        printIndent(out, indent + 1);
        out << "else:\n";
        elseStmt->print(out, indent + 2, st);
    }
}

void WhileStmtNode::print(ostream& out, int indent, const SymbolTable* st) const {
    printIndent(out, indent);
    out << "WhileStatement\n";
    if (condition) {
        printIndent(out, indent + 1);
        out << "condition:\n";
        condition->print(out, indent + 2, st);
    }
    if (body) {
        printIndent(out, indent + 1);
        out << "body:\n";
        body->print(out, indent + 2, st);
    }
}

void ForStmtNode::print(ostream& out, int indent, const SymbolTable* st) const {
    printIndent(out, indent);
    out << "ForStatement('" << iterVar << "' "
        << (isDownto ? "downto" : "to") << ")\n";
    if (startExpr) {
        printIndent(out, indent + 1);
        out << "from:\n";
        startExpr->print(out, indent + 2, st);
    }
    if (endExpr) {
        printIndent(out, indent + 1);
        out << "to:\n";
        endExpr->print(out, indent + 2, st);
    }
    if (body) {
        printIndent(out, indent + 1);
        out << "body:\n";
        body->print(out, indent + 2, st);
    }
}

// =========================================================
// SemanticAnalyzer: visit(CompoundStmtNode)
// Blok pernyataan — tidak membuat scope baru sendiri
// (scope sudah dikelola oleh ProgramNode/SubprogDeclNode)
// =========================================================

void SemanticAnalyzer::visit(CompoundStmtNode* node) {
    // Buat btab entry untuk compound statement ini (untuk tracking vsze)
    int blockIdx    = st.insertBTab();
    int startTabIdx = (int)st.tab.size() - 1;

    node->symRef      = blockIdx;
    node->lexicalLevel = st.currentLevel;

    // Kunjungi setiap statement
    for (ASTNode* stmt : node->statements)
        if (stmt) stmt->accept(this);

    // Hitung jumlah variabel lokal yang dideklarasikan dalam compound statement ini
    // (tidak berlaku karena deklarasi ada di declaration-part, bukan di dalam begin..end)
    int endTabIdx = (int)st.tab.size() - 1;
    int vsze = 0;
    for (int i = startTabIdx + 1; i <= endTabIdx; i++) {
        if (st.tab[i].obj == ObjClass::VARIABLE && st.tab[i].lev == st.currentLevel)
            vsze++;
    }
    st.updateBTab(blockIdx, endTabIdx, 0, 0, vsze);

    node->evalType = DataType::NONE;
}

// =========================================================
// SemanticAnalyzer: visit(AssignStmtNode)
// Validasi assignment compatibility sesuai spek Type Compatibility
// =========================================================

void SemanticAnalyzer::visit(AssignStmtNode* node) {
    // Kunjungi sisi kiri (target) dan kanan (value)
    if (node->left)  node->left->accept(this);
    if (node->right) node->right->accept(this);

    DataType targetType = node->left  ? node->left->evalType  : DataType::NOTYPE;
    DataType valueType  = node->right ? node->right->evalType : DataType::NOTYPE;

    // Validasi: target harus variabel (bukan konstanta atau tipe)
    if (node->left) {
        VarAccessNode* var = dynamic_cast<VarAccessNode*>(node->left);
        if (var) {
            TabEntry* e = st.lookupTab(var->name);
            if (e && e->obj == ObjClass::CONSTANT) {
                semanticError("Tidak bisa assign ke konstanta '" + var->name + "'.");
            }
        }
    }

    // Validasi type compatibility (assignment-compatible)
    if (!isAssignmentCompatible(targetType, valueType)) {
        semanticError("Type mismatch dalam assignment: tidak bisa assign " +
                      ASTNode::dtStr(valueType) + " ke " +
                      ASTNode::dtStr(targetType) + ".");
    }

    node->evalType    = DataType::NONE;
    node->lexicalLevel = st.currentLevel;
}

// =========================================================
// SemanticAnalyzer: visit(IfStmtNode)
// Kondisi harus bertipe Boolean
// =========================================================

void SemanticAnalyzer::visit(IfStmtNode* node) {
    if (node->condition) {
        node->condition->accept(this);
        if (node->condition->evalType != DataType::BOOLEAN &&
            node->condition->evalType != DataType::NOTYPE) {
            semanticError("Kondisi 'if' harus bertipe Boolean, diberikan " +
                          ASTNode::dtStr(node->condition->evalType) + ".");
        }
    } else {
        semanticError("Statement 'if' tidak memiliki kondisi.");
    }

    if (node->thenStmt) node->thenStmt->accept(this);
    if (node->elseStmt) node->elseStmt->accept(this);

    node->evalType    = DataType::NONE;
    node->lexicalLevel = st.currentLevel;
}

// =========================================================
// SemanticAnalyzer: visit(WhileStmtNode)
// Kondisi harus bertipe Boolean (sesuai spek revisi: while expr do compound)
// =========================================================

void SemanticAnalyzer::visit(WhileStmtNode* node) {
    if (node->condition) {
        node->condition->accept(this);
        if (node->condition->evalType != DataType::BOOLEAN &&
            node->condition->evalType != DataType::NOTYPE) {
            semanticError("Kondisi 'while' harus bertipe Boolean, diberikan " +
                          ASTNode::dtStr(node->condition->evalType) + ".");
        }
    } else {
        semanticError("Statement 'while' tidak memiliki kondisi.");
    }

    if (node->body) node->body->accept(this);

    node->evalType    = DataType::NONE;
    node->lexicalLevel = st.currentLevel;
}

// =========================================================
// SemanticAnalyzer: visit(ForStmtNode)
// Iterator harus ordinal; ekspresi start/end harus kompatibel dengan iterator
// (sesuai spek revisi: for ident := expr to/downto expr do compound)
// =========================================================

void SemanticAnalyzer::visit(ForStmtNode* node) {
    // Lookup variabel iterator
    TabEntry* iter = st.lookupTab(node->iterVar);
    DataType iterType = DataType::NOTYPE;

    if (!iter) {
        semanticError("Variabel iterator '" + node->iterVar +
                      "' pada 'for' belum dideklarasikan.");
    } else {
        iterType = iter->type;
        // Iterator harus bertipe ordinal (Integer, Char, atau Boolean)
        if (!isOrdinalType(iterType)) {
            semanticError("Variabel iterator '" + node->iterVar +
                          "' harus bertipe ordinal (Integer/Char/Boolean), diberikan " +
                          ASTNode::dtStr(iterType) + ".");
        }
    }

    // Kunjungi ekspresi start dan end
    if (node->startExpr) node->startExpr->accept(this);
    if (node->endExpr)   node->endExpr->accept(this);

    // Validasi kompatibilitas tipe start expression dengan iterator
    if (iter && node->startExpr) {
        if (!isAssignmentCompatible(iterType, node->startExpr->evalType)) {
            semanticError("Ekspresi awal 'for' bertipe " +
                          ASTNode::dtStr(node->startExpr->evalType) +
                          " tidak kompatibel dengan iterator bertipe " +
                          ASTNode::dtStr(iterType) + ".");
        }
    }

    // Validasi kompatibilitas tipe end expression dengan iterator
    if (iter && node->endExpr) {
        if (!isAssignmentCompatible(iterType, node->endExpr->evalType)) {
            semanticError("Ekspresi akhir 'for' bertipe " +
                          ASTNode::dtStr(node->endExpr->evalType) +
                          " tidak kompatibel dengan iterator bertipe " +
                          ASTNode::dtStr(iterType) + ".");
        }
    }

    if (node->body) node->body->accept(this);

    node->evalType    = DataType::NONE;
    node->lexicalLevel = st.currentLevel;
}