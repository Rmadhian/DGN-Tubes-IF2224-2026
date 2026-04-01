#include "DFA_Dinamis.hpp"
#include <cctype>
#include <cstdio>

DynamicToken DFADinamis::process(FILE* fp, char c) {
    std::string buffer = "";
    buffer += c;

    if (isalpha(c)) {
        char next;
        while ((next = fgetc(fp)) != EOF && (isalnum(next))) {
            buffer += next;
        }
        ungetc(next, fp);  
        return {DynamicTokenType::IDENT, buffer};
    }

    if (isdigit(c)) {
        char next;
        bool isReal = false;
        while ((next = fgetc(fp)) != EOF && (isdigit(next) || next == '.')) {
            if (next == '.') {
                if (isReal) break; 
                isReal = true;
            }
            buffer += next;
        }
        ungetc(next, fp);
        return {isReal ? DynamicTokenType::REALCON : DynamicTokenType::INTCON, buffer};
    }

    if (c == '\'') {
        char next;
        while ((next = fgetc(fp)) != EOF && next != '\'') {
            buffer += next;
        }
        buffer += '\'';
        if (buffer.length() == 3) return {DynamicTokenType::CHARCON, buffer};
        return {DynamicTokenType::STRING, buffer};
    }

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
            ungetc(next, fp); 
        }
    }

    if (isspace(c)) {
        return {DynamicTokenType::WHITESPACE, " "};
    }

    return {DynamicTokenType::UNKNOWN, buffer};
}