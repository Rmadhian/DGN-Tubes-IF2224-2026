#include "ast_parser.h"
#include <sstream>
#include <algorithm>

class DummyNode : public ASTNode {
public:
    void accept(SemanticVisitor* visitor) override {}
};

using namespace std;

ASTParser::ASTParser(const string& fileContent) {
    this->content = fileContent;
    this->root = nullptr;
    this->error = false;
}

vector<string> ASTParser::splitLines(const string& str) {
    vector<string> lines;
    istringstream iss(str);
    string line;
    while (getline(iss, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        lines.push_back(line);
    }
    return lines;
}

string ASTParser::trim(const string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (string::npos == first) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

vector<string> ASTParser::splitWhitespace(const string& str) {
    vector<string> parts;
    istringstream iss(str);
    string part;
    while (iss >> part) {
        parts.push_back(part);
    }
    return parts;
}

ObjClass ASTParser::strToObjClass(const string& str) {
    if (str == "const") return ObjClass::CONSTANT;
    if (str == "variable") return ObjClass::VARIABLE;
    if (str == "type") return ObjClass::TYPE_DEF;
    if (str == "procedure") return ObjClass::PROCEDURE;
    if (str == "function") return ObjClass::FUNCTION;
    return ObjClass::CONSTANT; // default dummy
}

void ASTParser::parse() {
    vector<string> lines = splitLines(content);
    size_t i = 0;
    while (i < lines.size()) {
        string line = trim(lines[i]);
        if (line == "tab:") {
            parseSymbolTable(lines, i);
        } else if (line == "Decorated AST:") {
            parseDecoratedAST(lines, i);
        } else {
            i++;
        }
    }
}

void ASTParser::parseSymbolTable(const vector<string>& lines, size_t& i) {
    // 1. Parse tab:
    i++; // lewati "tab:"
    while (i < lines.size() && lines[i].find("----") == string::npos) i++; // cari divider
    i++; // lewati divider
    
    st.tab.clear();
    while (i < lines.size() && !trim(lines[i]).empty() && trim(lines[i]) != "btab:") {
        vector<string> cols = splitWhitespace(lines[i]);
        if (cols.size() >= 7) {
            int idx = stoi(cols[0]);
            if (st.tab.size() <= idx) {
                st.tab.resize(idx + 1);
            }
            TabEntry t;
            t.link = stoi(cols[cols.size()-1]);
            t.adr = stoi(cols[cols.size()-2]);
            t.lev = stoi(cols[cols.size()-3]);
            t.nrm = stoi(cols[cols.size()-4]);
            t.ref = stoi(cols[cols.size()-5]);
            t.type = (DataType)stoi(cols[cols.size()-6]);
            st.tab[idx] = t;
        }
        i++;
    }

    // 2. Parse btab:
    while (i < lines.size() && trim(lines[i]) != "btab:") i++;
    if (i < lines.size()) {
        i++; // lewati btab:
        while (i < lines.size() && lines[i].find("----") == string::npos) i++; // divider
        i++; // lewati divider
        
        st.btab.clear();
        while (i < lines.size() && !trim(lines[i]).empty() && lines[i].find("atab:") == string::npos) {
            vector<string> cols = splitWhitespace(lines[i]);
            if (cols.size() >= 5) {
                BTabEntry b;
                b.blocks = stoi(cols[0]);
                b.last = stoi(cols[1]);
                b.lpar = stoi(cols[2]);
                b.psze = stoi(cols[3]);
                b.vsze = stoi(cols[4]);
                st.btab.push_back(b);
            }
            i++;
        }
    }

    // 3. Parse atab:
    while (i < lines.size() && lines[i].find("atab:") == string::npos) i++;
    if (i < lines.size()) {
        if (lines[i].find("(kosong") != string::npos) {
            i++;
        } else {
            i++;
            while (i < lines.size() && lines[i].find("----") == string::npos) i++;
            i++;
            st.atab.clear();
            while (i < lines.size() && !trim(lines[i]).empty() && lines[i].find("Decorated AST:") == string::npos) {
                vector<string> cols = splitWhitespace(lines[i]);
                if (cols.size() >= 8) {
                    ATabEntry a;
                    a.arrays = stoi(cols[0]);
                    a.xtyp = (DataType)stoi(cols[1]);
                    a.etyp = (DataType)stoi(cols[2]);
                    a.eref = stoi(cols[3]);
                    a.low = stoi(cols[4]);
                    a.high = stoi(cols[5]);
                    a.elsz = stoi(cols[6]);
                    a.size = stoi(cols[7]);
                    st.atab.push_back(a);
                }
                i++;
            }
        }
    }
}

// Regex / Pattern matching manual
ASTNode* ASTParser::parseASTNode(const string& text) {
    if (text.find("ProgramNode") != string::npos) return new ProgramNode();
    if (text.find("VarDecl") != string::npos) return new VarDeclNode();
    if (text.find("ConstDecl") != string::npos) return new ConstDeclNode();
    if (text.find("SubprogDecl") != string::npos) return new SubprogDeclNode();
    if (text.find("Assign") != string::npos) return new AssignStmtNode();
    if (text.find("IfStmt") != string::npos) return new IfStmtNode();
    if (text.find("WhileStmt") != string::npos) return new WhileStmtNode();
    if (text.find("ForStmt") != string::npos) return new ForStmtNode();
    if (text.find("FuncCall") != string::npos || text.find("writeln") != string::npos || text.find("readln") != string::npos) return new FuncCallNode();
    if (text.find("CompoundStmt") != string::npos || text.find("Block") != string::npos) return new CompoundStmtNode();
    if (text.find("BinOp") != string::npos) return new BinaryOpNode();
    if (text.find("UnOp") != string::npos) return new UnaryOpNode();
    
    // Cek jika Literal atau VarAccess
    if (text.find("\'") != string::npos || text.find("index ") != string::npos || text.find("tab_index:") != string::npos) {
        if (text.find("tab_index:") != string::npos && text.find("type:") != string::npos) {
            return new VarAccessNode();
        }
    }
    
    // Literal jika punya type tapi bukan var
    if (text.find("type:") != string::npos) return new LiteralNode();
    
    return nullptr;
}

void ASTParser::parseDecoratedAST(const vector<string>& lines, size_t& i) {
    i++; // lewati "Decorated AST:"
    
    vector<pair<int, ASTNode*>> stack;
    
    while (i < lines.size()) {
        string line = lines[i];
        if (line.find("Intermediate Code:") != string::npos || line.empty()) break;
        
        // Hitung depth berdasarkan posisi karakter huruf/angka pertama setelah grafis
        int depth = 0;
        for (int j = 0; j < line.length(); j++) {
            // Abaikan spasi dan box drawing (E2 94 XX di UTF8)
            unsigned char c = line[j];
            if (c != ' ' && c != '\t' && c != 0xE2 && c != 0x94 && c != 0x9C && c != 0x82 && c != 0x94 && c != 0x80 && c != 0x98) {
                if (isalnum(c) || c == '\'' || c == '_' || c == '<' || c == '>') {
                    depth = j;
                    break;
                }
            }
        }
        
        string text = trim(line); // Cukup untuk pengecekan pola
        ASTNode* node = parseASTNode(text);
        
        if (!node) {
            node = new DummyNode(); // Dummy node
        }

        if (stack.empty()) {
            root = dynamic_cast<ProgramNode*>(node);
            if (!root) { 
                root = new ProgramNode(); // Fallback
                delete node;
                node = root;
            }
            stack.push_back({depth, node});
        } else {
            while (!stack.empty() && stack.back().first >= depth) {
                stack.pop_back();
            }
            if (!stack.empty()) {
                ASTNode* parent = stack.back().second;
                
                if (auto prog = dynamic_cast<ProgramNode*>(parent)) {
                    if (text.find("Declarations") != string::npos) {
                        // Dummy node
                    } else if (text.find("Block") != string::npos) {
                        prog->mainBlock = node;
                    } else if (dynamic_cast<VarDeclNode*>(node) || dynamic_cast<ConstDeclNode*>(node)) {
                        prog->declarations.push_back(node);
                    }
                } 
                else if (auto comp = dynamic_cast<CompoundStmtNode*>(parent)) {
                    if (node && !dynamic_cast<DummyNode*>(node)) { 
                        comp->statements.push_back(node);
                    }
                }
                else if (auto asgn = dynamic_cast<AssignStmtNode*>(parent)) {
                    if (text.find("target ") != string::npos) asgn->left = node;
                    else if (text.find("value ") != string::npos) asgn->right = node;
                    else if (!asgn->left) asgn->left = node;
                    else asgn->right = node;
                }
                else if (auto ifst = dynamic_cast<IfStmtNode*>(parent)) {
                    if (text.find("condition ") != string::npos) ifst->condition = node;
                    else if (text.find("then ") != string::npos) ifst->thenStmt = node;
                    else if (text.find("else ") != string::npos) ifst->elseStmt = node;
                }
                else if (auto wh = dynamic_cast<WhileStmtNode*>(parent)) {
                    if (text.find("condition ") != string::npos) wh->condition = node;
                    else wh->body = node;
                }
                else if (auto bin = dynamic_cast<BinaryOpNode*>(parent)) {
                    if (text.find("BinOp") != string::npos) {
                        size_t firstQuote = text.find('\'');
                        size_t lastQuote = text.rfind('\'');
                        if (firstQuote != string::npos && lastQuote != string::npos && firstQuote != lastQuote) {
                            bin->op = text.substr(firstQuote + 1, lastQuote - firstQuote - 1);
                        }
                    }
                    if (!bin->left) bin->left = node;
                    else bin->right = node;
                }
                else if (auto fn = dynamic_cast<FuncCallNode*>(parent)) {
                    if (text.find("arg ") != string::npos) fn->args.push_back(node);
                }
            }
            stack.push_back({depth, node});
        }

        // Proses ekstraksi metadata penting
        if (text.find("tab_index:") != string::npos) {
            size_t pos = text.find("tab_index:");
            string idxStr = "";
            pos += 10;
            while (pos < text.length() && isdigit(text[pos])) idxStr += text[pos++];
            if (!idxStr.empty()) {
                if (auto vNode = dynamic_cast<VarAccessNode*>(node)) vNode->symRef = stoi(idxStr);
                if (auto vDecl = dynamic_cast<VarDeclNode*>(node)) vDecl->symRef = stoi(idxStr);
                if (auto fnNode = dynamic_cast<FuncCallNode*>(node)) fnNode->symRef = stoi(idxStr);
            }
        }
        if (auto lit = dynamic_cast<LiteralNode*>(node)) {
            size_t arrowPos = text.find(" \xE2\x86\x92"); // arrow
            if (arrowPos == string::npos) arrowPos = text.find(" ->");
            if (arrowPos == string::npos) arrowPos = text.find("\t");
            if (arrowPos != string::npos) {
                string val = text.substr(0, arrowPos);
                string prefixes[] = {"value ", "target ", "condition ", "then ", "else ", "arg ", "index ", "body "};
                for (const string& p : prefixes) {
                    size_t pPos = val.find(p);
                    if (pPos != string::npos) {
                        val = val.substr(pPos + p.length());
                        break;
                    }
                }
                while(!val.empty() && !isalnum(val[0]) && val[0] != '-' && val[0] != '\'') val.erase(0,1);
                lit->value = trim(val);
            }
        }
        if (auto bin = dynamic_cast<BinaryOpNode*>(node)) {
            size_t firstQuote = text.find('\'');
            size_t lastQuote = text.find('\'', firstQuote + 1);
            if (firstQuote != string::npos && lastQuote != string::npos) {
                bin->op = text.substr(firstQuote + 1, lastQuote - firstQuote - 1);
            }
        }
        if (auto varNode = dynamic_cast<VarAccessNode*>(node)) {
            size_t firstQuote = text.find('\'');
            size_t lastQuote = text.find('\'', firstQuote + 1);
            if (firstQuote != string::npos && lastQuote != string::npos) {
                varNode->name = text.substr(firstQuote + 1, lastQuote - firstQuote - 1);
            }
        }
        if (auto fnNode = dynamic_cast<FuncCallNode*>(node)) {
            size_t parenPos = text.find("(");
            if (parenPos != string::npos) {
                string name = text.substr(0, parenPos);
                // buang spasi dan karakter grafis
                while(!name.empty() && !isalpha(name[0])) name.erase(0,1);
                fnNode->name = name;
            }
        }

        i++;
    }
}
