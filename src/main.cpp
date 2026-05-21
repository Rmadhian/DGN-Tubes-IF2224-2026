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

// Format untuk output

string objToStr(ObjClass obj) {
    switch (obj) {
        case ObjClass::CONSTANT: return "constant";
        case ObjClass::VARIABLE: return "variable";
        case ObjClass::TYPE_DEF: return "type_def";
        case ObjClass::PROCEDURE: return "procedure";
        case ObjClass::FUNCTION: return "function";
        default: return "unknown";
    }
}

string typeToStr(DataType t) {
    switch (t) {
        case DataType::INTEGER: return "integer";
        case DataType::REAL: return "real";
        case DataType::CHAR: return "char";
        case DataType::BOOLEAN: return "boolean";
        case DataType::STRING: return "string";
        case DataType::ARRAY: return "array";
        case DataType::RECORD: return "record";
        case DataType::NONE: return "void";
        case DataType::NOTYPE: return "notype";
        default: return "?";
    }
}

// Fungsi Pencetak Symbol Table (tab, btab, atab)
void printSymbolTables(const SymbolTable& st, ostream& out) {
    out << "\ntab (hanya sebagian yang relevan):\n";
    out << left << setw(4) << "idx" << setw(15) << "id" << setw(15) << "obj" 
        << setw(10) << "type" << setw(5) << "ref" << setw(5) << "nrm" 
        << setw(5) << "lev" << setw(5) << "adr" << "link\n";
    out << string(65, '-') << "\n";
    
    for (size_t i = 0; i < st.tab.size(); i++) {
        // Asumsi predefined identifiers ada di index awal (misal 0-6)
        if (i == 0) {
            out << "... (reserved words & predefined)\n";
        }
        // Kita lewati cetak yang predefined untuk memperingkas output, mulai cetak user-defined
        if (i < 7) continue; 
        
        const auto& t = st.tab[i];
        out << left << setw(4) << i << setw(15) << t.identifiers 
            << setw(15) << objToStr(t.obj) << setw(10) << typeToStr(t.type) 
            << setw(5) << t.ref << setw(5) << t.nrm << setw(5) << t.lev 
            << setw(5) << t.adr << t.link << "\n";
    }

    out << "\nbtab:\n";
    out << left << setw(4) << "idx" << setw(6) << "last" << setw(6) << "lpar" 
        << setw(6) << "psze" << "vsze\n";
    out << string(30, '-') << "\n";
    for(size_t i = 0; i < st.btab.size(); i++) {
        const auto& b = st.btab[i];
        out << left << setw(4) << i << setw(6) << b.last << setw(6) << b.lpar 
            << setw(6) << b.psze << b.vsze << "\n";
    }

    out << "\natab:\n";
    if (st.atab.empty()) {
        out << "(kosong karena tidak ada array)\n";
    } else {
        out << left << setw(6) << "arrays" << setw(6) << "xtyp" << setw(6) << "etyp" 
            << setw(6) << "eref" << setw(6) << "low" << setw(6) << "high" 
            << setw(6) << "elsz" << "size\n";
        out << string(55, '-') << "\n";
        for(size_t i = 0; i < st.atab.size(); i++) {
            const auto& a = st.atab[i];
            out << left << setw(6) << a.arrays << setw(6) << (int)a.xtyp << setw(6) << (int)a.etyp 
                << setw(6) << a.eref << setw(6) << a.low << setw(6) << a.high 
                << setw(6) << a.elsz << a.size << "\n";
        }
    }
    out << "\n";
}

// Fungsi Pencetak Decorated AST dengan penelusuran hierarki
void printAST(ASTNode* node, string indent, bool isLast, ostream& out, string prefix = "") {
    if (!node) return;

    string marker = isLast ? " \\_ " : " |- "; 
    string nextIndent = indent + (isLast ? "    " : " |  ");
    
    // Siapkan Anotasi Semantik
    string anotasi = "";
    if (node->symRef != -1 || node->evalType != DataType::NOTYPE) {
        anotasi += " \t-> ";
        if (node->symRef != -1) anotasi += "tab_index:" + to_string(node->symRef) + ", ";
        anotasi += "type:" + typeToStr(node->evalType);
        if (node->lexicalLevel >= 0 && node->symRef != -1) anotasi += ", lev:" + to_string(node->lexicalLevel);
    }

    // Identifikasi Tipe Node dengan dynamic_cast
    if (auto p = dynamic_cast<ProgramNode*>(node)) {
        out << indent << "ProgramNode(name: '" << p->name << "')\n";
        out << indent << " |- Declarations\n";
        for (size_t i = 0; i < p->declarations.size(); i++) {
            printAST(p->declarations[i], indent + " |  ", (i == p->declarations.size() - 1), out);
        }
        out << indent << " \\_ Block";
        if (p->mainBlock) {
            out << " \t-> block_index:" << p->mainBlock->symRef << ", lev:" << p->mainBlock->lexicalLevel << "\n";
            auto block = dynamic_cast<CompoundStmtNode*>(p->mainBlock);
            for (size_t i = 0; block && i < block->statements.size(); i++) {
                printAST(block->statements[i], indent + "    ", (i == block->statements.size() - 1), out);
            }
        } else out << "\n";
    } 
    else if (auto v = dynamic_cast<VarDeclNode*>(node)) {
        out << indent << marker << "VarDecl(";
        for(size_t i=0; i<v->idents.size(); i++) out << "'" << v->idents[i] << "'" << (i+1==v->idents.size()?"":", ");
        out << ")" << anotasi << "\n";
    }
    else if (auto c = dynamic_cast<ConstDeclNode*>(node)) {
        out << indent << marker << "ConstDecl('" << c->name << "' = " << c->value << ")" << anotasi << "\n";
    }
    else if (auto a = dynamic_cast<AssignStmtNode*>(node)) {
        out << indent << marker << "Assign" << anotasi << "\n";
        printAST(a->left, nextIndent, false, out, "target ");
        printAST(a->right, nextIndent, true, out, "value ");
    }
    else if (auto b = dynamic_cast<BinaryOpNode*>(node)) {
        out << indent << marker << prefix << "BinOp '" << b->op << "'" << anotasi << "\n";
        printAST(b->left, nextIndent, false, out);
        printAST(b->right, nextIndent, true, out);
    }
    else if (auto u = dynamic_cast<UnaryOpNode*>(node)) {
        out << indent << marker << prefix << "UnaryOp '" << u->op << "'" << anotasi << "\n";
        printAST(u->operand, nextIndent, true, out);
    }
    else if (auto va = dynamic_cast<VarAccessNode*>(node)) {
        out << indent << marker << prefix << "'" << va->name << "'" << anotasi << "\n";
    }
    else if (auto lit = dynamic_cast<LiteralNode*>(node)) {
        out << indent << marker << prefix << lit->value << anotasi << "\n";
    }
    else if (auto fc = dynamic_cast<FuncCallNode*>(node)) {
        out << indent << marker << prefix << fc->name << "(...)" << anotasi << "\n";
        for (size_t i = 0; i < fc->args.size(); i++) {
            printAST(fc->args[i], nextIndent, (i == fc->args.size() - 1), out, "arg ");
        }
    }
    else if (auto comp = dynamic_cast<CompoundStmtNode*>(node)) {
        out << indent << marker << "CompoundStmt\n";
        for (size_t i = 0; i < comp->statements.size(); i++) {
            printAST(comp->statements[i], nextIndent, (i == comp->statements.size() - 1), out);
        }
    }
    else if (auto iff = dynamic_cast<IfStmtNode*>(node)) {
        out << indent << marker << "IfStmt" << anotasi << "\n";
        printAST(iff->condition, nextIndent, false, out, "condition ");
        printAST(iff->thenStmt, nextIndent, (iff->elseStmt == nullptr), out, "then ");
        if (iff->elseStmt) printAST(iff->elseStmt, nextIndent, true, out, "else ");
    }
    else if (auto wh = dynamic_cast<WhileStmtNode*>(node)) {
        out << indent << marker << "WhileStmt" << anotasi << "\n";
        printAST(wh->condition, nextIndent, false, out, "condition ");
        printAST(wh->body, nextIndent, true, out, "body ");
    }
    else if (auto fr = dynamic_cast<ForStmtNode*>(node)) {
        out << indent << marker << "ForStmt(iter: '" << fr->iterVar << "')" << anotasi << "\n";
        printAST(fr->startExpr, nextIndent, false, out, "start ");
        printAST(fr->endExpr, nextIndent, false, out, "end ");
        printAST(fr->body, nextIndent, true, out, "body ");
    }
    else {
        out << indent << marker << "UnknownNode\n";
    }
}

// Fungsi utility pembacaan file token (MILESTONE 1)

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

string trimStr(const string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (string::npos == first) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

vector<Token> parseTokenFile(const string& content) {
    vector<Token> tokens;
    istringstream iss(content);
    string line;
    
    while (getline(iss, line)) {
        line = trimStr(line);
        if (line.empty()) continue;

        string typeStr = line;
        string valueStr = "";

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

bool isTokenizedFile(const string& content) {
    istringstream iss(content);
    string firstWord;
    iss >> firstWord;
    firstWord = trimStr(firstWord);
    
    size_t openParen = firstWord.find('(');
    string typeStr = (openParen != string::npos) ? firstWord.substr(0, openParen) : firstWord;
    
    return (stringToType(typeStr) != TokenType::UNKNOWN);
}

// Program utama (MAIN)

int main(int argc, char* argv[]) {

    if (argc < 3) {
        cout << "Error: Argumen Kurang" << endl;
        cout << "Usage: ./arion_compiler <file_input> <file_output>" << endl;
        return 1;
    }

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

    // 1. TAHAP LEXICAL ANALYSIS
    if (isTokenizedFile(fileContent)) {
        cout << "[INFO] Format file Token terdeteksi. Membaca dari hasil Milestone 1..." << endl;
        tokens = parseTokenFile(fileContent);
    } else {
        cout << "[INFO] Format file Source Code terdeteksi. Menjalankan Lexer..." << endl;
        Lexer lexer(fileContent);
        tokens = lexer.tokenize();
    }

    // 2. TAHAP SYNTAX ANALYSIS (PARSER)
    cout << "[INFO] Menjalankan Syntax Analyzer (Parser)..." << endl;
    Parser parser(tokens);
    ParseTreeNode* tree = parser.parse();

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

    // (Opsional) Tulis parse tree dari tahap 2 ke file
    // parser.printTree(tree, fileOutput); 

    // 3. TAHAP SEMANTIC ANALYSIS
    cout << "[INFO] Memulai pembangunan AST (Syntax-Directed Translation)..." << endl;
    ASTBuilder astBuilder;
    ProgramNode* astRoot = astBuilder.build(tree);

    if (astRoot != nullptr) {
        cout << "[INFO] Menjalankan Semantic Analyzer (Decorating AST & Symbol Table)..." << endl;
        
        // Menjalankan pengecekan tipe dan scope
        SemanticAnalyzer semanticAnalyzer;
        astRoot->accept(&semanticAnalyzer); 

        cout << "===========================================" << endl;
        cout << "[SUCCESS] Semantic Analysis Selesai." << endl;
        cout << "Total Global Symbol terdaftar: " << semanticAnalyzer.st.tab.size() << endl;
        cout << "===========================================" << endl;
        
        // Cetak output ke file output.txt
        fileOutput << "===========================================\n";
        fileOutput << "[HASIL SEMANTIC ANALYSIS]\n";
        fileOutput << "===========================================\n";
        
        printSymbolTables(semanticAnalyzer.st, fileOutput);
        
        fileOutput << "Decorated AST:\n";
        printAST(astRoot, "", true, fileOutput);
        
        fileOutput << "\n[Analisis Semantik Sukses]\n";
        fileOutput << "Program berhasil lolos verifikasi tipe, scope, dan deklarasi.\n";

    } else {
        cout << "Error Semantik: Gagal membangun Abstract Syntax Tree dari struktur Parse Tree." << endl;
    }

    fileOutput.close();
    delete tree; // Cleanup memory Parse Tree

    return 0;
}