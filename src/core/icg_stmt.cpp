#include "icg_visitor.h"

// Mengeksekusi secara berurutan semua pernyataan di dalam blok utama.
void ICGVisitor::visit(CompoundStmtNode* node) {
    for (size_t i = 0; i < node->statements.size(); i++) {
        dispatch(node->statements[i]);
    }
}

// Menghasilkan instruksi ICG untuk struktur kontrol percabangan IF-THEN dan IF-THEN-ELSE.
void ICGVisitor::visit(IfStmtNode* node) {
    // Mengevaluasi kondisi dan menyimpan hasilnya sebagai boolean di stack teratas.
    dispatch(node->condition);

    // Menambahkan instruksi lompatan bersyarat (JPC) untuk mengeksekusi lompatan jika kondisi salah.
    int jpcIndex = instructions.size();
    instructions.push_back(Instruction(OpCode::JPC, 0, 0));

    // Menghasilkan blok eksekusi THEN.
    dispatch(node->thenStmt);

    if (node->elseStmt != nullptr) {
        // Menambahkan instruksi lompatan tidak bersyarat (JMP) untuk melewati blok ELSE setelah eksekusi blok THEN.
        int jmpIndex = instructions.size();
        instructions.push_back(Instruction(OpCode::JMP, 0, 0));

        // Melakukan backpatching agar JPC sebelumnya melompat ke awal blok ELSE.
        int elseStart = instructions.size();
        instructions[jpcIndex].a = elseStart;

        dispatch(node->elseStmt);

        // Melakukan backpatching agar JMP setelah THEN mengarah ke akhir struktur IF.
        int endPos = instructions.size();
        instructions[jmpIndex].a = endPos;
    } else {
        // Melakukan backpatching agar JPC mengarah langsung ke akhir blok jika tidak ada ELSE.
        int endPos = instructions.size();
        instructions[jpcIndex].a = endPos;
    }
}

// Menghasilkan instruksi ICG untuk perulangan bersyarat WHILE-DO.
void ICGVisitor::visit(WhileStmtNode* node) {
    // Menyimpan alamat awal blok kondisi untuk lompatan iterasi.
    int condStart = instructions.size();

    // Mengevaluasi kondisi loop.
    dispatch(node->condition);

    // Menambahkan instruksi JPC yang akan di-backpatch ke titik keluar loop.
    int jpcIndex = instructions.size();
    instructions.push_back(Instruction(OpCode::JPC, 0, 0));

    // Mengeksekusi blok pernyataan di dalam loop.
    dispatch(node->body);

    // Mengembalikan eksekusi ke awal blok kondisi.
    instructions.push_back(Instruction(OpCode::JMP, 0, condStart));

    // Menyelesaikan backpatching pada instruksi JPC menuju titik akhir perulangan.
    int endPos = instructions.size();
    instructions[jpcIndex].a = endPos;
}

// Menghasilkan instruksi ICG untuk struktur perulangan terbatas FOR-DO.
void ICGVisitor::visit(ForStmtNode* node) {
    TabEntry* iterEntry = getTabEntry(node->iterVar);
    if (!iterEntry) {
        std::cerr << "ICG Error: For loop iterator '" << node->iterVar << "' tidak ditemukan." << std::endl;
        return;
    }
    int iterAddr = iterEntry->adr;
    int levelDiff = node->lexicalLevel - iterEntry->lev;

    // Menginisialisasi nilai awal iterator.
    dispatch(node->startExpr);                              // hasil start di stack
    instructions.push_back(Instruction(OpCode::STO, levelDiff, iterAddr));

    // Mencatat alamat awal pengecekan batas perulangan.
    int condStart = instructions.size();

    // Mengevaluasi dan membandingkan variabel iterator terhadap batas akhir.
    instructions.push_back(Instruction(OpCode::LOD, levelDiff, iterAddr));
    dispatch(node->endExpr);

    if (node->isDownto) {
        instructions.push_back(Instruction(OpCode::OPR, 0, (int)OprCode::GEQ));
    } else {
        instructions.push_back(Instruction(OpCode::OPR, 0, (int)OprCode::LEQ));
    }

    // Menambahkan instruksi JPC yang akan di-backpatch saat keluar dari perulangan.
    int jpcIndex = instructions.size();
    instructions.push_back(Instruction(OpCode::JPC, 0, 0));

    // Mengeksekusi blok pernyataan di dalam for.
    dispatch(node->body);

    // Menginkrementasi atau mendekrementasi variabel iterator.
    instructions.push_back(Instruction(OpCode::LOD, levelDiff, iterAddr));
    instructions.push_back(Instruction(OpCode::LIT, 0, 1));
    if (node->isDownto) {
        instructions.push_back(Instruction(OpCode::OPR, 0, (int)OprCode::SUB));
    } else {
        instructions.push_back(Instruction(OpCode::OPR, 0, (int)OprCode::ADD));
    }
    instructions.push_back(Instruction(OpCode::STO, levelDiff, iterAddr));

    // Kembali mengeksekusi pemeriksaan kondisi.
    instructions.push_back(Instruction(OpCode::JMP, 0, condStart));

    // Menyelesaikan proses backpatching titik keluar.
    int endPos = instructions.size();
    instructions[jpcIndex].a = endPos;
}
