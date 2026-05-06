#include "lexer.h"
#include <cctype>

// Inisialisasi lexer, posisi mulai dari awal source code
Lexer::Lexer(string source) {
  source_code = source;
  current_pos=0;

  if (source_code.length() > 0) {
    current_char = source_code[0];
  } else {
    current_char = '\0'; // Source kosong, langsung set null
  }
}

// Maju satu karakter, set null kalau sudah di akhir
void Lexer::advance() {
  current_pos++;

  if (current_pos < source_code.length()) {
    current_char=source_code[current_pos];
  } else {
    current_char = '\0';
  }
}

// Lihat karakter selanjutnya tanpa memajukan posisi (lookahead 1)
char Lexer::peek() {
  size_t peek_pos = current_pos + 1;

  if (peek_pos < source_code.length()) {
    return source_code[peek_pos];
  } else {
    return '\0';
  }
}

// Fungsi utama tokenisasi: baca seluruh source code dan hasilkan list token
vector<Token> Lexer::tokenize() {
  vector<Token> hasil_token;
  while (current_char != '\0') {
    // Skip whitespace dan komentar dulu sebelum proses
    if (isspace(current_char) || current_char == '{' || (current_char == '(' && peek() == '*')) {
      skipWhitespaceAndComments();
      continue;
    }

    // Karakter huruf -> bisa identifier atau keyword
    if(isalpha(current_char)) {
      hasil_token.push_back(scanIdentOrKeyword());
      continue;
    }

    // Karakter angka -> integer atau real constant
    if (isdigit(current_char)) {
      hasil_token.push_back(scanNumber());
      continue;
    }

    // Karakter petik -> string atau char constant
    if (current_char == '\'') {
      hasil_token.push_back(scanStringOrChar());
      continue;
    }

    // Selain itu masuk ke pemindaian simbol/operator
    hasil_token.push_back(scanSymbol());
  }
  return hasil_token;
}

// Konversi enum TokenType ke string untuk output
string Lexer::tokenTypeToString(TokenType type) {
  switch (type) {
    case TokenType::INTCON: return "intcon";
    case TokenType::REALCON: return "realcon";
    case TokenType::CHARCON: return "charcon";
    case TokenType::STRING: return "string";
    case TokenType::IDENT: return "ident";
    case TokenType::NOTSY: return "notsy";
    case TokenType::PLUS: return "plus";
    case TokenType::MINUS: return "minus";
    case TokenType::TIMES: return "times";
    case TokenType::IDIV: return "idiv";
    case TokenType::RDIV: return "rdiv";
    case TokenType::IMOD: return "imod";
    case TokenType::ANDSY: return "andsy";
    case TokenType::ORSY: return "orsy";
    case TokenType::EQL: return "eql";
    case TokenType::NEQ: return "neq";
    case TokenType::GTR: return "gtr";
    case TokenType::GEQ: return "geq";
    case TokenType::LSS: return "lss";
    case TokenType::LEQ: return "leq";
    case TokenType::LPARENT: return "lparent";
    case TokenType::RPARENT: return "rparent";
    case TokenType::LBRACK: return "lbrack";
    case TokenType::RBRACK: return "rbrack";
    case TokenType::COMMA: return "comma";
    case TokenType::SEMICOLON: return "semicolon";
    case TokenType::PERIOD: return "period";
    case TokenType::COLON: return "colon";
    case TokenType::BECOMES: return "becomes";
    case TokenType::CONSTSY: return "constsy";
    case TokenType::TYPESY: return "typesy";
    case TokenType::VARSY: return "varsy";
    case TokenType::FUNCTIONSY: return "functionsy";
    case TokenType::PROCEDURESY: return "proceduresy";
    case TokenType::ARRAYSY: return "arraysy";
    case TokenType::RECORDSY: return "recordsy";
    case TokenType::PROGRAMSY: return "programsy";
    case TokenType::BEGINSY: return "beginsy";
    case TokenType::IFSY: return "ifsy";
    case TokenType::CASESY: return "casesy";
    case TokenType::REPEATSY: return "repeatsy";
    case TokenType::WHILESY: return "whilesy";
    case TokenType::FORSY: return "forsy";
    case TokenType::ENDSY: return "endsy";
    case TokenType::ELSESY: return "elsesy";
    case TokenType::UNTILSY: return "untilsy";
    case TokenType::OFSY: return "ofsy";
    case TokenType::DOSY: return "dosy";
    case TokenType::TOSY: return "tosy";
    case TokenType::DOWNTOSY: return "downtosy";
    case TokenType::THENSY: return "thensy";
    case TokenType::END_OF_FILE: return "EOF";
    default: return "UNKNOWN";
  }
}