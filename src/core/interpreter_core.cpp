#include "interpreter_core.h"
#include "interpreter_security.h" // Butuh ini untuk manggil fitur keamanan nelson
#include <iostream>

// Mengimplementasikan arsitektur dasar VirtualMachine

VirtualMachine::VirtualMachine(const std::vector<Instruction>& instructions) {
    this->code = instructions;
    this->IP = 0;
    this->BP = 0;
    this->SP = -1;
}

void VirtualMachine::run() {
    // TODO: Bikin loop eksekusi utama (Fetch - Decode - Execute)
}

void VirtualMachine::push(int value) {
    // TODO: Tambah nilai ke stack. Jangan lupa panggil Security::checkStackOverflow()
}

int VirtualMachine::pop() {
    // TODO: Ambil nilai dari stack. Jangan lupa panggil Security::checkStackUnderflow()
    return 0; // Return dummy sementara
}

int VirtualMachine::getMemory(int address) {
    // TODO: Ambil nilai dari memory address tertentu
    return 0;
}

void VirtualMachine::setMemory(int address, int value) {
    // TODO: Simpan nilai ke memory address tertentu
}