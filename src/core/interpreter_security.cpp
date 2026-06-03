#include "interpreter_security.h"
#include <iostream>
#include <string>
#include <cstdint>

namespace Security {

    // Stack Overflow: terlalu banyak push/frame sampai melebihi kapasitas memori.
    // Penyebab umum: infinite recursion. Dipanggil sebelum push()/CAL.
    void checkStackOverflow(int current_size, int additional_size) {
        if (current_size + additional_size > MAX_STACK_SIZE) {
            throwRuntimeError("StackOverflow",
                "Batas stack (" + std::to_string(MAX_STACK_SIZE) +
                ") terlampaui. Kemungkinan rekursi tak berujung.");
        }
    }

    // Stack Underflow: mencoba pop dari stack yang isinya kurang dari yang dibutuhkan.
    // Biasanya menandakan cacat logika di Intermediate Code Generator.
    void checkStackUnderflow(int current_size, int required_items) {
        if (current_size < required_items) {
            throwRuntimeError("StackUnderflow",
                "Stack hanya berisi " + std::to_string(current_size) +
                " nilai, tetapi operasi butuh " + std::to_string(required_items) + ".");
        }
    }

    // Out-of-Bounds: akses indeks memori/array di luar batas yang sah.
    // Juga membentengi Stack Smashing, karena penulisan (STO) tidak boleh
    // meluber melewati batas memori yang dialokasikan hingga menimpa
    // return address. Untuk array, [0, max) merepresentasikan indeks 0..panjang-1.
    void checkOutOfBounds(int index, int max_memory_limit) {
        if (index < 0 || index >= max_memory_limit) {
            throwRuntimeError("IndexOutOfBounds",
                "Akses ke alamat/indeks " + std::to_string(index) +
                " di luar batas valid [0, " + std::to_string(max_memory_limit) + ").");
        }
    }

    // Invalid Jump Target: target JMP/JPC harus berada di dalam daftar instruksi.
    // Mencegah Instruction Pointer tersesat ke area memori acak.
    void validateJumpTarget(int target_ip, int max_instructions) {
        if (target_ip < 0 || target_ip >= max_instructions) {
            throwRuntimeError("InvalidJumpTarget",
                "Lompatan ke baris " + std::to_string(target_ip) +
                " tidak valid (jumlah instruksi = " + std::to_string(max_instructions) + ").");
        }
    }

    // Numerical Overflow/Underflow: hasil hitung sudah dijadikan long long agar
    // bisa dibandingkan dengan batas signed integer 32-bit sebelum di-wrap.
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

    // Division by Zero: dipanggil sebelum OPR DIV / MOD.
    void checkDivisionByZero(int divisor) {
        if (divisor == 0) {
            throwRuntimeError("DivisionByZero",
                "Pembagian atau modulus dengan nol tidak diperbolehkan.");
        }
    }

    // Helper untuk melempar format pesan error yang rapi. Memakai exception
    // supaya eksekusi berhenti tapi mesin tidak crash (ditangkap di run()).
    void throwRuntimeError(const std::string& errorType, const std::string& message) {
        throw std::runtime_error("[" + errorType + "] " + message);
    }

} // akhir namespace Security
