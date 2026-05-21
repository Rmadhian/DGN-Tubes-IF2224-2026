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

// Formatter untuk output

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

void printSymbolTables(const SymbolTable& st, ostream& out) {
    out << "\ntab (hanya sebagian yang relevan):\n";
    out << left << setw(4) << "idx" << setw(15) << "id" << setw(15) << "obj" 
        << setw(10) << "type" << setw(5) << "ref" << setw(5) << "nrm" 
        << setw(5) << "lev" << setw(5) << "adr" << "link\n";
    out << string(65, '-') << "\n";
    
    for (size_t i = 0; i < st.tab.size(); i++) {
        if (i == 0) out << "... (reserved words & predefined)\n";
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

void printAST(ASTNode* node, string indent, bool isLast, ostream& out, string prefix = "") {
    if (!node) return;

    // Mengganti karakter ASCII dengan Unicode Box-Drawing
    string marker = isLast ? " └─ " : " ├─ "; 
    string nextIndent = indent + (isLast ? "    " : " │  ");
    
    // Siapkan Anotasi Semantik
    string anotasi = "";
    if (node->symRef != -1 || node->evalType != DataType::NOTYPE) {
        anotasi += " \t-> "; // Anda juga bisa mengubah ini menjadi " \t→ " jika ingin panah Unicode
        if (node->symRef != -1) anotasi += "tab_index:" + to_string(node->symRef) + ", ";
        anotasi += "type:" + typeToStr(node->evalType);
        if (node->lexicalLevel >= 0 && node->symRef != -1) anotasi += ", lev:" + to_string(node->lexicalLevel);
    }

    // Identifikasi Tipe Node dengan dynamic_cast
    if (auto p = dynamic_cast<ProgramNode*>(node)) {
        out << indent << "ProgramNode(name: '" << p->name << "')\n";
        out << indent << " ├─ Declarations\n";
        for (size_t i = 0; i < p->declarations.size(); i++) {
            printAST(p->declarations[i], indent + " │  ", (i == p->declarations.size() - 1), out);
        }
        out << indent << " └─ Block";
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
        
        fileOutput << "[HASIL SEMANTIC ANALYSIS]\n";
        
        printSymbolTables(semanticAnalyzer.st, fileOutput);
        
        fileOutput << "Decorated AST:\n";
        printAST(astRoot, "", true, fileOutput);
        
        fileOutput << "\n[Analisis Semantik Sukses]\n";
        fileOutput << "Program berhasil lolos verifikasi tipe, scope, dan deklarasi.\n";
    } else {
        cout << "Error Semantik: Gagal membangun Abstract Syntax Tree dari struktur Parse Tree." << endl;
    }

    fileOutput.close();
    delete tree; 
    if (parserPtr) delete parserPtr; 

    return 0;
}