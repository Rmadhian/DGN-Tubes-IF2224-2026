#include "interpreter_core.h"
#include "interpreter_security.h" // Security checks

// Implementasi arsitektur VirtualMachine

VirtualMachine::VirtualMachine(const std::vector<Instruction>& instructions) {
    this->code = instructions;
    this->IP = 0;
    this->BP = 0;
    this->SP = -1;
    this->stack.assign(MAX_STACK_SIZE, 0); // Alokasi stack awal
}

void VirtualMachine::run() {
    // Loop eksekusi utama (Fetch-Decode-Execute)
    while (IP < (int)code.size()) {
        // [FETCH] Ambil instruksi
        Instruction instr = code[IP];
        
        // Majukan IP (Return Address menunjuk baris berikutnya)
        IP++; 

        // Fungsi lambda internal untuk mencari Base Pointer dari Lexical Level Difference
        auto getBase = [&](int levelDiff) {
            int base = BP;
            for (int i = 0; i < levelDiff; i++) {
                base = stack[base]; // Traverse Static Link
            }
            return base;
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
                
            case OpCode::CAL: {
                int sl = getBase(instr.l); // Static Link
                int dl = BP;               // Dynamic Link
                int ra = IP;               // Return Address
                int args = instr.numArgs;  // Jumlah argumen
                
                // Sisipkan Call Frame (SL, DL, RA)
                BP = SP - args + 1; 
                SP = SP + 3;        
                IP = instr.a;       
                break;
            }
                
            case OpCode::INT: {
                // Alokasi stack memory
                Security::checkStackOverflow(SP + instr.a, MAX_STACK_SIZE);
                SP = SP + instr.a;                
                break;
            }
                
            case OpCode::JMP: {
                jump(instr.a); 
                break;
            }
                
            case OpCode::JPC: {
                conditionalJump(instr.a, pop() == 0); 
                break;
            }
                
            case OpCode::OPR: {
                executeOPR(instr.a); 
                break;
            }
                
            case OpCode::RET: {
                if (BP == 0) {
                    IP = code.size(); // Hentikan eksekusi global
                } else {
                    // Musnahkan Stack Frame dan kembali
                    SP = BP - 1;         
                    IP = stack[BP + 2];  
                    BP = stack[BP + 1];  
                }
                break;
            }
                
            default:
                Security::throwRuntimeError("VM_ENGINE_ERROR", "OpCode Instruksi tidak dikenali");
                break;
        }
    }
}

void VirtualMachine::push(int val) {
    SP++;
    Security::checkStackOverflow(SP, MAX_STACK_SIZE);
    
    // Resize array memori jika dibutuhkan
    if (SP >= (int)stack.size()) {
        stack.resize(SP + 100, 0);
    }
    stack[SP] = val;
}

// Mengambil nilai dari stack
int VirtualMachine::pop() {
    Security::checkStackUnderflow(SP, 0);
    return stack[SP--];
}

// Membaca nilai dari stack index
int VirtualMachine::getMemory(int address) {
    Security::checkOutOfBounds(address, stack.size());
    return stack[address];
}

// Menyimpan nilai ke stack index
void VirtualMachine::setMemory(int address, int val) {
    Security::checkOutOfBounds(address, stack.size());
    stack[address] = val;
}
