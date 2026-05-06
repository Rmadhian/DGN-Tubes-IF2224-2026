#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include "lexer.h"
#include "parser.h"

using namespace std;

int main(int argc, char* argv[]) {

  // Validasi jumlah argumen: butuh file input (source Arion) dan file output (parse tree)
  if (argc < 3) {
    cout << "Error: Argumen Kurang" << endl;
    cout << "Usage: ./arion <file_input> <file_output>" << endl;
    return 1;
  }

  // Baca source code Arion
  ifstream fileInput(argv[1]);
  if (!fileInput.is_open()) {
    cout << "Error: File Input Tidak Ditemukan (" << argv[1] << ")" << endl;
    return 1;
  }

  stringstream buffer;
  buffer << fileInput.rdbuf();
  string sourceCode = buffer.str();
  fileInput.close();

  // Lexical analysis (Milestone 1)
  Lexer lexer(sourceCode);
  vector<Token> tokens = lexer.tokenize();

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

  if (parser.isError()) {
    cout << "Parsing gagal: terdapat syntax error." << endl;
    fileOutput << "Parsing gagal: terdapat syntax error." << endl;
    fileOutput.close();
    delete tree;
    return 1;
  }

  // Tulis parse tree ke file (sesuai spek: disimpan ke .txt)
  parser.printTree(tree, fileOutput);
  fileOutput.close();

  // Cetak juga ke terminal (sesuai spek: print ke terminal)
  ifstream readBack(argv[2]);
  if (readBack.is_open()) {
    cout << readBack.rdbuf();
    readBack.close();
  }

  delete tree;
  return 0;
}
