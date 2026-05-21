#include "semantic.h"
#include <iostream>
#include <iomanip>
#include <sstream>

using namespace std;

// =========================================================
// SymbolTable: Helper konversi enum ke string
// =========================================================

string SymbolTable::dataTypeName(DataType t) const {
    switch (t) {
        case DataType::INTEGER: return "Integer";
        case DataType::REAL:    return "Real";
        case DataType::CHAR:    return "Char";
        case DataType::BOOLEAN: return "Boolean";
        case DataType::STRING:  return "String";
        case DataType::ARRAY:   return "Array";
        case DataType::RECORD:  return "Record";
        case DataType::NONE:    return "None";
        default:                return "Unknown";
    }
}

string SymbolTable::objClassName(ObjClass o) const {
    switch (o) {
        case ObjClass::CONSTANT:  return "Constant";
        case ObjClass::VARIABLE:  return "Variable";
        case ObjClass::TYPE_DEF:  return "TypeDef";
        case ObjClass::PROCEDURE: return "Procedure";
        case ObjClass::FUNCTION:  return "Function";
        default:                  return "?";
    }
}

// =========================================================
// SymbolTable: Inisialisasi predefined identifiers
// Sesuai spek: indeks dimulai dari 33 (karena 0-32 = reserved words)
// Predefined: integer, real, char, boolean, string, true, false
// Ditambah: writeln, readln, write, read (variadic built-ins)
// =========================================================

void SymbolTable::initPredefined() {
    currentLevel = 0;
    tab.clear();
    btab.clear();
    atab.clear();

    // Isi slot 0-32 dengan reserved words (sebagai placeholder)
    // Sesuai spek Lampiran D: 32 reserved words (indeks 1-32)
    // Slot 0 = sentinel/kosong
    vector<string> reservedWords = {
        "",          // 0: sentinel
        "and",       // 1
        "array",     // 2
        "begin",     // 3
        "case",      // 4
        "const",     // 5
        "div",       // 6
        "downto",    // 7
        "do",        // 8
        "else",      // 9
        "end",       // 10
        "for",       // 11
        "function",  // 12
        "if",        // 13
        "mod",       // 14
        "not",       // 15
        "of",        // 16
        "or",        // 17
        "procedure", // 18
        "program",   // 19
        "record",    // 20
        "repeat",    // 21
        "integer",   // 22
        "real",      // 23
        "boolean",   // 24
        "char",      // 25
        "string",    // 26
        "then",      // 27
        "to",        // 28
        "type",      // 29
        "until",     // 30
        "var",       // 31
        "while"      // 32
    };

    // Masukkan slot 0-32 sebagai placeholder reserved words
    for (int i = 0; i < 33; i++) {
        TabEntry e;
        e.identifiers = reservedWords[i];
        e.link = 0;
        e.obj  = ObjClass::TYPE_DEF;
        e.type = DataType::NOTYPE;
        e.ref  = 0;
        e.nrm  = 0;
        e.lev  = 0;
        e.adr  = 0;
        tab.push_back(e);
    }

    // Sekarang tab.size() == 33, predefined identifiers mulai dari indeks 33

    // --- Predefined tipe data (TYPE_DEF) ---
    insertTab("integer", ObjClass::TYPE_DEF, DataType::INTEGER, 0, 0, 0);  // idx 33
    insertTab("real",    ObjClass::TYPE_DEF, DataType::REAL,    0, 0, 0);  // idx 34
    insertTab("char",    ObjClass::TYPE_DEF, DataType::CHAR,    0, 0, 0);  // idx 35
    insertTab("boolean", ObjClass::TYPE_DEF, DataType::BOOLEAN, 0, 0, 0);  // idx 36
    insertTab("string",  ObjClass::TYPE_DEF, DataType::STRING,  0, 0, 0);  // idx 37

    // --- Predefined konstanta boolean ---
    insertTab("true",  ObjClass::CONSTANT, DataType::BOOLEAN, 0, 0, 1);   // idx 38, nilai=1
    insertTab("false", ObjClass::CONSTANT, DataType::BOOLEAN, 0, 0, 0);   // idx 39, nilai=0

    // --- Predefined prosedur/fungsi I/O (variadic, tidak divalidasi parameternya) ---
    // Dimasukkan ke tab agar lookup tidak gagal; ref=0 (tidak ada btab entry)
    insertTab("writeln", ObjClass::PROCEDURE, DataType::NONE, 0, 0, 0);   // idx 40
    insertTab("readln",  ObjClass::PROCEDURE, DataType::NONE, 0, 0, 0);   // idx 41
    insertTab("write",   ObjClass::PROCEDURE, DataType::NONE, 0, 0, 0);   // idx 42
    insertTab("read",    ObjClass::PROCEDURE, DataType::NONE, 0, 0, 0);   // idx 43
}

// =========================================================
// SymbolTable: Operasi pada tab
// =========================================================

int SymbolTable::insertTab(const string& name, ObjClass obj, DataType type,
                           int ref, int lev, int adr, int nrm) {
    TabEntry entry;
    entry.identifiers = name;
    entry.obj  = obj;
    entry.type = type;
    entry.ref  = ref;
    entry.lev  = (lev == -1) ? currentLevel : lev;
    entry.adr  = adr;
    entry.nrm  = nrm;
    entry.link = 0;

    // Bangun linked list per scope: cari identifier sebelumnya di level yang sama
    for (int i = (int)tab.size() - 1; i >= 0; i--) {
        if (tab[i].lev < entry.lev) break;
        if (tab[i].lev == entry.lev) {
            entry.link = i;
            break;
        }
    }

    tab.push_back(entry);
    return (int)tab.size() - 1;
}

// Lookup dari scope terdalam ke terluar (resolusi nama standar)
TabEntry* SymbolTable::lookupTab(const string& name) {
    for (int i = (int)tab.size() - 1; i >= 0; i--) {
        if (tab[i].identifiers == name)
            return &tab[i];
    }
    return nullptr;
}

// Lookup di scope lokal saja (untuk deteksi multiple declaration)
TabEntry* SymbolTable::lookupLocalTab(const string& name) {
    for (int i = (int)tab.size() - 1; i >= 0; i--) {
        if (tab[i].lev < currentLevel) break;
        if (tab[i].lev == currentLevel && tab[i].identifiers == name)
            return &tab[i];
    }
    return nullptr;
}

TabEntry* SymbolTable::getTab(int index) {
    if (index >= 0 && index < (int)tab.size())
        return &tab[index];
    return nullptr;
}

// =========================================================
// SymbolTable: Operasi pada atab
// =========================================================

int SymbolTable::insertATab(DataType xtyp, DataType etyp, int eref,
                             int low, int high, int elsz) {
    ATabEntry entry;
    entry.arrays = (int)atab.size();
    entry.xtyp   = xtyp;
    entry.etyp   = etyp;
    entry.eref   = eref;
    entry.low    = low;
    entry.high   = high;
    entry.elsz   = elsz;
    entry.size   = (high - low + 1) * elsz;
    atab.push_back(entry);
    return (int)atab.size() - 1;
}

ATabEntry* SymbolTable::getATab(int index) {
    if (index >= 0 && index < (int)atab.size())
        return &atab[index];
    return nullptr;
}

// =========================================================
// SymbolTable: Operasi pada btab
// =========================================================

int SymbolTable::insertBTab() {
    BTabEntry entry;
    entry.blocks = (int)btab.size();
    entry.last   = 0;
    entry.lpar   = 0;
    entry.psze   = 0;
    entry.vsze   = 0;
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
    if (index >= 0 && index < (int)btab.size())
        return &btab[index];
    return nullptr;
}

// =========================================================
// SymbolTable: Print symbol table (output)
// =========================================================

void SymbolTable::printTab(ostream& out) const {
    out << "\n=== Symbol Table (tab) ===\n";
    out << left
        << setw(5)  << "idx"
        << setw(16) << "identifier"
        << setw(5)  << "link"
        << setw(12) << "obj"
        << setw(10) << "type"
        << setw(5)  << "ref"
        << setw(5)  << "nrm"
        << setw(5)  << "lev"
        << setw(5)  << "adr"
        << "\n";
    out << string(68, '-') << "\n";
    for (int i = 0; i < (int)tab.size(); i++) {
        const TabEntry& e = tab[i];
        // Lewati slot placeholder reserved words (indeks 0-32) kecuali yg penting
        if (i < 33 && e.identifiers.empty()) continue;
        out << left
            << setw(5)  << i
            << setw(16) << e.identifiers
            << setw(5)  << e.link
            << setw(12) << objClassName(e.obj)
            << setw(10) << dataTypeName(e.type)
            << setw(5)  << e.ref
            << setw(5)  << e.nrm
            << setw(5)  << e.lev
            << setw(5)  << e.adr
            << "\n";
    }
}

void SymbolTable::printBTab(ostream& out) const {
    out << "\n=== Block Table (btab) ===\n";
    out << left
        << setw(8)  << "blocks"
        << setw(8)  << "last"
        << setw(8)  << "lpar"
        << setw(8)  << "psze"
        << setw(8)  << "vsze"
        << "\n";
    out << string(40, '-') << "\n";
    for (const auto& e : btab) {
        out << left
            << setw(8) << e.blocks
            << setw(8) << e.last
            << setw(8) << e.lpar
            << setw(8) << e.psze
            << setw(8) << e.vsze
            << "\n";
    }
}

void SymbolTable::printATab(ostream& out) const {
    out << "\n=== Array Table (atab) ===\n";
    if (atab.empty()) {
        out << "(kosong)\n";
        return;
    }
    out << left
        << setw(8)  << "arrays"
        << setw(10) << "xtyp"
        << setw(10) << "etyp"
        << setw(6)  << "eref"
        << setw(6)  << "low"
        << setw(6)  << "high"
        << setw(6)  << "elsz"
        << setw(6)  << "size"
        << "\n";
    out << string(58, '-') << "\n";
    for (const auto& e : atab) {
        out << left
            << setw(8)  << e.arrays
            << setw(10) << dataTypeName(e.xtyp)
            << setw(10) << dataTypeName(e.etyp)
            << setw(6)  << e.eref
            << setw(6)  << e.low
            << setw(6)  << e.high
            << setw(6)  << e.elsz
            << setw(6)  << e.size
            << "\n";
    }
}

// =========================================================
// Accept dispatch untuk node deklarasi & program
// =========================================================

void ProgramNode::accept(SemanticVisitor* v)     { v->visit(this); }
void VarDeclNode::accept(SemanticVisitor* v)     { v->visit(this); }
void ConstDeclNode::accept(SemanticVisitor* v)   { v->visit(this); }
void SubprogDeclNode::accept(SemanticVisitor* v) { v->visit(this); }

// =========================================================
// Print AST (Decorated AST output)
// =========================================================

void ProgramNode::print(ostream& out, int indent, const SymbolTable* st) const {
    printIndent(out, indent);
    out << "ProgramNode(name: '" << name << "')"
        << " → tab_index:" << symRef
        << ", lev:" << lexicalLevel << "\n";

    if (!declarations.empty()) {
        printIndent(out, indent + 1);
        out << "Declarations:\n";
        for (auto* d : declarations)
            if (d) d->print(out, indent + 2, st);
    }
    if (mainBlock) {
        printIndent(out, indent + 1);
        out << "Block:\n";
        mainBlock->print(out, indent + 2, st);
    }
}

void VarDeclNode::print(ostream& out, int indent, const SymbolTable* st) const {
    for (const auto& id : idents) {
        printIndent(out, indent);
        out << "VarDecl('" << id << "')"
            << " → type:" << dtStr(type)
            << ", tab_index:" << symRef
            << ", lev:" << lexicalLevel;
        if (type == DataType::ARRAY)
            out << ", ref(atab):" << ref
                << ", [" << lowBound << ".." << highBound << "]"
                << " of " << dtStr(elementType);
        out << "\n";
    }
}

void ConstDeclNode::print(ostream& out, int indent, const SymbolTable* st) const {
    printIndent(out, indent);
    out << "ConstDecl('" << name << "' = " << value << ")"
        << " → type:" << dtStr(type)
        << ", tab_index:" << symRef
        << ", lev:" << lexicalLevel << "\n";
}

void SubprogDeclNode::print(ostream& out, int indent, const SymbolTable* st) const {
    printIndent(out, indent);
    out << (isFunction ? "FunctionDecl" : "ProcedureDecl")
        << "('" << name << "')"
        << " → retType:" << dtStr(retType)
        << ", tab_index:" << symRef
        << ", lev:" << lexicalLevel << "\n";
    if (!params.empty()) {
        printIndent(out, indent + 1);
        out << "Params:\n";
        for (auto* p : params)
            if (p) p->print(out, indent + 2, st);
    }
    if (block) {
        printIndent(out, indent + 1);
        out << "Body:\n";
        block->print(out, indent + 2, st);
    }
}

// =========================================================
// SemanticAnalyzer: visit(ProgramNode)
// Entry point analisis semantik
// =========================================================

void SemanticAnalyzer::visit(ProgramNode* node) {
    // 1. Inisialisasi symbol table dengan predefined identifiers
    st.initPredefined();

    // 2. Buat btab global block (btab[0])
    int globalBlock = st.insertBTab();

    // 3. Registrasi nama program ke tab
    node->symRef      = st.insertTab(node->name, ObjClass::PROCEDURE, DataType::NONE,
                                     globalBlock, 0, 0);
    node->lexicalLevel = st.currentLevel;

    // 4. Push scope level 1 untuk isi program
    st.pushScope();
    int startIdx = (int)st.tab.size() - 1;

    // 5. Proses semua deklarasi
    for (ASTNode* decl : node->declarations)
        if (decl) decl->accept(this);

    // 6. Proses compound statement utama
    if (node->mainBlock)
        node->mainBlock->accept(this);

    // 7. Update btab global dengan info identifier terakhir
    int endIdx = (int)st.tab.size() - 1;
    // Hitung vsze: jumlah variabel di level global (level 1)
    int vsze = 0;
    for (int i = startIdx + 1; i <= endIdx; i++) {
        if (st.tab[i].obj == ObjClass::VARIABLE && st.tab[i].lev == st.currentLevel)
            vsze++;
    }
    st.updateBTab(globalBlock, endIdx, 0, 0, vsze);

    st.popScope();
}

// =========================================================
// SemanticAnalyzer: visit(VarDeclNode)
// Registrasi variabel ke symbol table + validasi multiple declaration
// =========================================================

void SemanticAnalyzer::visit(VarDeclNode* node) {
    // Jika array: daftarkan metadata ke atab
    if (node->type == DataType::ARRAY) {
        // Validasi: index type tidak boleh Real
        if (node->indexType == DataType::REAL) {
            semanticError("Array index type tidak boleh bertipe Real.");
        }
        // Hitung elsz berdasarkan tipe elemen (sederhana: 1 unit)
        int elsz = (node->elementType == DataType::REAL) ? 2 : 1;
        node->ref = st.insertATab(node->indexType, node->elementType, 0,
                                  node->lowBound, node->highBound, elsz);
    }

    // Alokasikan address counter untuk variabel lokal
    // (hitung offset dalam blok saat ini)
    int adr = 0;
    for (int i = (int)st.tab.size() - 1; i >= 0; i--) {
        if (st.tab[i].lev < st.currentLevel) break;
        if (st.tab[i].obj == ObjClass::VARIABLE && st.tab[i].lev == st.currentLevel) {
            adr = st.tab[i].adr + 1;
            break;
        }
    }

    int firstSymRef = -1;
    for (const string& ident : node->idents) {
        // Cek multiple declaration di scope yang sama
        if (st.lookupLocalTab(ident) != nullptr) {
            semanticError("Multiple declaration: variabel '" + ident +
                          "' sudah dideklarasikan di scope yang sama.");
            continue;
        }
        int idx = st.insertTab(ident, ObjClass::VARIABLE, node->type,
                               node->ref, st.currentLevel, adr, 1);
        if (firstSymRef == -1) firstSymRef = idx;
        adr++;
    }
    node->symRef      = firstSymRef;
    node->lexicalLevel = st.currentLevel;
    node->evalType    = node->type;
}

// =========================================================
// SemanticAnalyzer: visit(ConstDeclNode)
// Registrasi konstanta ke symbol table
// =========================================================

void SemanticAnalyzer::visit(ConstDeclNode* node) {
    if (st.lookupLocalTab(node->name) != nullptr) {
        semanticError("Multiple declaration: konstanta '" + node->name +
                      "' sudah dideklarasikan di scope yang sama.");
        return;
    }
    // Nilai konstanta disimpan di adr (untuk integer/boolean),
    // untuk string/char/real cukup 0 sebagai placeholder
    int adrVal = 0;
    if (node->type == DataType::INTEGER || node->type == DataType::BOOLEAN) {
        try { adrVal = stoi(node->value); } catch (...) { adrVal = 0; }
    }
    node->symRef      = st.insertTab(node->name, ObjClass::CONSTANT, node->type,
                                     0, st.currentLevel, adrVal);
    node->lexicalLevel = st.currentLevel;
    node->evalType    = node->type;
}

// =========================================================
// SemanticAnalyzer: visit(SubprogDeclNode)
// Registrasi dan analisis prosedur/fungsi
// =========================================================

void SemanticAnalyzer::visit(SubprogDeclNode* node) {
    // Cek multiple declaration di scope lokal
    if (st.lookupLocalTab(node->name) != nullptr) {
        semanticError("Multiple declaration: subprogram '" + node->name +
                      "' sudah dideklarasikan di scope yang sama.");
        return;
    }

    // Buat btab entry untuk blok prosedur/fungsi ini
    int blockIdx = st.insertBTab();

    // Registrasi nama prosedur/fungsi ke tab
    ObjClass objType = node->isFunction ? ObjClass::FUNCTION : ObjClass::PROCEDURE;
    node->symRef      = st.insertTab(node->name, objType, node->retType,
                                     blockIdx, st.currentLevel, 0);
    node->lexicalLevel = st.currentLevel;

    // Push scope untuk parameter dan variabel lokal
    st.pushScope();

    // Catat indeks awal parameter
    int paramStartIdx = (int)st.tab.size();

    // Proses parameter (disimpan di node->params sebagai VarDeclNode)
    for (ASTNode* param : node->params)
        if (param) param->accept(this);

    // Hitung indeks parameter terakhir (lpar)
    int lpar = 0;
    int psze = 0;
    if ((int)st.tab.size() > paramStartIdx) {
        lpar = (int)st.tab.size() - 1;
        psze = (int)st.tab.size() - paramStartIdx;
    }

    // Proses body (compound statement)
    if (node->block)
        node->block->accept(this);

    // Hitung variabel lokal (setelah parameter)
    int lastIdx = (int)st.tab.size() - 1;
    int vsze    = 0;
    for (int i = lpar + 1; i <= lastIdx; i++) {
        if (st.tab[i].obj == ObjClass::VARIABLE && st.tab[i].lev == st.currentLevel)
            vsze++;
    }

    // Update btab dengan info lengkap blok ini
    st.updateBTab(blockIdx, lastIdx, lpar, psze, vsze);

    st.popScope();
    node->evalType = node->retType;
}