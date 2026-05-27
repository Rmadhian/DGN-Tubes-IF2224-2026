#include "interpreter_security.h"
#include <iostream>

// Mengimplementasikan fungsi-fungsi interceptor keamanan di dalam namespace Security

namespace Security {

    void checkStackOverflow(int current_size, int additional_size) {
        // TODO: Cek jika current_size + additional_size > MAX_STACK_SIZE, lempar error
    }

    void checkStackUnderflow(int current_size, int required_items) {
        // TODO: Cek jika current_size < required_items, lempar error
    }

    void checkOutOfBounds(int index, int max_memory_limit) {
        // TODO: Validasi batas array/memori
    }

    void validateJumpTarget(int target_ip, int max_instructions) {
        // TODO: Pastikan target lompatan tidak minus dan tidak melebihi jumlah baris instruksi
    }

    void checkNumericalOverflow(long long calculation_result) {
        // TODO: Deteksi batasan integer 32-bit (overflow atau underflow)
    }

    void checkDivisionByZero(int divisor) {
        // TODO: Lempar error jika divisor == 0
    }

    void throwRuntimeError(const std::string& errorType, const std::string& message) {
        throw std::runtime_error("[" + errorType + "] " + message);
    }

} // akhir namespace Security