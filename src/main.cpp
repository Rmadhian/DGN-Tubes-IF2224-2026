#include "lexer.h"

#include <fstream>
#include <sstream>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <source_file>" << endl;
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

    for (const Token& token : tokens) {
        if (token.type == TokenType::END_OF_FILE) continue;

        const string type_str = lexer.tokenTypeToString(token.type);

        // Value tokens print: tokentype (value)
        if (token.type == TokenType::IDENT   ||
            token.type == TokenType::INTCON  ||
            token.type == TokenType::REALCON ||
            token.type == TokenType::CHARCON ||
            token.type == TokenType::STRING) {
            cout << type_str << " (" << token.value << ")" << endl;
        } else {
            cout << type_str << endl;
        }
    }

    return 0;
}
