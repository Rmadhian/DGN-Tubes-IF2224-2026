#include "semantic.h"
#include <iostream>
#include <cstdlib>

using namespace std;

// Manajemen tabel registrasi (Symbol Table)

// Inisialisasi Predefined Identifier (Integer, Real, True, False, dll)
void SymbolTable::initPredefined() {
    currentLevel = 0; // Predefined berada di lexical level 0

    // Registrasi Tipe Data Dasar ke tabel utama (tab)
    insertTab("integer", ObjClass::TYPE_DEF, DataType::INTEGER, 0, 0, 0);
    insertTab("real", ObjClass::TYPE_DEF, DataType::REAL, 0, 0, 0);
    insertTab("char", ObjClass::TYPE_DEF, DataType::CHAR, 0, 0, 0);
    insertTab("boolean", ObjClass::TYPE_DEF, DataType::BOOLEAN, 0, 0, 0);
    insertTab("string", ObjClass::TYPE_DEF, DataType::STRING, 0, 0, 0);

    // Registrasi Konstanta Predefined (True / False)
    insertTab("true", ObjClass::CONSTANT, DataType::BOOLEAN, 0, 0, 1);
    insertTab("false", ObjClass::CONSTANT, DataType::BOOLEAN, 0, 0, 0);
}

// Fungsi untuk memasukkan entry baru ke tabel tab utama
int SymbolTable::insertTab(string name, ObjClass obj, DataType type, int ref, int lev, int adr) {
    TabEntry entry;
    entry.identifiers = name;
    entry.obj = obj;
    entry.type = type;
    entry.ref = ref;
    // Jika level tidak diset (-1), gunakan scope/level saat ini
    entry.lev = (lev == -1) ? currentLevel : lev; 
    entry.adr = adr;
    entry.link = -1; // Default no link
    
    tab.push_back(entry);
    return tab.size() - 1; // Mengembalikan index (symRef)
}

// Fungsi untuk mencari identifier HANYA di scope lokal (untuk validasi Multiple Declaration)
TabEntry* SymbolTable::lookupLocalTab(string name) {
    // Cari dari belakang untuk mendapatkan deklarasi terbaru
    for (int i = tab.size() - 1; i >= 0; i--) {
        if (tab[i].lev < currentLevel) {
            break; // Sudah keluar dari scope lokal saat ini
        }
        if (tab[i].lev == currentLevel && tab[i].identifiers == name) {
            return &tab[i]; // Ditemukan duplikat di scope yang sama
        }
    }
    return nullptr;
}

// Fungsi untuk mencari identifier dari scope terdalam ke terluar (visible scope)
TabEntry* SymbolTable::lookupTab(string name) {
    for (int i = (int)tab.size() - 1; i >= 0; i--) {
        if (tab[i].identifiers == name) {
            return &tab[i];
        }
    }
    return nullptr;
}

// Akses entry atab berdasarkan index
ATabEntry* SymbolTable::getATab(int index) {
    if (index < 0 || index >= (int)atab.size()) return nullptr;
    return &atab[index];
}

// Fungsi untuk mendaftarkan detail Array ke tabel atab
int SymbolTable::insertATab(DataType xtyp, DataType etyp, int eref, int low, int high, int elsz) {
    ATabEntry entry;
    entry.arrays = atab.size();
    entry.xtyp = xtyp;
    entry.etyp = etyp;
    entry.eref = eref;
    entry.low = low;
    entry.high = high;
    entry.elsz = elsz;
    entry.size = (high - low + 1) * elsz; // Kalkulasi total ukuran array
    
    atab.push_back(entry);
    return atab.size() - 1; // Mengembalikan index referensi array
}

// Implementasi metode accept() untuk AST deklarasi

void ProgramNode::accept(SemanticVisitor* visitor) {
    visitor->visit(this);
}

void VarDeclNode::accept(SemanticVisitor* visitor) {
    visitor->visit(this);
}

void ConstDeclNode::accept(SemanticVisitor* visitor) {
    visitor->visit(this);
}

void SubprogDeclNode::accept(SemanticVisitor* visitor) {
    visitor->visit(this);
}

// Logika visitor untuk deklarasi (Semantic Analysis)
// Definisi metode SemanticAnalyzer untuk node deklarasi.
// Class SemanticAnalyzer dideklarasikan di semantic.h.

void SemanticAnalyzer::visit(ProgramNode* node) {
    // 1. Inisialisasi predefined types & constants (Integer, Real, True, dll)
    st.initPredefined();

    // 2. Registrasi nama program ke tabel
    node->symRef = st.insertTab(node->name, ObjClass::PROCEDURE, DataType::NONE);
    node->lexicalLevel = st.currentLevel;

    // 3. Masuk ke scope global
    st.pushScope();

    // 4. Kunjungi semua deklarasi global (variabel, konstanta, fungsi, prosedur)
    for (ASTNode* decl : node->declarations) {
        decl->accept(this);
    }

    // 5. Kunjungi block program utama
    if (node->mainBlock) {
        node->mainBlock->accept(this);
    }

    // 6. Keluar scope global
    st.popScope();
}

void SemanticAnalyzer::visit(VarDeclNode* node) {
    for (const string& ident : node->idents) {
        // Validasi Multiple Declaration di scope yang sama
        if (st.lookupLocalTab(ident) != nullptr) {
            cerr << "Semantic Error: Multiple declaration of variable '"
                 << ident << "' in the same scope." << endl;
            exit(EXIT_FAILURE);
        }

        // Registrasi variabel ke tabel
        // node->ref akan menunjuk ke atab index jika tipenya adalah ARRAY
        node->symRef = st.insertTab(ident, ObjClass::VARIABLE, node->type, node->ref);
        node->lexicalLevel = st.currentLevel;
    }
}

void SemanticAnalyzer::visit(ConstDeclNode* node) {
    // Validasi Multiple Declaration di scope yang sama
    if (st.lookupLocalTab(node->name) != nullptr) {
        cerr << "Semantic Error: Multiple declaration of constant '"
             << node->name << "' in the same scope." << endl;
        exit(EXIT_FAILURE);
    }

    // Registrasi konstanta ke tabel
    node->symRef = st.insertTab(node->name, ObjClass::CONSTANT, node->type);
    node->lexicalLevel = st.currentLevel;
}

void SemanticAnalyzer::visit(SubprogDeclNode* node) {
    // Validasi Multiple Declaration sebelum masuk ke scope fungsi
    if (st.lookupLocalTab(node->name) != nullptr) {
        cerr << "Semantic Error: Multiple declaration of subprogram '"
             << node->name << "' in the same scope." << endl;
        exit(EXIT_FAILURE);
    }

    // Registrasi signature Prosedur/Fungsi ke tabel di level saat ini
    ObjClass objType = node->isFunction ? ObjClass::FUNCTION : ObjClass::PROCEDURE;
    node->symRef = st.insertTab(node->name, objType, node->retType);
    node->lexicalLevel = st.currentLevel;

    // Buka scope baru untuk parameter dan variabel lokal subprogram
    st.pushScope();

    // Kunjungi dan registrasikan semua parameter
    for (ASTNode* param : node->params) {
        param->accept(this);
    }

    // Kunjungi blok isi subprogram
    if (node->block) {
        node->block->accept(this);
    }

    // Keluar dari scope lokal subprogram
    st.popScope();
}