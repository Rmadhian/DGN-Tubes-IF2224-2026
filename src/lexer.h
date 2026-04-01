#ifndef LEXER_H
#define LEXER_H

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>

using namespace std;

enum class TokenType {
    // Tipe Data & Identifier
    INTCON, REALCON, CHARCON, STRING, IDENT,
    
    // Operator Logika & Aritmatika
    NOTSY, PLUS, MINUS, TIMES, IDIV, RDIV, IMOD, ANDSY, ORSY,
    
    // Operator Relasional & Simbol
    EQL, NEQ, GTR, GEQ, LSS, LEQ, 
    LPARENT, RPARENT, LBRACK, RBRACK, 
    COMMA, SEMICOLON, PERIOD, COLON, BECOMES,
    
    // Keywords
    CONSTSY, TYPESY, VARSY, FUNCTIONSY, PROCEDURESY, ARRAYSY, RECORDSY, PROGRAMSY,
    BEGINSY, IFSY, CASESY, REPEATSY, WHILESY, FORSY, ENDSY, ELSESY, 
    UNTILSY, OFSY, DOSY, TOSY, DOWNTOSY, THENSY,
    
    // Special
    COMMENT, UNKNOWN, END_OF_FILE
};

// Struktur data untuk menyimpan hasil token yang ditemukan
struct Token {
    TokenType type;
    string value;
    
    Token(TokenType t, string v) : type(t), value(v) {}
};

class Lexer {
private:
    string source_code;
    size_t current_pos;
    char current_char;

    // Fungsi untuk maju 1 karakter di source code
    void advance();
    
    // Fungsi untuk ngintip 1 karakter di depan tanpa memajukan posisi
    char peek();

    // --- DFA DINAMIS ---
    // Mengabaikan spasi, enter, dan tab
    void skipWhitespace();

    // Mendeteksi dan menghasilkan token komentar { } atau (* *)
    Token scanComment();

    // Mendeteksi angka (bisa intcon atau realcon)
    Token scanNumber();

    // Mendeteksi identifier, lalu dicek apakah dia keyword atau bukan
    Token scanIdentOrKeyword();
    
    // Mendeteksi string atau charcon yang diapit tanda petik tunggal
    Token scanStringOrChar();

    // --- DFA STATIS ---
    // Mendeteksi operator matematika, relasional, dan tanda baca
    Token scanSymbol();

    // Hash map untuk mencocokkan string dengan TokenType keyword secara instan (Case-insensitive)
    TokenType checkKeyword(const string& ident);

public:
    // Constructor menerima isi mentah dari file .txt
    Lexer(string source);
    
    // Fungsi utama yang akan dipanggil di main.cpp untuk menghasilkan seluruh token
    vector<Token> tokenize();
    
    // Fungsi utilitas untuk mengubah Enum menjadi String sesuai format Output TXT
    string tokenTypeToString(TokenType type);
};

#endif