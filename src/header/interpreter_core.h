#ifndef INTERPRETER_CORE_H
#define INTERPRETER_CORE_H

#include <vector>
#include <string>
#include <stdexcept>
#include "tac_instruction.h"

class VirtualMachine {
private:
    // Core engine dan struktur stack memori utama.
    
    std::vector<int> stack;             // Memori utama / The Stack
    std::vector<Instruction> code;      // Kumpulan instruksi TAC yang akan dieksekusi
    
    int IP; // Instruction Pointer (menunjuk baris instruksi saat ini)
    int BP; // Base Pointer / Dynamic Link (menunjuk awal frame fungsi saat ini)
    int SP; // Stack Pointer (menunjuk elemen teratas dari stack)

    // Kapasitas maksimum memori untuk deteksi Stack Overflow
    const int MAX_STACK_SIZE = 1000;

public:
    // String pool untuk mencetak literal string via WRT
    std::vector<std::string> stringPool;

    // Constructor
    VirtualMachine(const std::vector<Instruction>& instrs, const std::vector<std::string>& sp);

    // Daur Utama Eksekusi (Fetch-Decode-Execute)
    void run();
    
    // Utilitas Manipulasi Memori Dasar
    void push(int value);
    int pop();
    int getMemory(int address);
    void setMemory(int address, int value);

    // Mengeksekusi instruksi matematika dan operasi logika.
    
    // Menjalankan operasi OPR (l=0: integer, l=1: string)
    void executeOPR(int oprCode, int lField);


    // Mengatur aliran kontrol eksekusi dan memvalidasi keamanan operasi.
    
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