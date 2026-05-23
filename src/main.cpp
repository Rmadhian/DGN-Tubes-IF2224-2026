#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <iomanip>
#include "lexer.h"
#include "parser.h"
#include "ast_builder.h"
#include "semantic.h"

using namespace std;

// Utility file reader & parser helper

enum class InputType { SOURCE_CODE, TOKEN_LIST, PARSE_TREE };

TokenType stringToType(string s) {
    if (s == "programsy") return TokenType::PROGRAMSY;
    if (s == "constsy") return TokenType::CONSTSY;
    if (s == "typesy") return TokenType::TYPESY;
    if (s == "varsy") return TokenType::VARSY;
    if (s == "beginsy") return TokenType::BEGINSY;
    if (s == "endsy") return TokenType::ENDSY;
    if (s == "ident") return TokenType::IDENT;
    if (s == "intcon") return TokenType::INTCON;
    if (s == "realcon") return TokenType::REALCON;
    if (s == "string") return TokenType::STRING;
    if (s == "plus") return TokenType::PLUS;
    if (s == "becomes") return TokenType::BECOMES;
    if (s == "lparent") return TokenType::LPARENT;
    if (s == "rparent") return TokenType::RPARENT;
    if (s == "semicolon") return TokenType::SEMICOLON;
    if (s == "comma") return TokenType::COMMA;
    return TokenType::UNKNOWN;
}

string trimStr(const string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (string::npos == first) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

InputType detectInputType(const string& content) {
    istringstream iss(content);
    string firstLine;
    while(getline(iss, firstLine)) {
        firstLine = trimStr(firstLine);
        if (!firstLine.empty()) break;
    }
    
    // Cek jika ini format Parse Tree Milestone 2
    if (firstLine.find("<program>") != string::npos || firstLine.find("\xE2\x94") != string::npos) {
        return InputType::PARSE_TREE;
    }
    // Cek jika ini format Token List Milestone 1
    size_t openParen = firstLine.find('(');
    string typeStr = (openParen != string::npos) ? firstLine.substr(0, openParen) : firstLine;
    if (stringToType(trimStr(typeStr)) != TokenType::UNKNOWN) {
        return InputType::TOKEN_LIST;
    }
    return InputType::SOURCE_CODE;
}

vector<Token> parseTokenFile(const string& content) {
    vector<Token> tokens;
    istringstream iss(content);
    string line;
    while (getline(iss, line)) {
        line = trimStr(line);
        if (line.empty()) continue;
        size_t openParen = line.find('(');
        size_t closeParen = line.rfind(')');
        if (openParen != string::npos && closeParen != string::npos && closeParen > openParen) {
            string typeStr = trimStr(line.substr(0, openParen));
            string valueStr = line.substr(openParen + 1, closeParen - openParen - 1);
            tokens.push_back(Token(stringToType(typeStr), valueStr));
        }
    }
    return tokens;
}

// Reconstructor parse tree (Milestone 2 -> Memori AST)

// Membaca satu baris Parse Tree dan membuatnya menjadi node
ParseTreeNode* parseTreeLine(const string& line, int& outStartIdx) {
    int startIdx = -1;
    for (int i = 0; i < line.length(); i++) {
        // Cari karakter awal dari label (Bisa '<' untuk Non-terminal, atau huruf kapital untuk Terminal)
        if (line[i] == '<' || isalnum((unsigned char)line[i])) {
            startIdx = i; break;
        }
    }
    if (startIdx == -1) return nullptr;

    outStartIdx = startIdx;
    string content = line.substr(startIdx);
    string label = "", value = "";
    
    if (content[0] == '<') {
        size_t endPos = content.find('>');
        label = (endPos != string::npos) ? content.substr(0, endPos + 1) : content;
    } else {
        size_t openParen = content.find('(');
        size_t closeParen = content.rfind(')');
        if (openParen != string::npos && closeParen != string::npos && closeParen > openParen) {
            label = trimStr(content.substr(0, openParen));
            value = content.substr(openParen + 1, closeParen - openParen - 1);
        } else {
            label = trimStr(content);
        }
    }
    return new ParseTreeNode(label, value);
}

// Membangun kembali pohon menggunakan Stack berbasis kedalaman indentasi
ParseTreeNode* buildParseTreeFromText(const string& content) {
    istringstream iss(content);
    string line;
    ParseTreeNode* root = nullptr;
    vector<pair<int, ParseTreeNode*>> stack;
    
    while (getline(iss, line)) {
        if (trimStr(line).empty()) continue;
        
        int startIdx = 0;
        ParseTreeNode* node = parseTreeLine(line, startIdx);
        if (!node) continue;
        
        if (stack.empty()) {
            root = node;
            stack.push_back({startIdx, node});
        } else {
            // Jika depth node saat ini lebih kecil/sama dengan elemen di stack, pop!
            while (!stack.empty() && stack.back().first >= startIdx) {
                stack.pop_back();
            }
            if (!stack.empty()) {
                stack.back().second->children.push_back(node);
            }
            stack.push_back({startIdx, node});
        }
    }
    return root;
}

// Main program

int main(int argc, char* argv[]) {
    if (argc < 3) {
        cout << "Usage: ./arion_compiler <file_input> <file_output>" << endl;
        return 1;
    }

    ifstream fileInput(argv[1]);
    if (!fileInput.is_open()) return 1;

    stringstream buffer;
    buffer << fileInput.rdbuf();
    string fileContent = buffer.str();
    fileInput.close();

    InputType type = detectInputType(fileContent);
    ParseTreeNode* tree = nullptr;
    Parser* parserPtr = nullptr; // Digunakan jika input perlu melewati parser

    // TAHAP 1 & 2: MEMBANGUN PARSE TREE
    if (type == InputType::PARSE_TREE) {
        cout << "[INFO] Format file Parse Tree terdeteksi. Melakukan rekonstruksi Tree..." << endl;
        tree = buildParseTreeFromText(fileContent);
    } else {
        vector<Token> tokens;
        if (type == InputType::TOKEN_LIST) {
            cout << "[INFO] Format file Token terdeteksi. Membaca Token..." << endl;
            tokens = parseTokenFile(fileContent);
        } else {
            cout << "[INFO] Format file Source Code terdeteksi. Menjalankan Lexer..." << endl;
            Lexer lexer(fileContent);
            tokens = lexer.tokenize();
        }
        
        cout << "[INFO] Menjalankan Syntax Analyzer (Parser)..." << endl;
        parserPtr = new Parser(tokens);
        tree = parserPtr->parse();
        
        if (parserPtr->isError()) {
            cout << "Parsing gagal: terdapat syntax error." << endl;
            delete tree;
            delete parserPtr;
            return 1;
        }
    }

    // TAHAP 3: SEMANTIC ANALYSIS
    ofstream fileOutput(argv[2]);
    if (!fileOutput.is_open()) return 1;

    cout << "[INFO] Memulai pembangunan AST (Syntax-Directed Translation)..." << endl;
    ASTBuilder astBuilder;
    ProgramNode* astRoot = astBuilder.build(tree);

    if (astRoot != nullptr) {
        cout << "[INFO] Menjalankan Semantic Analyzer (Decorating AST & Symbol Table)..." << endl;
        
        SemanticAnalyzer semanticAnalyzer;
        astRoot->accept(&semanticAnalyzer); 

        cout << "[SUCCESS] Semantic Analysis Selesai." << endl;
        
        // Panggil fungsi printSymbolTables
        printSymbolTables(semanticAnalyzer.st, fileOutput);
        
        fileOutput << "Decorated AST:\n";
        
        // Panggil Visitor PrintASTVisitor dengan meneruskan stream fileOutput
        PrintASTVisitor astPrinter(fileOutput);
        astRoot->accept(&astPrinter);
        
    } else {
        cout << "Error Semantik: Gagal membangun Abstract Syntax Tree dari struktur Parse Tree." << endl;
    }

    fileOutput.close();
    delete tree; 
    if (parserPtr) delete parserPtr; 

    return 0;
}