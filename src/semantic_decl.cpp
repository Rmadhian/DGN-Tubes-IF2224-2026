#include "semantic.h"
#include <iostream>
#include <cstdlib>

using namespace std;

// Inisialisasi identifier bawaan (tipe dasar dan konstanta boolean)

void SymbolTable::initPredefined() {
    currentLevel = 0;

    // Tipe data dasar
    insertTab("integer", ObjClass::TYPE_DEF, DataType::INTEGER, 0, 0, 0);
    insertTab("real", ObjClass::TYPE_DEF, DataType::REAL, 0, 0, 0);
    insertTab("char", ObjClass::TYPE_DEF, DataType::CHAR, 0, 0, 0);
    insertTab("boolean", ObjClass::TYPE_DEF, DataType::BOOLEAN, 0, 0, 0);
    insertTab("string", ObjClass::TYPE_DEF, DataType::STRING, 0, 0, 0);

    // Konstanta boolean
    insertTab("true", ObjClass::CONSTANT, DataType::BOOLEAN, 0, 0, 1);
    insertTab("false", ObjClass::CONSTANT, DataType::BOOLEAN, 0, 0, 0);
}

// Insert entry baru ke tab, return indeks yang baru ditambahkan

int SymbolTable::insertTab(string name, ObjClass obj, DataType type, int ref, int lev, int adr) {
    TabEntry entry;
    entry.identifiers = name;
    entry.obj = obj;
    entry.type = type;
    entry.ref = ref;
    entry.lev = (lev == -1) ? currentLevel : lev;
    entry.adr = adr;
    entry.link = -1;
    
    tab.push_back(entry);
    return tab.size() - 1;
}

// Lookup di scope lokal saja (untuk deteksi multiple declaration)

TabEntry* SymbolTable::lookupLocalTab(string name) {
    for (int i = tab.size() - 1; i >= 0; i--) {
        if (tab[i].lev < currentLevel)
            break; // Sudah melewati batas scope lokal
        if (tab[i].lev == currentLevel && tab[i].identifiers == name)
            return &tab[i];
    }
    return nullptr;
}

TabEntry* SymbolTable::getTab(int index) {
    if (index >= 0 && index < tab.size()) {
        return &tab[index];
    }
    return nullptr;
}

// Lookup dari scope terdalam ke terluar (resolusi nama standar)

TabEntry* SymbolTable::lookupTab(string name) {
    for (int i = (int)tab.size() - 1; i >= 0; i--) {
        if (tab[i].identifiers == name)
            return &tab[i];
    }
    return nullptr;
}

ATabEntry* SymbolTable::getATab(int index) {
    if (index < 0 || index >= (int)atab.size()) return nullptr;
    return &atab[index];
}

// Insert metadata array ke atab, return indeks referensi

int SymbolTable::insertATab(DataType xtyp, DataType etyp, int eref, int low, int high, int elsz) {
    ATabEntry entry;
    entry.arrays = atab.size();
    entry.xtyp = xtyp;
    entry.etyp = etyp;
    entry.eref = eref;
    entry.low = low;
    entry.high = high;
    entry.elsz = elsz;
    entry.size = (high - low + 1) * elsz;
    
    atab.push_back(entry);
    return atab.size() - 1;
}

// Dispatch accept() untuk node deklarasi

void ProgramNode::accept(SemanticVisitor* visitor)    { visitor->visit(this); }
void VarDeclNode::accept(SemanticVisitor* visitor)    { visitor->visit(this); }
void ConstDeclNode::accept(SemanticVisitor* visitor)  { visitor->visit(this); }
void SubprogDeclNode::accept(SemanticVisitor* visitor) { visitor->visit(this); }

// Visitor: ProgramNode — entry point analisis semantik

void SemanticAnalyzer::visit(ProgramNode* node) {
    st.initPredefined();

    node->symRef = st.insertTab(node->name, ObjClass::PROCEDURE, DataType::NONE);
    node->lexicalLevel = st.currentLevel;

    st.pushScope();

    for (ASTNode* decl : node->declarations)
        decl->accept(this);

    if (node->mainBlock)
        node->mainBlock->accept(this);

    st.popScope();
}

// Visitor: VarDeclNode — registrasi variabel ke symbol table

void SemanticAnalyzer::visit(VarDeclNode* node) {
    for (const string& ident : node->idents) {
        if (st.lookupLocalTab(ident) != nullptr) {
            cerr << "Semantic Error: Multiple declaration of variable '"
                 << ident << "' in the same scope." << endl;
            exit(EXIT_FAILURE);
        }

        node->symRef = st.insertTab(ident, ObjClass::VARIABLE, node->type, node->ref);
        node->lexicalLevel = st.currentLevel;
    }
}

// Visitor: ConstDeclNode — registrasi konstanta ke symbol table

void SemanticAnalyzer::visit(ConstDeclNode* node) {
    if (st.lookupLocalTab(node->name) != nullptr) {
        cerr << "Semantic Error: Multiple declaration of constant '"
             << node->name << "' in the same scope." << endl;
        exit(EXIT_FAILURE);
    }

    node->symRef = st.insertTab(node->name, ObjClass::CONSTANT, node->type);
    node->lexicalLevel = st.currentLevel;
}

// Visitor: SubprogDeclNode — registrasi dan analisis prosedur/fungsi

void SemanticAnalyzer::visit(SubprogDeclNode* node) {
    if (st.lookupLocalTab(node->name) != nullptr) {
        cerr << "Semantic Error: Multiple declaration of subprogram '"
             << node->name << "' in the same scope." << endl;
        exit(EXIT_FAILURE);
    }

    ObjClass objType = node->isFunction ? ObjClass::FUNCTION : ObjClass::PROCEDURE;
    node->symRef = st.insertTab(node->name, objType, node->retType);
    node->lexicalLevel = st.currentLevel;

    st.pushScope();

    for (ASTNode* param : node->params)
        param->accept(this);

    if (node->block)
        node->block->accept(this);

    st.popScope();
}