#include "interpreter_core.h"
#include "interpreter_security.h"

// JMP: lompat tanpa syarat ke alamat tujuan
void VirtualMachine::jump(int address) {
    checkInvalidJump(address);
    IP = address;
}

// JPC: lompat ke alamat tujuan HANYA kalau kondisi bernilai FALSE,
// selain itu lanjut ke instruksi berikutnya.
void VirtualMachine::conditionalJump(int address, bool condition) {
    checkInvalidJump(address);
    if (!condition) {
        IP = address;
    } else {
        IP = IP + 1;
    }
}

// ----------------------------------------------------------------------------
// Bridge ke namespace Security memakai state VM saat ini.
// Dongun/Rama bisa memanggil method ini, atau langsung Security::... .
// ----------------------------------------------------------------------------

void VirtualMachine::checkStackOverflow(int additionalSize) {
    Security::checkStackOverflow(SP + 1, additionalSize);
}

void VirtualMachine::checkStackUnderflow(int requiredItems) {
    Security::checkStackUnderflow(SP + 1, requiredItems);
}

void VirtualMachine::checkOutOfBounds(int address) {
    // Batas memori = ukuran stack yang sudah dialokasikan
    Security::checkOutOfBounds(address, (int)stack.size());
}

void VirtualMachine::checkInvalidJump(int targetAddress) {
    Security::validateJumpTarget(targetAddress, (int)code.size());
}

void VirtualMachine::checkDivisionByZero(int divisor) {
    Security::checkDivisionByZero(divisor);
}
