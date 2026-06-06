#ifndef TAC_INSTRUCTION_H
#define TAC_INSTRUCTION_H

#include <string>

// ============================================================================
// Semua akan menggunakan enum dan struct ini untuk ICG dan Interpreter.
// ============================================================================

// Daftar Instruksi Utama berdasarkan Spesifikasi Milestone 4 
enum class OpCode {
    LIT, // Load Literal: Memasukkan nilai literal ke dalam stack 
    LOD, // Load Value: Memuat nilai dari address 
    STO, // Store Value: Menyimpan nilai ke address 
    CAL, // Call: Memanggil fungsi di garis l 
    INT, // Initiate Memory: Membuat memory dengan ukuran m 
    JMP, // Unconditional Jump: Lompat ke garis l tanpa kondisi apapun 
    JPC, // Conditional Jump: Lompat ke garis l bila kondisi tidak memenuhi (IF_FALSE) 
    OPR, // Operation: Memanggil operasi o 
    RET  // Return: Keluar dari fungsi atau prosedur 
};

// Daftar Operasi untuk instruksi OPR berdasarkan Spesifikasi Milestone 4 
enum class OprCode {
    NEG = 1,   // Negasi value dari stack teratas 
    ADD = 2,   // Menambah value stack teratas dengan sebelahnya 
    SUB = 3,   // Mengurangi value stack teratas dengan sebelahnya 
    MUL = 4,   // Mengali value stack teratas dengan sebelahnya 
    DIV = 5,   // Membagi value stack teratas dengan sebelahnya 
    MOD = 6,   // Modulus value stack teratas dengan sebelahnya 
    EQL = 7,   // Membandingkan kesamaan (==) [cite: 404]
    NEQ = 8,   // Membandingkan ketidaksamaan (!=) 
    LSS = 9,   // Kurang dari (<) 
    GEQ = 10,  // Lebih dari sama dengan (>=) 
    GTR = 11,  // Lebih dari (>) 
    LEQ = 12,  // Kurang dari sama dengan (<=) 
    WRT = 13,  // Menulis output tanpa newline 
    WRTLN = 14, // Menulis output dengan newline 
    READ = 15,  // Membaca input tanpa newline
    READLN = 16 // Membaca input dengan newline
};

// Struktur representasi satu baris instruksi TAC
struct Instruction {
    OpCode op;   // Kode Instruksi (LIT, LOD, dll)
    int l;       // Lexical Level / Static Link (biasanya 0 untuk Arion tanpa nested function)
    int a;       // Alamat memori, identifier literal, atau kode operasi (OprCode)
    int numArgs; // Jumlah argumen (khusus untuk instruksi CAL)
    
    // Constructor untuk kemudahan inisialisasi
    Instruction(OpCode op, int l, int a, int numArgs = 0) : op(op), l(l), a(a), numArgs(numArgs) {}
};

#endif // TAC_INSTRUCTION_H