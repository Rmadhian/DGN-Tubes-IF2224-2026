#include "interpreter_core.h"
#include "interpreter_security.h"
#include <iostream>

// Eksekusi operasi matematika dan logika di atas Stack (Bagian Rama)

void VirtualMachine::executeOPR(int oprCode) {
    switch (oprCode) {
        case 1: { // NEG: negasi nilai teratas stack
            Security::checkStackUnderflow(SP + 1, 1);
            int val = pop();
            push(-val);
            break;
        }
        case 2: { // ADD: penjumlahan
            Security::checkStackUnderflow(SP + 1, 2);
            int b = pop();
            int a = pop();
            long long result = (long long)a + b;
            Security::checkNumericalOverflow(result);
            push((int)result);
            break;
        }
        case 3: { // SUB: pengurangan
            Security::checkStackUnderflow(SP + 1, 2);
            int b = pop();
            int a = pop();
            long long result = (long long)a - b;
            Security::checkNumericalOverflow(result);
            push((int)result);
            break;
        }
        case 4: { // MUL: perkalian
            Security::checkStackUnderflow(SP + 1, 2);
            int b = pop();
            int a = pop();
            long long result = (long long)a * b;
            Security::checkNumericalOverflow(result);
            push((int)result);
            break;
        }
        case 5: { // DIV: pembagian integer
            Security::checkStackUnderflow(SP + 1, 2);
            int b = pop();
            Security::checkDivisionByZero(b);
            int a = pop();
            push(a / b);
            break;
        }
        case 6: { // MOD: modulus
            Security::checkStackUnderflow(SP + 1, 2);
            int b = pop();
            Security::checkDivisionByZero(b);
            int a = pop();
            push(a % b);
            break;
        }
        case 7: { // EQL: a == b
            Security::checkStackUnderflow(SP + 1, 2);
            int b = pop();
            int a = pop();
            push(a == b ? 1 : 0);
            break;
        }
        case 8: { // NEQ: a != b
            Security::checkStackUnderflow(SP + 1, 2);
            int b = pop();
            int a = pop();
            push(a != b ? 1 : 0);
            break;
        }
        case 9: { // LSS: a < b
            Security::checkStackUnderflow(SP + 1, 2);
            int b = pop();
            int a = pop();
            push(a < b ? 1 : 0);
            break;
        }
        case 10: { // GEQ: a >= b
            Security::checkStackUnderflow(SP + 1, 2);
            int b = pop();
            int a = pop();
            push(a >= b ? 1 : 0);
            break;
        }
        case 11: { // GTR: a > b
            Security::checkStackUnderflow(SP + 1, 2);
            int b = pop();
            int a = pop();
            push(a > b ? 1 : 0);
            break;
        }
        case 12: { // LEQ: a <= b
            Security::checkStackUnderflow(SP + 1, 2);
            int b = pop();
            int a = pop();
            push(a <= b ? 1 : 0);
            break;
        }
        case 13: { // WRT: cetak tanpa newline
            Security::checkStackUnderflow(SP + 1, 1);
            int val = pop();
            std::cout << val;
            break;
        }
        case 14: { // WRTLN: cetak dengan newline
            Security::checkStackUnderflow(SP + 1, 1);
            int val = pop();
            std::cout << val << std::endl;
            break;
        }
        default:
            std::cerr << "Runtime Error: OPR code " << oprCode << " tidak dikenal" << std::endl;
            break;
    }
}