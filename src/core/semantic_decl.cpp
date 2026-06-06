#include "semantic.h"
#include <iostream>

using namespace std;

// Inisialisasi identifier bawaan (reserved words dan tipe dasar)
void SymbolTable::initPredefined() {
    currentLevel = 0;

    // 1. Mengisi indeks 0 s.d 32 dengan dummy/reserved words sesuai spesifikasi
    string reservedWords[33] = {
        "empty", "and", "array", "begin", "case", "const", "div", "downto", 
        "do", "else", "end", "for", "function", "if", "mod", "not", "of", 
        "or", "procedure", "program", "record", "repeat", "integer", "real", 
        "boolean", "char", "string", "then", "to", "type", "until", "var", "while"
    };

    for (int i = 0; i <= 32; i++) {
        TabEntry entry;
        entry.identifiers = reservedWords[i];
        entry.obj = ObjClass::CONSTANT; 
        entry.type = DataType::NONE;
        entry.ref = 0; entry.nrm = 0; entry.lev = 0; entry.adr = 0; entry.link = 0;
        tab.push_back(entry);
    }

    // 2. Predefined identifiers (Mulai dari indeks 33)
    insertTab("integer", ObjClass::TYPE_DEF, DataType::INTEGER, 0, 0, 0);
    insertTab("real", ObjClass::TYPE_DEF, DataType::REAL, 0, 0, 0);
    insertTab("char", ObjClass::TYPE_DEF, DataType::CHAR, 0, 0, 0);
    insertTab("boolean", ObjClass::TYPE_DEF, DataType::BOOLEAN, 0, 0, 0);
    insertTab("string", ObjClass::TYPE_DEF, DataType::STRING, 0, 0, 0);

    // Konstanta boolean dan predefined functions
    insertTab("true", ObjClass::CONSTANT, DataType::BOOLEAN, 0, 0, 1);
    insertTab("false", ObjClass::CONSTANT, DataType::BOOLEAN, 0, 0, 0);
    insertTab("writeln", ObjClass::PROCEDURE, DataType::NONE, 0, 0, 0);
    insertTab("readln", ObjClass::PROCEDURE, DataType::NONE, 0, 0, 0);

    int globalBlockIdx = insertBTab();
    activeBlocks.push_back(globalBlockIdx);
}

// Insert entry baru ke tab dengan link yang tepat, return indeks
int SymbolTable::insertTab(string name, ObjClass obj, DataType type, int ref, int lev, int adr) {
    TabEntry entry;
    entry.identifiers = name;
    entry.obj = obj;
    entry.type = type;
    entry.ref = ref;
    entry.lev = (lev == -1) ? currentLevel : lev;
    entry.adr = adr;
    
    // Set parameter nrm (1 untuk variabel normal, 0 untuk reference)
    entry.nrm = (obj == ObjClass::VARIABLE || obj == ObjClass::CONSTANT) ? 1 : 0;

    // Mencari identifier sebelumnya di scope (level) yang sama untuk dihubungkan via link
    int lastLink = 0;
    for (int i = (int)tab.size() - 1; i >= 33; i--) {
        if (tab[i].lev == entry.lev) {
            lastLink = i;
            break;
        }
    }
    entry.link = lastLink;
    
    tab.push_back(entry);
    return tab.size() - 1;
}

TabEntry* SymbolTable::lookupLocalTab(string name) {
    for (int i = tab.size() - 1; i >= 33; i--) {
        if (tab[i].lev < currentLevel) break; 
        if (tab[i].lev == currentLevel && tab[i].identifiers == name) return &tab[i];
    }
    return nullptr;
}

TabEntry* SymbolTable::getTab(int index) {
    if (index >= 0 && index < tab.size()) return &tab[index];
    return nullptr;
}

TabEntry* SymbolTable::lookupTab(string name) {
    // Mulai dari blok paling dalam (scope saat ini) ke blok global
    for (int i = activeBlocks.size() - 1; i >= 0; i--) {
        int blockIdx = activeBlocks[i];
        int currIdx = btab[blockIdx].last; // Mulai dari identifier terakhir di blok ini
        
        while (currIdx > 0 && currIdx < tab.size()) {
            if (tab[currIdx].identifiers == name) {
                return &tab[currIdx];
            }
            currIdx = tab[currIdx].link; // Lompat ke identifier sebelumnya di scope yang sama
        }
    }
    
    // Fallback: periksa reserved words (indeks 0 - 32)
    for (int i = 32; i >= 0; i--) {
        if (tab[i].identifiers == name) return &tab[i];
    }
    
    return nullptr; // Tidak ditemukan di scope manapun
}

ATabEntry* SymbolTable::getATab(int index) {
    if (index < 0 || index >= (int)atab.size()) return nullptr;
    return &atab[index];
}

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

void ProgramNode::accept(SemanticVisitor* visitor)     { visitor->visit(this); }
void VarDeclNode::accept(SemanticVisitor* visitor)     { visitor->visit(this); }
void ConstDeclNode::accept(SemanticVisitor* visitor)   { visitor->visit(this); }
void SubprogDeclNode::accept(SemanticVisitor* visitor) { visitor->visit(this); }

// Visitor: ProgramNode 
void SemanticAnalyzer::visit(ProgramNode* node) {
    st.initPredefined();

    int globalBlockIdx = 0;
    
    node->symRef = st.insertTab(node->name, ObjClass::PROCEDURE, DataType::NONE);
    node->lexicalLevel = st.currentLevel;

    for (ASTNode* decl : node->declarations)
        if (decl) decl->accept(this);
        
    // Update vsze (jumlah variabel) & last identifier untuk btab[0]
    int lastVar = 0;
    int vsze = 0;
    for (int i = 33; i < st.tab.size(); i++) {
        if (st.tab[i].lev == 0) {
            lastVar = i;
            if (st.tab[i].obj == ObjClass::VARIABLE) vsze++;
        }
    }
    st.updateBTab(globalBlockIdx, lastVar, 0, 0, vsze);

    if (node->mainBlock)
        node->mainBlock->accept(this);
}

// Visitor: VarDeclNode
void SemanticAnalyzer::visit(VarDeclNode* node) {
    for (const string& ident : node->idents) {
        if (st.lookupLocalTab(ident) != nullptr) {
            cerr << "Semantic Error: Multiple declaration of variable '"
                 << ident << "' in the same scope." << endl;
            // Dihilangkan exit(EXIT_FAILURE) agar memenuhi syarat error handling tidak crash
        } else {
            if (node->type == DataType::ARRAY) {
                // Parameter: tipe index (INTEGER), tipe elemen, array ref(0), low, high, elsz(1)
                node->ref = st.insertATab(DataType::INTEGER, node->elementType, 0, node->lowBound, node->highBound, 1);
            }
            // Hitung address variabel berdasarkan jumlah variabel di scope saat ini (mulai dari offset 3)
            int varCount = 0;
            for (int i = 33; i < (int)st.tab.size(); i++) {
                if (st.tab[i].lev == st.currentLevel && st.tab[i].obj == ObjClass::VARIABLE) {
                    varCount++;
                }
            }
            int address = 3 + varCount;
            node->symRef = st.insertTab(ident, ObjClass::VARIABLE, node->type, node->ref, -1, address);
            node->lexicalLevel = st.currentLevel;

            // Update vsze di btab agar icg_decl tahu jumlah variabel di blok ini
            if (!st.activeBlocks.empty()) {
                int blockIdx = st.activeBlocks.back();
                st.btab[blockIdx].vsze++;
                st.btab[blockIdx].last = node->symRef;
            }
        }
    }
}

// Visitor: ConstDeclNode
void SemanticAnalyzer::visit(ConstDeclNode* node) {
    if (st.lookupLocalTab(node->name) != nullptr) {
        cerr << "Semantic Error: Multiple declaration of constant '"
             << node->name << "' in the same scope." << endl;
    } else {
        int val = 0;
        if (node->type == DataType::INTEGER) {
            try { val = std::stoi(node->value); } catch (...) { val = 0; }
        } else if (node->type == DataType::BOOLEAN) {
            val = (node->value == "true" || node->value == "1") ? 1 : 0;
        } else if (node->type == DataType::CHAR) {
            val = (node->value.length() > 0) ? (int)node->value[0] : 0;
        }
        
        node->symRef = st.insertTab(node->name, ObjClass::CONSTANT, node->type, 0, 0, val);
        node->lexicalLevel = st.currentLevel;
    }
}

// Visitor: SubprogDeclNode
void SemanticAnalyzer::visit(SubprogDeclNode* node) {
    if (st.lookupLocalTab(node->name) != nullptr) {
        cerr << "Semantic Error: Multiple declaration of subprogram '"
             << node->name << "' in the same scope." << endl;
    }

    ObjClass objType = node->isFunction ? ObjClass::FUNCTION : ObjClass::PROCEDURE;
    node->symRef = st.insertTab(node->name, objType, node->retType);
    node->lexicalLevel = st.currentLevel;

    st.pushScope();
    int blockIdx = st.insertBTab(); 
    st.activeBlocks.push_back(blockIdx);
    
    // Hubungkan prosedur/fungsi di tab ke btab blocknya melalui referensi (ref)
    if (node->symRef != -1) {
        st.tab[node->symRef].ref = blockIdx;
    }

    for (ASTNode* param : node->params) {
        if (param) param->accept(this);
    }

    int lastParamIdx = 0;
    int paramSize = 0;
    
    // Cari parameter terakhir dari belakang tab
    for (int i = st.tab.size() - 1; i >= 33; i--) {
        if (st.tab[i].lev == st.currentLevel) {
            if (lastParamIdx == 0) lastParamIdx = i; // Ini parameter terakhir
            paramSize++; // Asumsi 1 param = 1 ukuran dasar
        } else if (st.tab[i].lev < st.currentLevel) {
            break;
        }
    }

    if (node->block) node->block->accept(this);

    // Sekarang, cari last identifier (termasuk variabel lokal yang di-declare di dalam block)
    int lastVarIdx = 0;
    int vsze = 0;
    for (int i = st.tab.size() - 1; i >= 33; i--) {
        if (st.tab[i].lev == st.currentLevel) {
            if (lastVarIdx == 0) lastVarIdx = i;
            if (st.tab[i].obj == ObjClass::VARIABLE) vsze++;
        } else if (st.tab[i].lev < st.currentLevel) {
            break;
        }
    }

    // Update BTab untuk lpar, psze, dan last
    st.updateBTab(blockIdx, lastVarIdx, lastParamIdx, paramSize, vsze);

    st.activeBlocks.pop_back(); // Keluar dari scope
    st.popScope();
}