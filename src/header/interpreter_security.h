#ifndef INTERPRETER_SECURITY_H
#define INTERPRETER_SECURITY_H

#include <stdexcept>
#include <string>

// Sekuritas dan penanganan kerentanan sistem saat interpreter berjalan.

namespace Security {
    
    // Batas default arsitektur
    const int MAX_STACK_SIZE = 1000;

    // Deteksi Stack Overflow (Infinite Recursion)
    // Panggil sebelum melakukan instruksi 'CAL' atau 'push()'
    void checkStackOverflow(int current_size, int additional_size = 1);

    // Deteksi Stack Underflow
    // Panggil sebelum melakukan intruksi 'pop()'
    void checkStackUnderflow(int current_size, int required_items = 1);

    // Deteksi Out-of-Bounds Variable/Array Access
    // Panggil saat instruksi 'LOD' atau 'STO' untuk memastikan alamat valid
    void checkOutOfBounds(int index, int max_memory_limit);

    // Deteksi Invalid Jump Targets
    // Panggil sebelum merubah IP (Instruction Pointer) pada 'JMP' atau 'JPC'
    void validateJumpTarget(int target_ip, int max_instructions);

    // Deteksi Numerical Overflow/Underflow pada operasi Aritmatika
    // Panggil saat melakukan OPR ADD, SUB, MUL untuk memastikan batas integer 32-bit
    void checkNumericalOverflow(long long calculation_result);

    // Deteksi Division by Zero
    // Panggil saat melakukan OPR DIV atau MOD
    void checkDivisionByZero(int divisor);

    // Helper untuk melempar format pesan error yang rapi
    void throwRuntimeError(const std::string& errorType, const std::string& message);
}

#endif // INTERPRETER_SECURITY_H