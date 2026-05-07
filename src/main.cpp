#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include "lexer.h"
#include "parser.h"

using namespace std;

// Fungsi bantuan untuk mengubah string label token menjadi Enum TokenType
TokenType stringToType(string s) {
    if (s == "programsy") return TokenType::PROGRAMSY;
    if (s == "constsy") return TokenType::CONSTSY;
    if (s == "typesy") return TokenType::TYPESY;
    if (s == "varsy") return TokenType::VARSY;
    if (s == "arraysy") return TokenType::ARRAYSY;
    if (s == "ofsy") return TokenType::OFSY;
    if (s == "recordsy") return TokenType::RECORDSY;
    if (s == "beginsy") return TokenType::BEGINSY;
    if (s == "endsy") return TokenType::ENDSY;
    if (s == "ifsy") return TokenType::IFSY;
    if (s == "thensy") return TokenType::THENSY;
    if (s == "elsesy") return TokenType::ELSESY;
    if (s == "whilesy") return TokenType::WHILESY;
    if (s == "dosy") return TokenType::DOSY;
    if (s == "repeatsy") return TokenType::REPEATSY;
    if (s == "untilsy") return TokenType::UNTILSY;
    if (s == "forsy") return TokenType::FORSY;
    if (s == "tosy") return TokenType::TOSY;
    if (s == "downtosy") return TokenType::DOWNTOSY;
    if (s == "casesy") return TokenType::CASESY;
    if (s == "proceduresy") return TokenType::PROCEDURESY;
    if (s == "functionsy") return TokenType::FUNCTIONSY;
    if (s == "ident") return TokenType::IDENT;
    if (s == "intcon") return TokenType::INTCON;
    if (s == "realcon") return TokenType::REALCON;
    if (s == "charcon") return TokenType::CHARCON;
    if (s == "string") return TokenType::STRING;
    if (s == "plus") return TokenType::PLUS;
    if (s == "minus") return TokenType::MINUS;
    if (s == "times") return TokenType::TIMES;
    if (s == "rdiv") return TokenType::RDIV;
    if (s == "idiv") return TokenType::IDIV;
    if (s == "imod") return TokenType::IMOD;
    if (s == "eql") return TokenType::EQL;
    if (s == "neq") return TokenType::NEQ;
    if (s == "lss") return TokenType::LSS;
    if (s == "leq") return TokenType::LEQ;
    if (s == "gtr") return TokenType::GTR;
    if (s == "geq") return TokenType::GEQ;
    if (s == "orsy") return TokenType::ORSY;
    if (s == "andsy") return TokenType::ANDSY;
    if (s == "notsy") return TokenType::NOTSY;
    if (s == "becomes") return TokenType::BECOMES;
    if (s == "lparent") return TokenType::LPARENT;
    if (s == "rparent") return TokenType::RPARENT;
    if (s == "lbrack") return TokenType::LBRACK;
    if (s == "rbrack") return TokenType::RBRACK;
    if (s == "period") return TokenType::PERIOD;
    if (s == "comma") return TokenType::COMMA;
    if (s == "colon") return TokenType::COLON;
    if (s == "semicolon") return TokenType::SEMICOLON;
    return TokenType::UNKNOWN;
}

// Fungsi untuk menghapus spasi/enter di awal dan akhir string
string trimStr(const string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (string::npos == first) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

// Mesin pembaca file token hasil Milestone 1
vector<Token> parseTokenFile(const string& content) {
    vector<Token> tokens;
    istringstream iss(content);
    string line;
    
    while (getline(iss, line)) {
        line = trimStr(line);
        if (line.empty()) continue;

        string typeStr = line;
        string valueStr = "";

        // Cek apakah formatnya tipe(nilai), misal: ident(LuasLingkaran)
        size_t openParen = line.find('(');
        size_t closeParen = line.rfind(')');
        
        if (openParen != string::npos && closeParen != string::npos && closeParen > openParen) {
            typeStr = line.substr(0, openParen);
            typeStr = trimStr(typeStr);
            valueStr = line.substr(openParen + 1, closeParen - openParen - 1);
        }

        TokenType type = stringToType(typeStr);
        tokens.push_back(Token(type, valueStr));
    }
    return tokens;
}

// Fungsi pendeteksi: Apakah ini source code asli atau file token?
bool isTokenizedFile(const string& content) {
    istringstream iss(content);
    string firstWord;
    iss >> firstWord;
    firstWord = trimStr(firstWord);
    
    // Mengecek apakah kata pertama adalah label token (misal: programsy atau ident(...))
    size_t openParen = firstWord.find('(');
    string typeStr = (openParen != string::npos) ? firstWord.substr(0, openParen) : firstWord;
    
    // Kata "program" di source code akan mengembalikan UNKNOWN, 
    // tapi "programsy" dari file token akan mengembalikan PROGRAMSY.
    return (stringToType(typeStr) != TokenType::UNKNOWN);
}

int main(int argc, char* argv[]) {

  // Validasi jumlah argumen
  if (argc < 3) {
    cout << "Error: Argumen Kurang" << endl;
    cout << "Usage: ./arion_lexer <file_input> <file_output>" << endl;
    return 1;
  }

  // Baca isi file input
  ifstream fileInput(argv[1]);
  if (!fileInput.is_open()) {
    cout << "Error: File Input Tidak Ditemukan (" << argv[1] << ")" << endl;
    return 1;
  }

  stringstream buffer;
  buffer << fileInput.rdbuf();
  string fileContent = buffer.str();
  fileInput.close();

  vector<Token> tokens;

  if (isTokenizedFile(fileContent)) {
      cout << "[INFO] Format file Token terdeteksi. Membaca langsung dari hasil Milestone 1..." << endl;
      tokens = parseTokenFile(fileContent);
  } else {
      cout << "[INFO] Format file Source Code terdeteksi. Menjalankan Lexer..." << endl;
      Lexer lexer(fileContent);
      tokens = lexer.tokenize();
  }

  // Syntax analysis (Milestone 2)
  Parser parser(tokens);
  ParseTreeNode* tree = parser.parse();

  // Buka file output untuk hasil parse tree
  ofstream fileOutput(argv[2]);
  if (!fileOutput.is_open()) {
    cout << "Error: Tidak Bisa Membuat File Output (" << argv[2] << ")" << endl;
    delete tree;
    return 1;
  }

  // Handle Error
  if (parser.isError()) {
    cout << "Parsing gagal: terdapat syntax error." << endl;
    fileOutput << "Parsing gagal: terdapat syntax error." << endl;
    fileOutput.close();
    delete tree;
    return 1;
  }

  // Tulis parse tree ke file .txt
  parser.printTree(tree, fileOutput);
  fileOutput.close();

  // Cetak Parse Tree ke layar terminal
  ifstream readBack(argv[2]);
  if (readBack.is_open()) {
    cout << readBack.rdbuf();
    readBack.close();
  }

  delete tree;
  return 0;
}