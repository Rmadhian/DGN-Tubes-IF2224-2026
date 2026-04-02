#include "lexer.h"
#include <cctype>
#include <utility>

TokenType Lexer::checkKeyword(const string& ident) {
    static const unordered_map<string, TokenType> keyword_map = {
        // Keywords
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

        // Operator kata
        {"not", TokenType::NOTSY},
        {"and", TokenType::ANDSY},
        {"or", TokenType::ORSY},
        {"div", TokenType::IDIV},
        {"mod", TokenType::IMOD},
    };

    string lower_ident;
    lower_ident.reserve(ident.size());

    for (const char ch : ident) {
        lower_ident += static_cast<char>(
            tolower(static_cast<unsigned char>(ch)));
    }

    const auto it = keyword_map.find(lower_ident);
    if (it != keyword_map.end()) {
        return it->second;
    }
    return TokenType::IDENT;
}

Token Lexer::scanSymbol() {
    switch (current_char) {
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
        case '=':
            advance();
            if (current_char == '=') {
                advance();
                return Token(TokenType::EQL, "==");
            }
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
            if (current_char == '=') {
                advance();
                return Token(TokenType::BECOMES, ":=");
            }
            return Token(TokenType::COLON, ":");
        default: {
            const string unknown_lexeme(1, current_char);
            advance();
            return Token(TokenType::UNKNOWN, unknown_lexeme);
        }
    }
}