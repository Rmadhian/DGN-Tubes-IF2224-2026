#include "ast_parser.h"
#include <sstream>
#include <algorithm>


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
        if (line == "Decorated AST:" || line.find("ProgramNode") != string::npos) {
            if (line == "Decorated AST:") {
                parseDecoratedAST(lines, i);
            } else {
                size_t start = i;
                parseDecoratedAST(lines, start);
                i = start;
            }
        } else {
            i++;
        }
    }

    // Bangun symbol table dari tree
    buildSymbolTableFromTree();
}

// Pattern matching untuk node AST (support format internal dan format spek)
ASTNode* ASTParser::parseASTNode(const string& text) {
    // Format internal: ProgramNode, VarDecl, AssignStmt, etc.
    if (text.find("ProgramNode") != string::npos) return new ProgramNode();
    if (text.find("VarDecl") != string::npos) return new VarDeclNode();
    if (text.find("ConstDecl") != string::npos) return new ConstDeclNode();
    if (text.find("SubprogDecl") != string::npos) return new SubprogDeclNode();
    if (text.find("Assign") != string::npos) return new AssignStmtNode();
    if (text.find("IfSt") != string::npos) return new IfStmtNode();
    if (text.find("WhileSt") != string::npos) return new WhileStmtNode();
    if (text.find("ForSt") != string::npos) return new ForStmtNode();
    if (text.find("FuncCall") != string::npos || text.find("writeln") != string::npos || text.find("readln") != string::npos) return new FuncCallNode();
    if (text.find("CompoundStmt") != string::npos || text.find("CompoundBlock") != string::npos || text.find("Block") != string::npos) return new CompoundStmtNode();
    if (text.find("BinOp") != string::npos) {
        auto bin = new BinaryOpNode();
        size_t firstQuote = text.find('\'');
        size_t lastQuote = text.rfind('\'');
        if (firstQuote != string::npos && lastQuote != string::npos && firstQuote != lastQuote) {
            bin->op = text.substr(firstQuote + 1, lastQuote - firstQuote - 1);
        }
        return bin;
    }
    if (text.find("UnOp") != string::npos || text.find("UnaryOp") != string::npos) {
        auto un = new UnaryOpNode();
        size_t firstQuote = text.find('\'');
        size_t lastQuote = text.rfind('\'');
        if (firstQuote != string::npos && lastQuote != string::npos && firstQuote != lastQuote) {
            un->op = text.substr(firstQuote + 1, lastQuote - firstQuote - 1);
        }
        return un;
    }
    
    // Format internal: cek tab_index untuk VarAccess
    if (text.find("tab_index:") != string::npos && text.find("type:") != string::npos) {
        return new VarAccessNode();
    }
    if (text.find("\'") != string::npos && text.find("type:") != string::npos) {
        // Bisa jadi literal string atau var access
        if (text.find("tab_index:") != string::npos) return new VarAccessNode();
    }
    
    // Literal jika punya type tapi bukan var
    if (text.find("type:") != string::npos) return new LiteralNode();
    
    return nullptr;
}

void ASTParser::parseDecoratedAST(const vector<string>& lines, size_t& i) {
    if (lines[i].find("Decorated AST:") != string::npos) {
        i++; // lewati "Decorated AST:"
    }
    
    vector<pair<int, ASTNode*>> stack;
    
    while (i < lines.size()) {
        string line = lines[i];
        if (line.find("Intermediate Code:") != string::npos) break;
        if (trim(line).empty()) {
            i++;
            continue;
        }
        
        // Hitung depth secara logis (UTF-8 safe)
        int depth = 0;
        int bytePos = 0;
        while (bytePos < (int)line.length()) {
            unsigned char c = line[bytePos];
            if (isalnum(c) || c == '\'' || c == '_' || c == '<' || c == '>') break;
            
            if ((c & 0xE0) == 0xE0) bytePos += 3;
            else if ((c & 0xC0) == 0xC0) bytePos += 2;
            else bytePos += 1;
            depth++;
        }
        
        string text = trim(line.substr(bytePos)); // Cukup untuk pengecekan pola
        ASTNode* node = parseASTNode(text);
        
        if (!node) {
            i++;
            continue; // Skip decorative nodes like 'Declarations'
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
                    if (text.find("Block") != string::npos) {
                        prog->mainBlock = node;
                    } else if (dynamic_cast<VarDeclNode*>(node) || dynamic_cast<ConstDeclNode*>(node) || dynamic_cast<SubprogDeclNode*>(node)) {
                        prog->declarations.push_back(node);
                    }
                } 
                else if (auto comp = dynamic_cast<CompoundStmtNode*>(parent)) {
                    if (node) { 
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
                    if (!bin->left) bin->left = node;
                    else bin->right = node;
                }
                else if (auto un = dynamic_cast<UnaryOpNode*>(parent)) {
                    un->operand = node;
                }
                else if (auto fn = dynamic_cast<FuncCallNode*>(parent)) {
                    if (text.find("arg ") != string::npos || text.find("target ") != string::npos) fn->args.push_back(node);
                }
                else if (auto fs = dynamic_cast<ForStmtNode*>(parent)) {
                    if (text.find("start ") != string::npos) fs->startExpr = node;
                    else if (text.find("end ") != string::npos) fs->endExpr = node;
                    else if (text.find("body ") != string::npos) fs->body = node;
                    else {
                        // asumsi iterVar
                        if (auto v = dynamic_cast<VarAccessNode*>(node)) fs->iterVar = v->name;
                    }
                }
                else if (auto subp = dynamic_cast<SubprogDeclNode*>(parent)) {
                    if (text.find("param ") != string::npos || dynamic_cast<VarDeclNode*>(node)) subp->params.push_back(node);
                    else if (text.find("block") != string::npos || text.find("Block") != string::npos) subp->block = node;
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
                int symIdx = stoi(idxStr);
                if (auto vNode = dynamic_cast<VarAccessNode*>(node)) {
                    vNode->symRef = symIdx;
                    if (symIdx >= 0 && symIdx < (int)st.tab.size()) vNode->lexicalLevel = st.tab[symIdx].lev;
                }
                if (auto vDecl = dynamic_cast<VarDeclNode*>(node)) {
                    vDecl->symRef = symIdx;
                    if (symIdx >= 0 && symIdx < (int)st.tab.size()) vDecl->lexicalLevel = st.tab[symIdx].lev;
                }
                if (auto fnNode = dynamic_cast<FuncCallNode*>(node)) fnNode->symRef = symIdx;
            }
        }
        
        // Ekstrak lev (Lexical Level) jika ada tertulis eksplisit
        if (text.find("lev:") != string::npos) {
            size_t pos = text.find("lev:");
            string levStr = "";
            pos += 4;
            while (pos < text.length() && isdigit(text[pos])) levStr += text[pos++];
            if (!levStr.empty()) {
                node->lexicalLevel = stoi(levStr);
            }
        }

        if (auto lit = dynamic_cast<LiteralNode*>(node)) {
            size_t arrowPos = text.find(" \xE2\x86\x92"); // arrow
            if (arrowPos == string::npos) arrowPos = text.find(" ->");
            if (arrowPos == string::npos) arrowPos = text.find("\t");
            if (arrowPos != string::npos) {
                string val = text.substr(0, arrowPos);
                string prefixes[] = {"value ", "target ", "condition ", "then ", "else ", "arg ", "index ", "body ", "Literal: "};
                for (const string& p : prefixes) {
                    size_t pPos = val.find(p);
                    if (pPos != string::npos) {
                        val = val.substr(pPos + p.length());
                        break;
                    }
                }
                while(!val.empty() && !isalnum(val[0]) && val[0] != '-' && val[0] != '\'') val.erase(0,1);
                lit->value = trim(val);
            } else if (text.find("Literal: ") != string::npos) {
                lit->value = trim(text.substr(text.find("Literal: ") + 9));
            }
            
            if (text.find("type:integer") != string::npos || text.find("Type: integer") != string::npos) lit->literalType = DataType::INTEGER;
            else if (text.find("type:real") != string::npos || text.find("Type: real") != string::npos) lit->literalType = DataType::REAL;
            else if (text.find("type:boolean") != string::npos || text.find("Type: boolean") != string::npos) lit->literalType = DataType::BOOLEAN;
            else if (text.find("type:char") != string::npos || text.find("Type: char") != string::npos) lit->literalType = DataType::CHAR;
            else if (text.find("type:string") != string::npos || text.find("Type: string") != string::npos) lit->literalType = DataType::STRING;
        }
        if (auto bin = dynamic_cast<BinaryOpNode*>(node)) {
            size_t firstQuote = text.find('\'');
            size_t lastQuote = text.find('\'', firstQuote + 1);
            if (firstQuote != string::npos && lastQuote != string::npos) {
                bin->op = text.substr(firstQuote + 1, lastQuote - firstQuote - 1);
            }
        }
        if (auto varDecl = dynamic_cast<VarDeclNode*>(node)) {
            size_t firstColon = text.find(":");
            size_t secondColon = text.find(":", firstColon + 1);
            if (text.find("VarDecl: ") != string::npos && secondColon != string::npos) {
                string name = text.substr(firstColon + 1, secondColon - firstColon - 1);
                name = trim(name);
                varDecl->idents.push_back(name);
                varDeclNames.push_back(name);
            } else {
                size_t firstQuote = text.find('\'');
                size_t lastQuote = text.find('\'', firstQuote + 1);
                if (firstQuote != string::npos && lastQuote != string::npos) {
                    string name = text.substr(firstQuote + 1, lastQuote - firstQuote - 1);
                    varDecl->idents.push_back(name);
                    varDeclNames.push_back(name);
                }
            }
            if (text.find("type:integer") != string::npos || text.find("Type: integer") != string::npos) varDecl->type = DataType::INTEGER;
            else if (text.find("type:real") != string::npos || text.find("Type: real") != string::npos) varDecl->type = DataType::REAL;
            else if (text.find("type:boolean") != string::npos || text.find("Type: boolean") != string::npos) varDecl->type = DataType::BOOLEAN;
            else if (text.find("type:char") != string::npos || text.find("Type: char") != string::npos) varDecl->type = DataType::CHAR;
            else if (text.find("type:string") != string::npos || text.find("Type: string") != string::npos) varDecl->type = DataType::STRING;
        }
        if (auto varNode = dynamic_cast<VarAccessNode*>(node)) {
            size_t firstQuote = text.find('\'');
            size_t lastQuote = text.find('\'', firstQuote + 1);
            if (firstQuote != string::npos && lastQuote != string::npos) {
                varNode->name = text.substr(firstQuote + 1, lastQuote - firstQuote - 1);
            } else if (text.find("Identifier: ") != string::npos) {
                size_t arrowPos = text.find(" -->");
                if (arrowPos == string::npos) arrowPos = text.find(" ->");
                if (arrowPos == string::npos) arrowPos = text.find(" \xE2\x86\x92");
                if (arrowPos != string::npos) {
                    varNode->name = trim(text.substr(text.find("Identifier: ") + 12, arrowPos - (text.find("Identifier: ") + 12)));
                } else {
                    varNode->name = trim(text.substr(text.find("Identifier: ") + 12));
                }
            }
        }
        if (auto fnNode = dynamic_cast<FuncCallNode*>(node)) {
            size_t parenPos = text.find("(");
            if (parenPos != string::npos) {
                string name = text.substr(0, parenPos);
                string prefixes[] = {"condition ", "then ", "else ", "body "};
                for (const string& p : prefixes) {
                    size_t pPos = name.find(p);
                    if (pPos != string::npos) {
                        name = name.substr(pPos + p.length());
                        break;
                    }
                }
                // buang spasi dan karakter grafis
                while(!name.empty() && !isalpha(name[0])) name.erase(0,1);
                fnNode->name = name;
            }
        }

        i++;
    }
}

void ASTParser::buildSymbolTableFromTree() {
    // Kosongkan tab, btab, atab
    st.tab.clear();
    st.btab.clear();
    st.atab.clear();

    // Inisialisasi predefined identifiers
    st.initPredefined();

    // reset tab supaya yang sisa cuma built-in (0..39)
    while (st.tab.size() > 40) {
        st.tab.pop_back();
    }

    // Alamat variabel dimulai dari offset 3 (karena 0=SL, 1=DL, 2=RA)
    int currentAdr = 3;
    
    for (size_t i = 0; i < varDeclNames.size(); i++) {
        TabEntry t;
        t.identifiers = varDeclNames[i];
        t.obj = ObjClass::VARIABLE;
        t.type = DataType::INTEGER; // Default, karena kita tidak parsing tipe secara ketat di sini jika tidak ada
        t.ref = 0;
        t.nrm = 1;
        t.lev = 0;
        t.adr = currentAdr++;
        t.link = 0;
        st.tab.push_back(t);
    }

    // Resolusi symRef untuk VarAccessNode dan VarDeclNode yang tidak punya index
    // DFS traversal sederhana untuk assign symRef
    vector<ASTNode*> stack;
    if (root) stack.push_back(root);
    
    while (!stack.empty()) {
        ASTNode* curr = stack.back();
        stack.pop_back();
        
        if (auto vNode = dynamic_cast<VarAccessNode*>(curr)) {
            for (size_t k = 33; k < st.tab.size(); k++) {
                if (st.tab[k].identifiers == vNode->name) {
                    vNode->symRef = k;
                    vNode->lexicalLevel = st.tab[k].lev;
                    break;
                }
            }
        }
        else if (auto asgn = dynamic_cast<AssignStmtNode*>(curr)) {
            if (asgn->left) stack.push_back(asgn->left);
            if (asgn->right) stack.push_back(asgn->right);
        }
        else if (auto bin = dynamic_cast<BinaryOpNode*>(curr)) {
            if (bin->left) stack.push_back(bin->left);
            if (bin->right) stack.push_back(bin->right);
        }
        else if (auto un = dynamic_cast<UnaryOpNode*>(curr)) {
            if (un->operand) stack.push_back(un->operand);
        }
        else if (auto comp = dynamic_cast<CompoundStmtNode*>(curr)) {
            for (auto* stmt : comp->statements) stack.push_back(stmt);
        }
        else if (auto ifst = dynamic_cast<IfStmtNode*>(curr)) {
            if (ifst->condition) stack.push_back(ifst->condition);
            if (ifst->thenStmt) stack.push_back(ifst->thenStmt);
            if (ifst->elseStmt) stack.push_back(ifst->elseStmt);
        }
        else if (auto wh = dynamic_cast<WhileStmtNode*>(curr)) {
            if (wh->condition) stack.push_back(wh->condition);
            if (wh->body) stack.push_back(wh->body);
        }
        else if (auto fs = dynamic_cast<ForStmtNode*>(curr)) {
            if (fs->startExpr) stack.push_back(fs->startExpr);
            if (fs->endExpr) stack.push_back(fs->endExpr);
            if (fs->body) stack.push_back(fs->body);
        }
        else if (auto wrt = dynamic_cast<WriteStatementNode*>(curr)) {
            for (auto* arg : wrt->args) stack.push_back(arg);
        }
        else if (auto fn = dynamic_cast<FuncCallNode*>(curr)) {
            for (auto* arg : fn->args) stack.push_back(arg);
        }
        else if (auto prog = dynamic_cast<ProgramNode*>(curr)) {
            if (prog->mainBlock) stack.push_back(prog->mainBlock);
        }
    }

    // Init 1 block di btab (Program block)
    BTabEntry b;
    b.blocks = 0;
    b.last = st.tab.size() - 1;
    b.lpar = 0;
    b.psze = 0;
    b.vsze = varDeclNames.size(); // Hanya jumlah variabel, +3 (SL/DL/RA) akan ditambahkan saat instruksi INT
    st.btab.push_back(b);
    
    // Main block
    BTabEntry b2 = b;
    b2.vsze = 0; // blok utama biasanya vsze-nya menumpang program block di arsitektur kita
    st.btab.push_back(b2);
}
