#include "semantic.h"
#include <iostream>
#include <cstdlib>

using namespace std;

// --- AST accept dispatch ---

void CompoundStmtNode::accept(SemanticVisitor* visitor) { visitor->visit(this); }
void AssignStmtNode::accept(SemanticVisitor* visitor)   { visitor->visit(this); }
void IfStmtNode::accept(SemanticVisitor* visitor)       { visitor->visit(this); }
void WhileStmtNode::accept(SemanticVisitor* visitor)    { visitor->visit(this); }
void ForStmtNode::accept(SemanticVisitor* visitor)      { visitor->visit(this); }

// --- Symbol Table: btab management ---

int SymbolTable::insertBTab() {
    BTabEntry entry;
    entry.blocks = btab.size();
    entry.last = 0;
    entry.lpar = 0;
    entry.psze = 0;
    entry.vsze = 0;
    btab.push_back(entry);
    return (int)btab.size() - 1;
}

void SymbolTable::updateBTab(int bIndex, int last, int lpar, int psze, int vsze) {
    if (bIndex < 0 || bIndex >= (int)btab.size()) return;
    btab[bIndex].last = last;
    btab[bIndex].lpar = lpar;
    btab[bIndex].psze = psze;
    btab[bIndex].vsze = vsze;
}

BTabEntry* SymbolTable::getBTab(int index) {
    if (index < 0 || index >= (int)btab.size()) return nullptr;
    return &btab[index];
}

// --- Type helpers (local, not exposed in header) ---

static const char* dataTypeName(DataType t) {
    switch (t) {
        case DataType::INTEGER: return "Integer";
        case DataType::REAL:    return "Real";
        case DataType::CHAR:    return "Char";
        case DataType::BOOLEAN: return "Boolean";
        case DataType::STRING:  return "String";
        case DataType::ARRAY:   return "Array";
        case DataType::RECORD:  return "Record";
        case DataType::NONE:    return "None";
        case DataType::NOTYPE:  return "Unknown";
        default:                return "?";
    }
}

// Integer, Char, Boolean can be used as for-loop iterators
static bool isOrdinalType(DataType t) {
    return t == DataType::INTEGER || t == DataType::CHAR || t == DataType::BOOLEAN;
}

// Real accepts Integer (widening); NOTYPE is permissive to avoid cascading errors
static bool isAssignmentCompatible(DataType target, DataType value) {
    if (target == DataType::NOTYPE || value == DataType::NOTYPE) return true;
    if (target == value) return true;
    if (target == DataType::REAL && value == DataType::INTEGER) return true;
    return false;
}

// --- Statement visitors ---

void SemanticAnalyzer::visit(CompoundStmtNode* node) {
    st.pushScope();
    int blockIdx    = st.insertBTab();
    int startTabIdx = (int)st.tab.size() - 1;

    node->symRef      = blockIdx;
    node->lexicalLevel = st.currentLevel;

    for (ASTNode* stmt : node->statements)
        if (stmt) stmt->accept(this);

    // count local variables declared inside this block
    int endTabIdx = (int)st.tab.size() - 1;
    int vsze = 0;
    for (int i = startTabIdx + 1; i <= endTabIdx; i++) {
        if (st.tab[i].obj == ObjClass::VARIABLE &&
            st.tab[i].lev == st.currentLevel &&
            st.tab[i].nrm == 1)
            vsze++;
    }
    st.updateBTab(blockIdx, endTabIdx, 0, 0, vsze);

    node->evalType = DataType::NONE;
    st.popScope();
}

void SemanticAnalyzer::visit(AssignStmtNode* node) {
    if (node->left)  node->left->accept(this);
    if (node->right) node->right->accept(this);

    DataType targetType = node->left  ? node->left->evalType  : DataType::NOTYPE;
    DataType valueType  = node->right ? node->right->evalType : DataType::NOTYPE;

    if (!isAssignmentCompatible(targetType, valueType))
        cerr << "Semantic Error: Type mismatch in assignment. Cannot assign "
             << dataTypeName(valueType) << " value to "
             << dataTypeName(targetType) << " target." << endl;

    node->evalType     = DataType::NONE;
    node->lexicalLevel = st.currentLevel;
}

void SemanticAnalyzer::visit(IfStmtNode* node) {
    if (node->condition) {
        node->condition->accept(this);
        if (node->condition->evalType != DataType::BOOLEAN &&
            node->condition->evalType != DataType::NOTYPE)
            cerr << "Semantic Error: 'if' condition must be Boolean (got "
                 << dataTypeName(node->condition->evalType) << ")." << endl;
    }

    if (node->thenStmt) node->thenStmt->accept(this);
    if (node->elseStmt) node->elseStmt->accept(this);

    node->evalType     = DataType::NONE;
    node->lexicalLevel = st.currentLevel;
}

void SemanticAnalyzer::visit(WhileStmtNode* node) {
    if (node->condition) {
        node->condition->accept(this);
        if (node->condition->evalType != DataType::BOOLEAN &&
            node->condition->evalType != DataType::NOTYPE)
            cerr << "Semantic Error: 'while' condition must be Boolean (got "
                 << dataTypeName(node->condition->evalType) << ")." << endl;
    }

    if (node->body) node->body->accept(this);

    node->evalType     = DataType::NONE;
    node->lexicalLevel = st.currentLevel;
}

void SemanticAnalyzer::visit(ForStmtNode* node) {
    TabEntry* iter   = st.lookupTab(node->iterVar);
    DataType iterType = DataType::NOTYPE;

    if (!iter) {
        cerr << "Semantic Error: For loop iterator '" << node->iterVar
             << "' is not declared." << endl;
    } else {
        iterType = iter->type;
        if (!isOrdinalType(iterType))
            cerr << "Semantic Error: For loop iterator '" << node->iterVar
                 << "' must be of ordinal type (Integer/Char/Boolean), got "
                 << dataTypeName(iterType) << "." << endl;
    }

    if (node->startExpr) node->startExpr->accept(this);
    if (node->endExpr)   node->endExpr->accept(this);

    if (iter) {
        if (node->startExpr && !isAssignmentCompatible(iterType, node->startExpr->evalType))
            cerr << "Semantic Error: 'for' start expression type ("
                 << dataTypeName(node->startExpr->evalType)
                 << ") incompatible with iterator type ("
                 << dataTypeName(iterType) << ")." << endl;

        if (node->endExpr && !isAssignmentCompatible(iterType, node->endExpr->evalType))
            cerr << "Semantic Error: 'for' end expression type ("
                 << dataTypeName(node->endExpr->evalType)
                 << ") incompatible with iterator type ("
                 << dataTypeName(iterType) << ")." << endl;
    }

    if (node->body) node->body->accept(this);

    node->evalType     = DataType::NONE;
    node->lexicalLevel = st.currentLevel;
}