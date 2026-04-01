#include "lexer.h"
#include <cctype>
#include <algorithm>

void Lexer::skipWhitespaceAndComments() {
    while (current_char != '\0') {
        if (isspace(current_char)) {
            advance();
        } 
        else if (current_char == '{') {
            advance();
            while (current_char != '\0' && current_char != '}') {
                advance();
            }
            if (current_char == '}') advance();
        } 
        else if (current_char == '(' && peek() == '*') {
            advance();
            advance();
            while (current_char != '\0') {
                if (current_char == '*' && peek() == ')') {
                    advance();
                    advance();
                    break;
                }
                advance();
            }
        } else {
            break;
        }
    }
}

Token Lexer::scanNumber() {
    string res = "";
    bool isReal = false;

    while (isdigit(current_char)) {
        res += current_char;
        advance();
    }

    if (current_char == '.') {
        isReal = true;
        res += current_char;
        advance();
        while (isdigit(current_char)) {
            res += current_char;
            advance();
        }
    }

    return Token(isReal ? TokenType::REALCON : TokenType::INTCON, res);
}

Token Lexer::scanIdentOrKeyword() {
    string res = "";
    res += current_char;
    advance();

    while (current_char != '\0' && isalnum(current_char)) {
        res += current_char;
        advance();
    }

    string lookup = res;
    transform(lookup.begin(), lookup.end(), lookup.begin(), ::tolower);

    TokenType type = checkKeyword(lookup);
    return Token(type, res);
}

Token Lexer::scanStringOrChar() {
    string content = "";
    advance();

    while (current_char != '\0' && current_char != '\'') {
        content += current_char;
        advance();
    }

    string fullToken = "'" + content + "'";
    if (current_char == '\'') {
        advance();
    }

    if (content.length() == 1) {
        return Token(TokenType::CHARCON, fullToken);
    }
    return Token(TokenType::STRING, fullToken);
}

