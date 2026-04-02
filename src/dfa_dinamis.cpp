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

    while (isdigit(current_char)) {
        res += current_char;
        advance();
    }

    if (current_char == '.' && isdigit(peek())) {
        res += current_char;
        advance();

        while (isdigit(current_char)) {
            res += current_char;
            advance();
        }
        return Token(TokenType::REALCON, res);
    }

    return Token(TokenType::INTCON, res);
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
    string teks;
    teks += current_char;
    advance();

    while (current_char != '\0') {
      if (current_char == '\'') {
        if (peek() == '\'') {
          teks += current_char;
          advance();
          teks +=current_char;
          advance();
        } else {
          teks += current_char;
          advance();
          break;
        }
      } else {
        teks += current_char;
        advance();
      }
    } 

    if (teks.length() == 3) {
      return Token(TokenType::CHARCON, teks);
    } else {
      return Token(TokenType::STRING, teks);
    }
}