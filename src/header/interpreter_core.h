#ifndef INTERPRETER_CORE_H
#define INTERPRETER_CORE_H

#include <vector>
#include <string>
#include <stdexcept>
#include "tac_instruction.h"

class VirtualMachine {
private:
    // ============================================================================
    // DONGUN (Core Engine & Struktur Stack)
    // ============================================================================
    
    std::vector<int> stack;             // Memori utama / The Stack
    std::vector<Instruction> code;      // Kumpulan instruksi TAC yang akan dieksekusi
    
    int IP; // Instruction Pointer (menunjuk baris instruksi saat ini)
    int BP; // Base Pointer / Dynamic Link (menunjuk awal frame fungsi saat ini)
    int SP; // Stack Pointer (menunjuk elemen teratas dari stack)

    // Kapasitas maksimum memori untuk deteksi Stack Overflow
    const int MAX_STACK_SIZE = 1000;

public:
    // Constructor
    VirtualMachine(const std::vector<Instruction>& instrs);

    // Daur Utama Eksekusi (Fetch-Decode-Execute)
    void run();
    
    // Utilitas Manipulasi Memori Dasar
    void push(int value);
    int pop();
    int getMemory(int address);
    void setMemory(int address, int value);

    // ============================================================================
    // RAMA (Eksekusi Operasi Matematika & Logika)
    // ============================================================================
    
    // Menjalankan operasi OPR spesifik
    void executeOPR(int oprCode);


    // ============================================================================
    // NELSON (Control Flow & Penanganan Keamanan)
    // ============================================================================
    
    // Fungsi Manipulasi Alur Kontrol
    void jump(int address);
    void conditionalJump(int address, bool condition); // IF_FALSE jump

    // Fungsi Validasi & Sekuritas (Vulnerability Handling)
    void checkStackOverflow(int additionalSize = 1);
    void checkStackUnderflow(int requiredItems = 1);
    void checkOutOfBounds(int address);
    void checkInvalidJump(int targetAddress);
    void checkDivisionByZero(int divisor);
};

#endif // INTERPRETER_CORE_H