#include "icg_visitor.h"

// Mengimplementasikan fungsi-fungsi dari ICGVisitor yang mengurus Statement & Alur Kontrol

void ICGVisitor::visit(CompoundStmtNode* node) {
    // TODO: Telusuri seluruh statement di dalam blok begin..end
}

void ICGVisitor::visit(IfStmtNode* node) {
    // TODO: Bikin instruksi JPC (IF_FALSE) dan JMP untuk meratakan percabangan
}

void ICGVisitor::visit(WhileStmtNode* node) {
    // TODO: Bikin instruksi loop (JMP mundur ke kondisi, JPC keluar dari loop)
}

void ICGVisitor::visit(ForStmtNode* node) {
    // TODO: Bikin instruksi loop untuk For
}