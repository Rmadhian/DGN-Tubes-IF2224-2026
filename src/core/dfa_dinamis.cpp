#include "lexer.h"
#include <cctype>
#include <algorithm>

// Mengabaikan karakter whitespace dan blok komentar.
void Lexer::skipWhitespaceAndComments() {
    while (current_char != '\0') {
        if (isspace(current_char)) {
            advance();
        } 
        // Menangani komentar dengan format kurung kurawal.
        else if (current_char == '{') {
            advance();
            while (current_char != '\0' && current_char != '}') {
                advance();
            }
            if (current_char == '}') advance();
        } 
        // Menangani komentar dengan format kurung buka dan bintang.
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

// Memindai tipe data numerik integer atau real.
Token Lexer::scanNumber() {
    string res = "";

    // Mengurai bagian bilangan bulat.
    while (isdigit(current_char)) {
        res += current_char;
        advance();
    }

    // Mengurai bagian pecahan jika ditemukan titik desimal.
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

// Memindai identifier dan memeriksa kecocokannya dengan keyword bahasa ini.
Token Lexer::scanIdentOrKeyword() {
    string res = "";
    res += current_char;
    advance();

    // Mengekstraksi karakter alfanumerik secara berurutan.
    while (current_char != '\0' && isalnum(current_char)) {
        res += current_char;
        advance();
    }

    // Mengubah string ke huruf kecil karena bahasa ini bersifat case-insensitive.
    string lookup = res;
    transform(lookup.begin(), lookup.end(), lookup.begin(), ::tolower);

    TokenType type = checkKeyword(lookup);
    return Token(type, res);
}

// Memindai literal string atau karakter konstan yang diapit tanda kutip tunggal.
Token Lexer::scanStringOrChar() {
    string teks;
    teks += current_char;
    advance();

    while (current_char != '\0') {
      if (current_char == '\'') {
        // Menangani karakter escape berupa kutip tunggal ganda.
        if (peek() == '\'') {
          teks += current_char;
          advance();
          teks += current_char;
          advance();
        } else {
          // Mengakhiri pemindaian saat kutip penutup ditemukan.
          teks += current_char;
          advance();
          break;
        }
      } else {
        teks += current_char;
        advance();
      }
    } 

    // Menentukan tipe token berdasarkan panjang string yang dipindai.
    if (teks.length() == 3) {
      return Token(TokenType::CHARCON, teks);
    } else {
      return Token(TokenType::STRING, teks);
    }
}