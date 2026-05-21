#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "lexer.h"
#include "parser.h"
#include "ast_builder.h"
#include "semantic.h"

using namespace std;

// =========================================================
// Utility: Baca file teks
// =========================================================
static string readFile(const string& path) {
    ifstream f(path);
    if (!f.is_open()) {
        cerr << "Error: tidak bisa membuka file '" << path << "'\n";
        return "";
    }
    return string((istreambuf_iterator<char>(f)),
                   istreambuf_iterator<char>());
}

// =========================================================
// Main
// =========================================================
int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Penggunaan: " << argv[0] << " <input.pas>\n";
        cerr << "  Contoh: " << argv[0] << " program.pas\n";
        return 1;
    }

    string inputFile = argv[1];

    // ---- Tahap 1: Lexical Analysis ----
    cout << "=== [Tahap 1] Lexical Analysis ===\n";
    string source = readFile(inputFile);
    if (source.empty()) return 1;

    Lexer lexer(source);
    vector<Token> tokens = lexer.tokenize();

    cout << "Total token dihasilkan: " << tokens.size() << "\n\n";

    // ---- Tahap 2: Syntax Analysis (Parsing) ----
    cout << "=== [Tahap 2] Syntax Analysis (Parsing) ===\n";
    Parser parser(tokens);
    ParseTreeNode* parseTree = parser.parse();

    if (parser.isError() || !parseTree) {
        cerr << "Parsing gagal. Semantic analysis dihentikan.\n";
        delete parseTree;
        return 1;
    }
    cout << "Parsing berhasil.\n\n";

    // Opsional: cetak parse tree ke file
    string parseTreeFile = "parse_tree.txt";
    {
        ofstream ptOut(parseTreeFile);
        if (ptOut.is_open()) {
            parser.printTree(parseTree, ptOut);
            cout << "Parse tree ditulis ke: " << parseTreeFile << "\n\n";
        }
    }

    // ---- Tahap 3: Konversi Parse Tree → AST ----
    cout << "=== [Tahap 3] Konversi Parse Tree → AST ===\n";
    ASTBuilder builder;
    ProgramNode* ast = builder.build(parseTree);

    if (!ast) {
        cerr << "Gagal membangun AST dari parse tree.\n";
        delete parseTree;
        return 1;
    }
    cout << "AST berhasil dibangun.\n\n";

    // ---- Tahap 4: Semantic Analysis ----
    cout << "=== [Tahap 4] Semantic Analysis ===\n";
    SemanticAnalyzer analyzer;
    ast->accept(&analyzer);

    if (analyzer.hasError) {
        cout << "\nSemantic Analysis selesai dengan ERROR.\n";
    } else {
        cout << "Semantic Analysis selesai tanpa error.\n";
    }

    // ---- Output: Decorated AST ----
    cout << "\n=== Decorated Abstract Syntax Tree (AST) ===\n";
    ast->print(cout, 0, &analyzer.st);

    // ---- Output: Symbol Tables ----
    analyzer.st.printTab(cout);
    analyzer.st.printBTab(cout);
    analyzer.st.printATab(cout);

    // ---- Cleanup ----
    delete parseTree;
    // ast nodes dibersihkan oleh destructor (tidak diimplementasikan di sini
    // karena scope tugas hanya sampai semantic analysis)

    return analyzer.hasError ? 1 : 0;
}