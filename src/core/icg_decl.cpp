#include "icg_visitor.h"

// Mengimplementasikan fungsi-fungsi dari ICGVisitor yang mengurus Deklarasi

void ICGVisitor::visit(ProgramNode* node) {
    // TODO: Bikin instruksi INT m untuk alokasi memori utama
    // Lalu telusuri isi blok program
    // Menghitung jumlah variabel di level global (Block Index 0)
    int numVars = 0;
    if (!st.btab.empty()) {
        numVars = st.btab[0].vsze;
    }

    // Fallback: jika btab belum di-update, hitung manual dari tab
    if (numVars == 0) {
        for (size_t i = 33; i < st.tab.size(); i++) {
            if (st.tab[i].obj == ObjClass::VARIABLE && st.tab[i].lev == 0) {
                numVars++;
            }
        }
    }
    
    // Bikin instruksi INT m untuk alokasi memori utama. 
    // m = 3 blok dasar (Static Link, Dynamic Link, Return Address) + jumlah variabel
    instructions.push_back(Instruction(OpCode::INT, 0, 3 + numVars));

    // Telusuri isi blok program (Deklarasi)
    for (ASTNode* decl : node->declarations) {
        if (decl) decl->accept(this);
    }

    // Telusuri blok main statement
    if (node->mainBlock) {
        node->mainBlock->accept(this);
    }

    // Akhiri eksekusi program
    instructions.push_back(Instruction(OpCode::RET, 0, 0));
}

void ICGVisitor::visit(VarDeclNode* node) {
    // TODO: Alokasikan ukuran memori untuk variabel/array
    // Alokasi memori untuk variabel/array secara teknis sudah di-handle oleh instruksi INT
    // di awal blok program/fungsi. Oleh karena itu, bagian ini bisa dibiarkan kosong.
}

void ICGVisitor::visit(ConstDeclNode* node) {
    // TODO: Alokasikan memori untuk konstanta
    // Konstanta tidak disimpan di blok memori The Stack, karena nilainya 
    // langsung diload saat runtime menggunakan instruksi LIT.
}

void ICGVisitor::visit(SubprogDeclNode* node) {
    // TODO: Bikin instruksi CAL dan siapkan Stack Frame. Akhiri dengan RET
    // Catat "nomor baris instruksi" tempat fungsi ini bermula ke dalam Symbol Table
    // agar instruksi CAL tahu harus melompat (jump) ke baris mana.
    if (node->symRef != -1 && node->symRef < (int)st.tab.size()) {
        st.tab[node->symRef].adr = instructions.size();
    }

    // Hitung jumlah alokasi lokal yang dibutuhkan untuk Parameter dan Variabel Fungsi
    // vsze di btab sudah mencakup semua variabel (termasuk parameter), 
    // jadi kita cukup gunakan vsze untuk alokasi memori lokal.
    int numVars = 0;
    if (node->symRef != -1 && st.tab[node->symRef].ref < (int)st.btab.size()) {
        int blockIdx = st.tab[node->symRef].ref;
        numVars = st.btab[blockIdx].vsze;
    }

    // Inisiasi Stack Frame untuk fungsi: 3 (SL, DL, RA) + variabel lokal (termasuk param)
    instructions.push_back(Instruction(OpCode::INT, 0, 3 + numVars));

    // Eksekusi semua kode/statement yang ada di dalam badan fungsi
    if (node->block) {
        node->block->accept(this);
    }

    // Bikin instruksi RET untuk menghancurkan Stack Frame dan kembali ke pemanggil
    instructions.push_back(Instruction(OpCode::RET, 0, 0));
}