#include "lexer.h"
#include <cctype>
#include <algorithm>

// Melewati whitespace (spasi, tab, newline) dan komentar Pascal
void Lexer::skipWhitespaceAndComments() {
    while (current_char != '\0') {
        if (isspace(current_char)) {
            advance();
        } 
        // Komentar gaya { ... }
        else if (current_char == '{') {
            advance();
            while (current_char != '\0' && current_char != '}') {
                advance();
            }
            if (current_char == '}') advance();
        } 
        // Komentar gaya (* ... *)
        else if (current_char == '(' && peek() == '*') {
            advance(); // skip '('
            advance(); // skip '*'
            while (current_char != '\0') {
                if (current_char == '*' && peek() == ')') {
                    advance(); // skip '*'
                    advance(); // skip ')'
                    break;
                }
                advance();
            }
        } else {
            break;
        }
    }
}

// Memindai bilangan: integer atau real (jika ada titik desimal)
Token Lexer::scanNumber() {
    string res = "";

    // Baca digit integer dulu
    while (isdigit(current_char)) {
        res += current_char;
        advance();
    }

    // Kalau ketemu titik dan diikuti digit, berarti bilangan real
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

// Memindai identifier lalu cek apakah termasuk keyword Pascal
Token Lexer::scanIdentOrKeyword() {
    string res = "";
    res += current_char;
    advance();

    // Terus baca selama masih alfanumerik
    while (current_char != '\0' && isalnum(current_char)) {
        res += current_char;
        advance();
    }

    // Lowercase untuk pencocokan keyword (Pascal case-insensitive)
    string lookup = res;
    transform(lookup.begin(), lookup.end(), lookup.begin(), ::tolower);

    TokenType type = checkKeyword(lookup);
    return Token(type, res);
}

// Memindai string literal atau char constant yang diapit petik tunggal
Token Lexer::scanStringOrChar() {
    string teks;
    teks += current_char; // Simpan petik pembuka
    advance();

    while (current_char != '\0') {
      if (current_char == '\'') {
        // Petik ganda ('') artinya escape, bukan penutup
        if (peek() == '\'') {
          teks += current_char;
          advance();
          teks += current_char;
          advance();
        } else {
          // Petik penutup
          teks += current_char;
          advance();
          break;
        }
      } else {
        teks += current_char;
        advance();
      }
    } 

    // Panjang 3 (misal 'A') berarti char, selain itu string
    if (teks.length() == 3) {
      return Token(TokenType::CHARCON, teks);
    } else {
      return Token(TokenType::STRING, teks);
    }
}