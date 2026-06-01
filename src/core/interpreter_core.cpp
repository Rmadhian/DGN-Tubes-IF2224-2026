#include "interpreter_core.h"
#include "interpreter_security.h" // Butuh ini untuk manggil fitur keamanan nelson
#include <iostream>

// Mengimplementasikan arsitektur dasar VirtualMachine

VirtualMachine::VirtualMachine(const std::vector<Instruction>& instructions) {
    this->code = instructions;
    this->IP = 0;
    this->BP = 0;
    this->SP = -1;
    this->stack.resize(MAX_STACK_SIZE, 0); // Pre-alokasikan memori kosong
}

void VirtualMachine::run() {
    // TODO: Bikin loop eksekusi utama (Fetch - Decode - Execute)
    // Loop eksekusi utama (Fetch - Decode - Execute)
    while (IP < (int)code.size()) {
        // [FETCH] Ambil instruksi yang ditunjuk Instruction Pointer
        Instruction instr = code[IP];
        
        // Segera majukan IP agar Return Address (RA) mengarah ke baris yang tepat
        IP++; 

        // Fungsi lambda internal untuk mencari Base Pointer dari Lexical Level Absolut
        auto getBase = [&](int level) {
            if (level == 0) {
                return 0; // Level 0 selalu merujuk ke blok variabel global yang Base Pointer-nya dimulai dari 0
            } else {
                return BP; // Level > 0 (Lokal) merujuk ke Base Pointer fungsi yang sedang aktif saat ini
            }
        };

        // [DECODE & EXECUTE]
        switch (instr.op) {
            case OpCode::LIT:
                push(instr.a);
                break;
                
            case OpCode::LOD:
                push(getMemory(getBase(instr.l) + instr.a));
                break;
                
            case OpCode::STO:
                setMemory(getBase(instr.l) + instr.a, pop());
                break;
                
            case OpCode::CAL:
                // Menyiapkan Stack Frame baru saat prosedur/fungsi dipanggil
                stack[SP + 1] = getBase(instr.l); // Static Link (SL)
                stack[SP + 2] = BP;               // Dynamic Link (DL)
                stack[SP + 3] = IP;               // Return Address (RA)
                
                BP = SP + 1;                      // Majukan Base Pointer ke awal frame baru
                IP = instr.a;                     // Jump ke alamat awal fungsi
                break;
                
            case OpCode::INT:
                SP = SP + instr.a;                // Ubah top-of-stack sesuai memori yang diminta
                Security::checkStackOverflow(SP + 1, 0);
                break;
                
            case OpCode::JMP:
                jump(instr.a); // Akan merubah IP (diimplementasi Nelson)
                break;
                
            case OpCode::JPC:
                conditionalJump(instr.a, pop() == 0); // (IF_FALSE) Jump jika top stack = 0
                break;
                
            case OpCode::OPR:
                executeOPR(instr.a); // Eksekusi MTK & Output (diimplementasi Rama)
                break;
                
            case OpCode::RET:
                SP = BP - 1;         // Musnahkan Stack Frame (hapus memori lokal)
                IP = stack[BP + 2];  // Kembalikan alur program ke Return Address
                BP = stack[BP + 1];  // Kembalikan Base Pointer ke fungsi pemanggil (DL)
                break;
                
            default:
                Security::throwRuntimeError("VM_ENGINE_ERROR", "OpCode Instruksi tidak dikenali");
                break;
        }
    }
}

void VirtualMachine::push(int value) {
    // TODO: Tambah nilai ke stack. Jangan lupa panggil Security::checkStackOverflow()
    Security::checkStackOverflow(SP + 1, 1);
    SP++;
    
    // Safety net: expand capacity jika diperlukan (meskipun sudah dipre-alokasi)
    if (SP >= stack.size()) stack.resize(SP + 100, 0);
    
    stack[SP] = value;
}

int VirtualMachine::pop() {
    // TODO: Ambil nilai dari stack. Jangan lupa panggil Security::checkStackUnderflow()
    Security::checkStackUnderflow(SP + 1, 1);
    int val = stack[SP];
    SP--;
    return val;
}

int VirtualMachine::getMemory(int address) {
    // TODO: Ambil nilai dari memory address tertentu
    Security::checkOutOfBounds(address, stack.size());
    return stack[address];
}

void VirtualMachine::setMemory(int address, int value) {
    // TODO: Simpan nilai ke memory address tertentu
    Security::checkOutOfBounds(address, stack.size());
    stack[address] = value;
}