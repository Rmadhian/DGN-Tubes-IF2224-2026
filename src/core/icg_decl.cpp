#include "icg_visitor.h"

// Mengimplementasikan fungsi-fungsi dari ICGVisitor yang mengurus Deklarasi

void ICGVisitor::visit(ProgramNode* node) {
    // Menghitung jumlah variabel di level global (Block Index 0).
    int numVars = 0;
    if (!st.btab.empty()) {
        numVars = st.btab[0].vsze;
    }

    // Menghitung jumlah variabel secara manual jika btab belum terisi.
    if (numVars == 0) {
        for (size_t i = 33; i < st.tab.size(); i++) {
            if (st.tab[i].obj == ObjClass::VARIABLE && st.tab[i].lev == 0) {
                numVars++;
            }
        }
    }
    
    // Alokasi memori dasar (SL, DL, RA) ditambah jumlah variabel.
    instructions.push_back(Instruction(OpCode::INT, 0, 3 + numVars));

    // Menelusuri seluruh deklarasi di dalam blok program.
    for (ASTNode* decl : node->declarations) {
        if (decl) decl->accept(this);
    }

    // Mengeksekusi blok pernyataan utama program.
    if (node->mainBlock) {
        node->mainBlock->accept(this);
    }

    // Menginstruksikan keluar dari program utama.
    instructions.push_back(Instruction(OpCode::RET, 0, 0));
}

void ICGVisitor::visit(VarDeclNode* node) {
    // Alokasi memori variabel ditangani secara akumulatif melalui instruksi INT awal.
}

void ICGVisitor::visit(ConstDeclNode* node) {
    // Nilai konstanta diload secara dinamis saat runtime sehingga tidak memerlukan alokasi.
}

void ICGVisitor::visit(SubprogDeclNode* node) {
    // Mencatat alamat awal eksekusi subprogram ke dalam Symbol Table.
    if (node->symRef != -1 && node->symRef < (int)st.tab.size()) {
        st.tab[node->symRef].adr = instructions.size();
    }

    // Menghitung jumlah alokasi memori lokal yang dibutuhkan dari btab.
    int numVars = 0;
    if (node->symRef != -1 && st.tab[node->symRef].ref < (int)st.btab.size()) {
        int blockIdx = st.tab[node->symRef].ref;
        numVars = st.btab[blockIdx].vsze;
    }

    // Menginisiasi Stack Frame untuk fungsi dengan alokasi dasar dan variabel lokal.
    instructions.push_back(Instruction(OpCode::INT, 0, 3 + numVars));

    // Mengeksekusi pernyataan di dalam subprogram.
    if (node->block) {
        node->block->accept(this);
    }

    // Menambahkan instruksi RET untuk mengakhiri eksekusi subprogram.
    instructions.push_back(Instruction(OpCode::RET, 0, 0));
}