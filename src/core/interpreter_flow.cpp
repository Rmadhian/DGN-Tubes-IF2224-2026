#include "interpreter_core.h"
#include "interpreter_security.h"

// Mengimplementasikan perpindahan Instruction Pointer (IP) milik VirtualMachine

void VirtualMachine::jump(int address) {
    // TODO: Ubah nilai IP ke address target. Panggil Security::validateJumpTarget()
}

void VirtualMachine::conditionalJump(int address, bool condition) {
    // TODO: Ubah nilai IP ke address target HANYA JIKA condition bernilai FALSE
}