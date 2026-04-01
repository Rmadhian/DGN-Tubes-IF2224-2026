#include "DFA_Dinamis.hpp"
#include <cctype>
#include <cstdio>

DynamicToken DFADinamis::process(FILE* fp, char c) {
    std::string buffer = "";
    buffer += c;

    // 1. Handling Identifiers (ident)
    // Aturan: Diawali huruf, diikuti huruf/angka, case-insensitive 
    if (isalpha(c)) {
        char next;
        while ((next = fgetc(fp)) != EOF && (isalnum(next))) {
            buffer += next;
        }
        ungetc(next, fp); // Kembalikan karakter non-alnum ke stream 
        return {DynamicTokenType::IDENT, buffer};
    }

    // 2. Handling Numbers (intcon & realcon)
    // Aturan: intcon adalah digit, realcon memiliki titik desimal 
    if (isdigit(c)) {
        char next;
        bool isReal = false;
        while ((next = fgetc(fp)) != EOF && (isdigit(next) || next == '.')) {
            if (next == '.') {
                if (isReal) break; // Titik kedua mengakhiri token
                isReal = true;
            }
            buffer += next;
        }
        ungetc(next, fp);
        return {isReal ? DynamicTokenType::REALCON : DynamicTokenType::INTCON, buffer};
    }

    // 3. Handling Charcon & String
    // Aturan: Diapit petik tunggal 
    if (c == '\'') {
        char next;
        while ((next = fgetc(fp)) != EOF && next != '\'') {
            buffer += next;
        }
        buffer += '\'';
        // Jika hanya 1 karakter di dalam petik (misal 'a'), itu charcon 
        if (buffer.length() == 3) return {DynamicTokenType::CHARCON, buffer};
        return {DynamicTokenType::STRING, buffer};
    }

    // 4. Handling Comments { } atau (* *) 
    if (c == '{') {
        char next;
        while ((next = fgetc(fp)) != EOF && next != '}') {
            buffer += next;
        }
        buffer += '}';
        return {DynamicTokenType::COMMENT, buffer};
    }
    
    if (c == '(') {
        char next = fgetc(fp);
        if (next == '*') {
            buffer += next;
            while (true) {
                char d = fgetc(fp);
                if (d == EOF) break;
                buffer += d;
                if (d == '*') {
                    char e = fgetc(fp);
                    if (e == ')') {
                        buffer += e;
                        break;
                    }
                    ungetc(e, fp);
                }
            }
            return {DynamicTokenType::COMMENT, buffer};
        } else {
            ungetc(next, fp); // Bukan komentar, biarkan Nelson (DFA Statis) menangani lparent 
        }
    }

    // 5. Handling Whitespace 
    if (isspace(c)) {
        return {DynamicTokenType::WHITESPACE, " "};
    }

    return {DynamicTokenType::UNKNOWN, buffer};
}