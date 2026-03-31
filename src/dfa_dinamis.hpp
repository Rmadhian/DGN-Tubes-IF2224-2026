#ifndef DFA_DINAMIS_HPP
#define DFA_DINAMIS_HPP

#include <string>
#include <vector>

enum class DynamicTokenType {
    IDENT,
    INTCON,
    REALCON,
    CHARCON,
    STRING,
    COMMENT,
    WHITESPACE,
    UNKNOWN
};

struct DynamicToken {
    DynamicTokenType type;
    std::string value;
};

class DFADinamis {
public:
    // Fungsi utama untuk memproses satu unit token dinamis atau komentar
    DynamicToken process(FILE* fp, char firstChar);
};

#endif