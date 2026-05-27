#include "icg_visitor.h"
#include <iostream>

// Mengimplementasikan fungsi-fungsi dari ICGVisitor yang mengurus Ekspresi

void ICGVisitor::visit(LiteralNode* node) {
    // TODO: Bikin instruksi LIT v
}

void ICGVisitor::visit(VarAccessNode* node) {
    // TODO: Bikin instruksi LOD a
}

void ICGVisitor::visit(BinaryOpNode* node) {
    // TODO: Telusuri node kiri, node kanan, lalu bikin instruksi OPR (Matematika/Logika)
}

void ICGVisitor::visit(UnaryOpNode* node) {
    // TODO: Bikin instruksi OPR 1 (NEG)
}

void ICGVisitor::visit(FuncCallNode* node) {
    // TODO: Setup parameter dan panggil fungsi
}

void ICGVisitor::visit(AssignStmtNode* node) {
    // TODO: Eksekusi ruas kanan, lalu bikin instruksi STO a
}

void ICGVisitor::visit(WriteStatementNode* node) {
    // TODO: Bikin instruksi WRT atau WRTLN
}