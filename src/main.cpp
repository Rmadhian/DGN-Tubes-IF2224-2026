#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include "lexer.h"

using namespace std;

int main(int argc, char* argv[]) {

  // Validasi jumlah argumen: harus ada file input dan output
  if (argc < 3) {
    cout << "Error: Argumen Kurang" << endl;
    cout << "Usage: ./arion_lexer <file_input> <file_output>" << endl;
    return 1;
  }

  // Buka file input Pascal
  ifstream fileInput(argv[1]);

  if (!fileInput.is_open()) {
    cout << "Error: File Tidak Ditemuukan" << endl;
    return 1;
  }

  // Buka/buat file output untuk hasil tokenisasi
  ofstream fileOutput(argv[2]);

  if (!fileOutput.is_open()) {
    cout << "Error: Tidak Bisa Membuat File Output" << endl;
    return 1;
  }

  // Baca seluruh isi file ke string
  stringstream buffer;
  buffer << fileInput.rdbuf();

  string sourceCode = buffer.str();

  // Jalankan tokenisasi
  Lexer lexer(sourceCode);
  vector<Token> hasilToken = lexer.tokenize();

  // Tulis hasil token ke file output sesuai format
  for(int i = 0; i < hasilToken.size(); i++) {
    fileOutput << lexer.tokenTypeToString(hasilToken[i].type);

    if (hasilToken[i].value != "") {
      fileOutput << " (" << hasilToken[i].value << ")";
    }

    fileOutput << endl;
  }

  fileInput.close();
  fileOutput.close();

  return 0;
}