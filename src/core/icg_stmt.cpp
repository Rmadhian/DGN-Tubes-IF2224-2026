#include "icg_visitor.h"

// Blok begin..end : telusuri semua statement secara berurutan
void ICGVisitor::visit(CompoundStmtNode* node) {
    for (size_t i = 0; i < node->statements.size(); i++) {
        dispatch(node->statements[i]);
    }
}

// IF-THEN dan IF-THEN-ELSE
void ICGVisitor::visit(IfStmtNode* node) {
    // 1. Hitung kondisi -> hasil boolean ada di stack teratas
    dispatch(node->condition);

    // 2. Kalau kondisi false, lompat melewati blok THEN.
    //    Alamat tujuan belum tahu, jadi diisi 0 dulu lalu di-backpatch.
    int jpcIndex = instructions.size();
    instructions.push_back(Instruction(OpCode::JPC, 0, 0));

    // 3. Blok THEN
    dispatch(node->thenStmt);

    if (node->elseStmt != nullptr) {
        // Ada ELSE: setelah THEN selesai, harus lompat melewati blok ELSE
        int jmpIndex = instructions.size();
        instructions.push_back(Instruction(OpCode::JMP, 0, 0));

        // Blok ELSE mulai di sini, jadi JPC tadi diarahkan ke sini
        int elseStart = instructions.size();
        instructions[jpcIndex].a = elseStart;

        dispatch(node->elseStmt);

        // Akhir dari seluruh if, JMP setelah THEN diarahkan ke sini
        int endPos = instructions.size();
        instructions[jmpIndex].a = endPos;
    } else {
        // Tanpa ELSE: kalau kondisi false langsung lompat ke akhir if
        int endPos = instructions.size();
        instructions[jpcIndex].a = endPos;
    }
}

// WHILE: cek kondisi di awal, lompat keluar kalau false, lompat mundur di akhir
void ICGVisitor::visit(WhileStmtNode* node) {
    // Alamat awal pengecekan kondisi (tempat kita lompat mundur tiap iterasi)
    int condStart = instructions.size();

    // 1. Hitung kondisi
    dispatch(node->condition);

    // 2. Kalau false, keluar dari loop (alamat di-backpatch nanti)
    int jpcIndex = instructions.size();
    instructions.push_back(Instruction(OpCode::JPC, 0, 0));

    // 3. Badan loop
    dispatch(node->body);

    // 4. Lompat mundur ke pengecekan kondisi
    instructions.push_back(Instruction(OpCode::JMP, 0, condStart));

    // 5. Titik keluar loop
    int endPos = instructions.size();
    instructions[jpcIndex].a = endPos;
}

// FOR: for i := start to/downto end do body
// Strukturnya kita ubah jadi mirip while:
//   i := start
//   L: kalau (i <= end) [untuk 'to'] tidak terpenuhi -> keluar
//      body
//      i := i + 1   (atau i - 1 untuk 'downto')
//      lompat ke L
// Batas akhir (end) dihitung ulang tiap iterasi supaya tidak perlu variabel
// sementara tambahan; cukup untuk kebutuhan Arion.
void ICGVisitor::visit(ForStmtNode* node) {
    int iterAddr = addressOf(node->iterVar);

    // 1. Inisialisasi: i := start
    dispatch(node->startExpr);                              // hasil start di stack
    instructions.push_back(Instruction(OpCode::STO, 0, iterAddr));

    // 2. Awal pengecekan kondisi
    int condStart = instructions.size();

    // Muat i lalu hitung batas akhir, lalu bandingkan.
    // 'to'     -> lanjut selama i <= end  (OPR LEQ)
    // 'downto' -> lanjut selama i >= end  (OPR GEQ)
    instructions.push_back(Instruction(OpCode::LOD, 0, iterAddr));
    dispatch(node->endExpr);

    if (node->isDownto) {
        instructions.push_back(Instruction(OpCode::OPR, 0, (int)OprCode::GEQ));
    } else {
        instructions.push_back(Instruction(OpCode::OPR, 0, (int)OprCode::LEQ));
    }

    // 3. Kalau kondisi false, keluar dari loop
    int jpcIndex = instructions.size();
    instructions.push_back(Instruction(OpCode::JPC, 0, 0));

    // 4. Badan loop
    dispatch(node->body);

    // 5. Update iterator: i := i +/- 1
    instructions.push_back(Instruction(OpCode::LOD, 0, iterAddr));
    instructions.push_back(Instruction(OpCode::LIT, 0, 1));
    if (node->isDownto) {
        instructions.push_back(Instruction(OpCode::OPR, 0, (int)OprCode::SUB));
    } else {
        instructions.push_back(Instruction(OpCode::OPR, 0, (int)OprCode::ADD));
    }
    instructions.push_back(Instruction(OpCode::STO, 0, iterAddr));

    // 6. Lompat mundur untuk cek kondisi lagi
    instructions.push_back(Instruction(OpCode::JMP, 0, condStart));

    // 7. Titik keluar loop
    int endPos = instructions.size();
    instructions[jpcIndex].a = endPos;
}
