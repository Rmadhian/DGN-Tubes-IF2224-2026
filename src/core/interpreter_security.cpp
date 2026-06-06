#include "interpreter_security.h"
#include <iostream>
#include <string>
#include <cstdint>

namespace Security {

    // Validasi batas memori Stack (Overflow).
    void checkStackOverflow(int targetSP, int maxCapacity) {
        if (targetSP >= maxCapacity) {
            throwRuntimeError("STACK_OVERFLOW", "Kapasitas stack terlampaui (" + std::to_string(targetSP) + " >= " + std::to_string(maxCapacity) + ")");
        }
    }

    // Validasi isi Stack sebelum Pop (Underflow).
    void checkStackUnderflow(int current_size, int required_items) {
        if (current_size < required_items) {
            throwRuntimeError("StackUnderflow",
                "Stack hanya berisi " + std::to_string(current_size) +
                " nilai, tetapi operasi butuh " + std::to_string(required_items) + ".");
        }
    }

    // Validasi akses indeks memori/array (Out-of-Bounds & Stack Smashing).
    void checkOutOfBounds(int index, int max_memory_limit) {
        if (index < 0 || index >= max_memory_limit) {
            throwRuntimeError("IndexOutOfBounds",
                "Akses ke alamat/indeks " + std::to_string(index) +
                " di luar batas valid [0, " + std::to_string(max_memory_limit) + ").");
        }
    }

    // Validasi target JMP/JPC ke dalam daftar instruksi.
    void validateJumpTarget(int target_ip, int max_instructions) {
        if (target_ip < 0 || target_ip > max_instructions) {
            throwRuntimeError("InvalidJumpTarget",
                "Lompatan ke baris " + std::to_string(target_ip) +
                " tidak valid (jumlah instruksi = " + std::to_string(max_instructions) + ").");
        }
    }

    // Deteksi Numerical Overflow/Underflow pada Signed Integer 32-bit.
    void checkNumericalOverflow(long long calculation_result) {
        if (calculation_result > INT32_MAX) {
            throwRuntimeError("OverflowError",
                "Hasil " + std::to_string(calculation_result) +
                " melampaui batas atas integer 32-bit (" + std::to_string(INT32_MAX) + ").");
        }
        if (calculation_result < INT32_MIN) {
            throwRuntimeError("UnderflowError",
                "Hasil " + std::to_string(calculation_result) +
                " di bawah batas bawah integer 32-bit (" + std::to_string(INT32_MIN) + ").");
        }
    }

    // Validasi pencegahan Division by Zero.
    void checkDivisionByZero(int divisor) {
        if (divisor == 0) {
            throwRuntimeError("DivisionByZero",
                "Pembagian atau modulus dengan nol tidak diperbolehkan.");
        }
    }

    // Helper pelempar pesan error runtime seragam.
    void throwRuntimeError(const std::string& errorType, const std::string& message) {
        throw std::runtime_error("[" + errorType + "] " + message);
    }

} 
