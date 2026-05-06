#include "lexer.h"

#include <fstream>
#include <sstream>

int main(int argc, char* argv[]) {
  if (argc < 2 || argc > 3) {
    cerr << "Usage: " << argv[0]
       << " <source_file> [output_file]" << endl;
    return 1;
  }

  ifstream input_file(argv[1]);
  if (!input_file.is_open()) {
    cerr << "Error: gagal membuka file " << argv[1] << endl;
    return 1;
  }

  ostringstream buffer;
  buffer << input_file.rdbuf();

  Lexer lexer(buffer.str());
  const vector<Token> tokens = lexer.tokenize();

  ostream* out = &cout;
  ofstream output_file;
  if (argc == 3) {
    output_file.open(argv[2]);
    if (!output_file.is_open()) {
      cerr << "Error: tidak bisa membuat file output " << argv[2] << endl;
      return 1;
    }
    out = &output_file;
  }

  for (const Token& token : tokens) {
    if (token.type == TokenType::END_OF_FILE) continue;

    const string type_str = lexer.tokenTypeToString(token.type);
    if (token.type == TokenType::IDENT   ||
      token.type == TokenType::INTCON  ||
      token.type == TokenType::REALCON ||
      token.type == TokenType::CHARCON ||
      token.type == TokenType::STRING  ||
      token.type == TokenType::UNKNOWN) {
      *out << type_str << " (" << token.value << ")" << endl;
    } else {
      *out << type_str << endl;
    }
  }

  return 0;
}
