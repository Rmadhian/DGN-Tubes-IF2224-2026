#include "lexer.h"
#include <cctype>
#include <utility>

// Memeriksa apakah identifier adalah sebuah keyword
TokenType Lexer::checkKeyword(const string& ident) {
// Memetakan string keyword ke tipe enumerasi TokenType yang sesuai.
    static const unordered_map<string, TokenType> keyword_map = {
        {"const", TokenType::CONSTSY},
        {"type", TokenType::TYPESY},
        {"var", TokenType::VARSY},
        {"function", TokenType::FUNCTIONSY},
        {"procedure", TokenType::PROCEDURESY},
        {"array", TokenType::ARRAYSY},
        {"record", TokenType::RECORDSY},
        {"program", TokenType::PROGRAMSY},
        {"begin", TokenType::BEGINSY},
        {"if", TokenType::IFSY},
        {"case", TokenType::CASESY},
        {"repeat", TokenType::REPEATSY},
        {"while", TokenType::WHILESY},
        {"for", TokenType::FORSY},
        {"end", TokenType::ENDSY},
        {"else", TokenType::ELSESY},
        {"until", TokenType::UNTILSY},
        {"of", TokenType::OFSY},
        {"do", TokenType::DOSY},
        {"to", TokenType::TOSY},
        {"downto", TokenType::DOWNTOSY},
        {"then", TokenType::THENSY},

        // Memetakan token untuk operator logika dan aritmatika berbentuk kata.
        {"not", TokenType::NOTSY},
        {"and", TokenType::ANDSY},
        {"or", TokenType::ORSY},
        {"div", TokenType::IDIV},
        {"mod", TokenType::IMOD},
    };

    // Mengubah identifier ke huruf kecil untuk mendukung case-insensitivity.
    string lower_ident;
    lower_ident.reserve(ident.size());

    for (const char ch : ident) {
        lower_ident += static_cast<char>(
            tolower(static_cast<unsigned char>(ch)));
    }

    // Memeriksa keberadaan identifier di dalam peta keyword.
    const auto it = keyword_map.find(lower_ident);
    if (it != keyword_map.end()) {
        return it->second;
    }
    return TokenType::IDENT; // Mengembalikan token sebagai identifier biasa jika tidak cocok dengan keyword.
}

// Melakukan pemindaian untuk simbol, operator, dan tanda baca
Token Lexer::scanSymbol() {
    switch (current_char) {
        // Mengembalikan token untuk operator aritmatika.
        case '+':
            advance();
            return Token(TokenType::PLUS, "+");
        case '-':
            advance();
            return Token(TokenType::MINUS, "-");
        case '*':
            advance();
            return Token(TokenType::TIMES, "*");
        case '/':
            advance();
            return Token(TokenType::RDIV, "/");
        // Mengembalikan token untuk operator relasional dan pembandingan.
        case '=':
            advance();
            if (current_char == '=') {
                advance();
                return Token(TokenType::EQL, "==");
            }
            // Mengembalikan UNKNOWN karena bahasa ini menggunakan operator assignment :=. 
            return Token(TokenType::UNKNOWN, "=");
        case '<':
            advance();
            if (current_char == '=') {
                advance();
                return Token(TokenType::LEQ, "<=");
            }
            if (current_char == '>') {
                advance();
                return Token(TokenType::NEQ, "<>");
            }
            return Token(TokenType::LSS, "<");
        case '>':
            advance();
            if (current_char == '=') {
                advance();
                return Token(TokenType::GEQ, ">=");
            }
            return Token(TokenType::GTR, ">");
        // Mengembalikan token untuk simbol tanda baca dan pengelompokan.
        case '(':
            advance();
            return Token(TokenType::LPARENT, "(");
        case ')':
            advance();
            return Token(TokenType::RPARENT, ")");
        case '[':
            advance();
            return Token(TokenType::LBRACK, "[");
        case ']':
            advance();
            return Token(TokenType::RBRACK, "]");
        case ',':
            advance();
            return Token(TokenType::COMMA, ",");
        case ';':
            advance();
            return Token(TokenType::SEMICOLON, ";");
        case '.':
            advance();
            return Token(TokenType::PERIOD, ".");
        case ':':
            advance();
            // Mengevaluasi dan mengembalikan token untuk operator assignment :=.
            if (current_char == '=') {
                advance();
                return Token(TokenType::BECOMES, ":=");
            }
            return Token(TokenType::COLON, ":");
            
        // Mengembalikan token UNKNOWN untuk karakter yang tidak valid.
        default: {
            const string unknown_lexeme(1, current_char);
            advance();
            return Token(TokenType::UNKNOWN, unknown_lexeme);
        }
    }
}