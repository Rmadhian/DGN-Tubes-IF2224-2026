#include "icg_visitor.h"

// Mengimplementasikan fungsi-fungsi dari ICGVisitor yang mengurus Deklarasi

void ICGVisitor::visit(ProgramNode* node) {
    // TODO: Bikin instruksi INT m untuk alokasi memori utama
    // Lalu telusuri isi blok program
}

void ICGVisitor::visit(VarDeclNode* node) {
    // TODO: Alokasikan ukuran memori untuk variabel/array
}

void ICGVisitor::visit(ConstDeclNode* node) {
    // TODO: Alokasikan memori untuk konstanta
}

void ICGVisitor::visit(SubprogDeclNode* node) {
    // TODO: Bikin instruksi CAL dan siapkan Stack Frame. Akhiri dengan RET
}