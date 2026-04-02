#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

using namespace std;

int main(int argc, char* argv[]) {

  if (argc < 3) {
    cout << "Error: Argumen Kurang" << endl;
    cout << "Usage: ./arion_lexer <file_input> <file_output>" << endl;
    return 1;
  }

  ifstream fileInput(argv[1]);

  if (!fileInput.is_open()) {
    cout << "Error: File Tidak Ditemuukan" << endl;
    return 1;
  }

  ofstream fileOutput(argv[2]);

  if (!fileOutput.is_open()) {
    cout << "Error: Tidak Bisa Membuat File Output" << endl;
    return 1;
  }

  stringstream buffer;
  buffer << fileInput.rdbuf();

  string sourceCode = buffer.str();

  Lexer lexer(sourceCode);
  vector<Token> hasilToken = lexer.tokenize();

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