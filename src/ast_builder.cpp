#include "ast_builder.h"

// =========================================================
// Entry point: konversi parse tree <program> → AST
// =========================================================

ProgramNode* ASTBuilder::build(ParseTreeNode* root) {
    if (!root || root->label != "<program>") return nullptr;

    ProgramNode* progNode = new ProgramNode();

    for (auto child : root->children) {
        if (child->label == "<program-header>") {
            // programsy + ident + semicolon
            for (auto hc : child->children) {
                if (hc->label == "ident") {
                    progNode->name = hc->value;
                    break;
                }
            }
        }
        else if (child->label == "<declaration-part>") {
            buildDeclarations(child, progNode->declarations);
        }
        else if (child->label == "<compound-statement>") {
            progNode->mainBlock = buildCompoundStatement(child);
        }
    }
    return progNode;
}

// =========================================================
// Deklarasi
// =========================================================

void ASTBuilder::buildDeclarations(ParseTreeNode* declPart, vector<ASTNode*>& declList) {
    for (auto child : declPart->children) {
        if (child->label == "<var-declaration>")
            buildVarDeclaration(child, declList);
        else if (child->label == "<const-declaration>")
            buildConstDeclaration(child, declList);
        else if (child->label == "<subprogram-declaration>")
            buildSubprogramDeclaration(child, declList);
        // <type-declaration> bisa ditambahkan di sini jika perlu
    }
}

void ASTBuilder::buildVarDeclaration(ParseTreeNode* varDecl, vector<ASTNode*>& declList) {
    // Struktur: varsy + (<identifier-list> colon <type> semicolon)*
    for (size_t i = 1; i < varDecl->children.size(); i++) {
        if (varDecl->children[i]->label == "<identifier-list>") {
            VarDeclNode* vNode = new VarDeclNode();
            vNode->idents = extractIdentifierList(varDecl->children[i]);

            // Tipe ada 2 posisi setelah identifier-list (lewati colon)
            if (i + 2 < varDecl->children.size() &&
                varDecl->children[i+2]->label == "<type>") {
                buildTypeInfo(varDecl->children[i+2], vNode);
            }
            declList.push_back(vNode);
        }
    }
}

// Helper: ekstrak informasi tipe dari node <type>
void ASTBuilder::buildTypeInfo(ParseTreeNode* typeNode, VarDeclNode* vNode) {
    if (typeNode->children.empty()) return;

    ParseTreeNode* firstChild = typeNode->children[0];

    if (firstChild->label == "arraysy" || firstChild->label == "<array-type>") {
        // Array type: array [low..high] of elementType
        vNode->type = DataType::ARRAY;
        vNode->indexType   = DataType::INTEGER; // default
        vNode->elementType = DataType::INTEGER; // default
        vNode->lowBound    = 1;
        vNode->highBound   = 10;

        // Cari <range> dan <type> elemen di dalam <array-type>
        ParseTreeNode* arrNode = (firstChild->label == "<array-type>") ? firstChild : typeNode;
        for (auto c : arrNode->children) {
            if (c->label == "<range>" || c->label == "<subrange-type>") {
                // Ambil batas bawah dan atas
                extractRangeBounds(c, vNode->lowBound, vNode->highBound);
            }
            else if (c->label == "<type>") {
                // Tipe elemen
                if (!c->children.empty())
                    vNode->elementType = stringToDataType(c->children[0]->value);
            }
            else if (c->label == "ident") {
                // Element type sebagai ident langsung
                vNode->elementType = stringToDataType(c->value);
            }
        }

        // Jika tidak ditemukan dari child, coba scan lebih dalam
        scanArrayType(typeNode, vNode);
    }
    else {
        // Simple type: ambil langsung dari ident/keyword
        if (!firstChild->children.empty())
            vNode->type = stringToDataType(firstChild->children[0]->value);
        else
            vNode->type = stringToDataType(firstChild->value);
    }
}

// Scan rekursif untuk menemukan batas array dan tipe elemen
void ASTBuilder::scanArrayType(ParseTreeNode* node, VarDeclNode* vNode) {
    if (!node) return;
    for (auto child : node->children) {
        if (child->label == "intcon") {
            // Akan diisi saat processing range, skip di sini
        }
        else if (child->label == "<range>" || child->label == "<subrange-type>") {
            extractRangeBounds(child, vNode->lowBound, vNode->highBound);
        }
        scanArrayType(child, vNode);
    }
}

// Ekstrak batas bawah dan atas dari node range
void ASTBuilder::extractRangeBounds(ParseTreeNode* rangeNode, int& low, int& high) {
    // Struktur: intcon/charcon period period intcon/charcon
    //        atau <constant> period period <constant>
    vector<int> vals;
    for (auto c : rangeNode->children) {
        if (c->label == "intcon") {
            try { vals.push_back(stoi(c->value)); } catch (...) {}
        }
        else if (c->label == "<constant>") {
            for (auto gc : c->children) {
                if (gc->label == "intcon") {
                    try { vals.push_back(stoi(gc->value)); } catch (...) {}
                }
            }
        }
    }
    if (vals.size() >= 2) {
        low  = vals[0];
        high = vals[1];
    }
}

void ASTBuilder::buildConstDeclaration(ParseTreeNode* constDecl, vector<ASTNode*>& declList) {
    // Struktur: constsy + (ident eql <constant> semicolon)*
    for (size_t i = 1; i < constDecl->children.size(); i++) {
        if (constDecl->children[i]->label == "ident") {
            ConstDeclNode* cNode = new ConstDeclNode();
            cNode->name = constDecl->children[i]->value;

            // Lewati '=' dan ambil <constant>
            if (i + 2 < constDecl->children.size() &&
                constDecl->children[i+2]->label == "<constant>") {
                ParseTreeNode* valNode = constDecl->children[i+2];
                if (!valNode->children.empty()) {
                    cNode->value = valNode->children[0]->value;
                    cNode->type  = stringToDataType(valNode->children[0]->label);
                }
            }
            declList.push_back(cNode);
        }
    }
}

void ASTBuilder::buildSubprogramDeclaration(ParseTreeNode* subprogDecl, vector<ASTNode*>& declList) {
    if (subprogDecl->children.empty()) return;

    ParseTreeNode* subNode = subprogDecl->children[0];
    SubprogDeclNode* funcNode = new SubprogDeclNode();

    if (subNode->label == "<procedure-declaration>") {
        funcNode->isFunction = false;
        funcNode->retType    = DataType::NONE;
        buildProcedureDeclaration(subNode, funcNode);
    }
    else if (subNode->label == "<function-declaration>") {
        funcNode->isFunction = true;
        buildFunctionDeclaration(subNode, funcNode);
    }

    declList.push_back(funcNode);
}

void ASTBuilder::buildProcedureDeclaration(ParseTreeNode* procNode, SubprogDeclNode* funcNode) {
    // Struktur: proceduresy + ident + <formal-parameter-list>? + semicolon + <block> + semicolon
    for (auto child : procNode->children) {
        if (child->label == "ident" && funcNode->name.empty()) {
            funcNode->name = child->value;
        }
        else if (child->label == "<formal-parameter-list>") {
            buildFormalParams(child, funcNode->params);
        }
        else if (child->label == "<block>") {
            buildBlock(child, funcNode);
        }
    }
}

void ASTBuilder::buildFunctionDeclaration(ParseTreeNode* funcDeclNode, SubprogDeclNode* funcNode) {
    // Struktur: functionsy + ident + <formal-parameter-list>? + colon + <type> + semicolon + <block>
    bool pastColon = false;
    for (size_t i = 0; i < funcDeclNode->children.size(); i++) {
        auto child = funcDeclNode->children[i];
        if (child->label == "ident" && funcNode->name.empty()) {
            funcNode->name = child->value;
        }
        else if (child->label == "<formal-parameter-list>") {
            buildFormalParams(child, funcNode->params);
        }
        else if (child->label == "colon") {
            pastColon = true;
        }
        else if (pastColon && child->label == "<type>") {
            // Return type
            if (!child->children.empty())
                funcNode->retType = stringToDataType(child->children[0]->value);
            pastColon = false;
        }
        else if (child->label == "<block>") {
            buildBlock(child, funcNode);
        }
    }
}

void ASTBuilder::buildFormalParams(ParseTreeNode* paramList, vector<ASTNode*>& params) {
    // Struktur: lparent + (<parameter-group> semicolon)* + rparent
    for (auto child : paramList->children) {
        if (child->label == "<parameter-group>") {
            buildParameterGroup(child, params);
        }
    }
}

void ASTBuilder::buildParameterGroup(ParseTreeNode* paramGroup, vector<ASTNode*>& params) {
    // Struktur: (varsy)? + <identifier-list> + colon + <type>
    VarDeclNode* vNode = new VarDeclNode();
    bool isByRef = false;

    for (size_t i = 0; i < paramGroup->children.size(); i++) {
        auto child = paramGroup->children[i];
        if (child->label == "varsy") {
            isByRef = true; // pass-by-reference (nrm=0)
        }
        else if (child->label == "<identifier-list>") {
            vNode->idents = extractIdentifierList(child);
        }
        else if (child->label == "<type>") {
            buildTypeInfo(child, vNode);
        }
    }

    // nrm akan diset saat dimasukkan ke symbol table
    // Kita simpan info by-ref dalam sebuah flag terpisah di VarDeclNode
    // (untuk kesederhanaan: simpan di ref sebagai -1 jika by-ref)
    if (isByRef) vNode->ref = -1; // flag: by-ref parameter

    params.push_back(vNode);
}

void ASTBuilder::buildBlock(ParseTreeNode* blockNode, SubprogDeclNode* funcNode) {
    for (auto child : blockNode->children) {
        if (child->label == "<declaration-part>")
            buildDeclarations(child, funcNode->params);
        else if (child->label == "<compound-statement>")
            funcNode->block = buildCompoundStatement(child);
    }
}

// =========================================================
// Statement
// =========================================================

CompoundStmtNode* ASTBuilder::buildCompoundStatement(ParseTreeNode* compStmt) {
    CompoundStmtNode* cNode = new CompoundStmtNode();
    for (auto child : compStmt->children) {
        if (child->label == "<statement-list>") {
            for (auto stmtChild : child->children) {
                if (stmtChild->label == "<statement>") {
                    ASTNode* s = buildStatement(stmtChild);
                    if (s) cNode->statements.push_back(s);
                }
            }
        }
    }
    return cNode;
}

ASTNode* ASTBuilder::buildStatement(ParseTreeNode* stmt) {
    if (stmt->children.empty()) return nullptr; // empty statement

    ParseTreeNode* realStmt = stmt->children[0];

    if (realStmt->label == "<assignment-statement>")    return buildAssignment(realStmt);
    if (realStmt->label == "<if-statement>")            return buildIf(realStmt);
    if (realStmt->label == "<while-statement>")         return buildWhile(realStmt);
    if (realStmt->label == "<for-statement>")           return buildFor(realStmt);
    if (realStmt->label == "<procedure/function-call>") return buildProcedureFunctionCall(realStmt);
    if (realStmt->label == "<compound-statement>")      return buildCompoundStatement(realStmt);
    if (realStmt->label == "<repeat-statement>")        return buildRepeat(realStmt);

    return nullptr;
}

AssignStmtNode* ASTBuilder::buildAssignment(ParseTreeNode* assignStmt) {
    AssignStmtNode* node = new AssignStmtNode();

    // Cek apakah target adalah <variable> atau ident langsung
    for (size_t i = 0; i < assignStmt->children.size(); i++) {
        auto child = assignStmt->children[i];
        if (child->label == "<variable>") {
            node->left = buildVariable(child);
        }
        else if (child->label == "ident" && node->left == nullptr) {
            // Target bisa berupa ident langsung (tanpa wrapper <variable>)
            VarAccessNode* v = new VarAccessNode();
            v->name = child->value;
            node->left = v;
        }
        else if (child->label == "<expression>") {
            node->right = buildExpression(child);
        }
    }
    return node;
}

IfStmtNode* ASTBuilder::buildIf(ParseTreeNode* ifStmt) {
    IfStmtNode* node = new IfStmtNode();
    // Struktur: ifsy + <expression> + thensy + <statement> + (elsesy + <statement>)?
    for (size_t i = 0; i < ifStmt->children.size(); i++) {
        auto child = ifStmt->children[i];
        if (child->label == "<expression>") {
            node->condition = buildExpression(child);
        }
        else if (child->label == "thensy") {
            if (i + 1 < ifStmt->children.size())
                node->thenStmt = buildStatement(ifStmt->children[i+1]);
        }
        else if (child->label == "elsesy") {
            if (i + 1 < ifStmt->children.size())
                node->elseStmt = buildStatement(ifStmt->children[i+1]);
        }
    }
    return node;
}

WhileStmtNode* ASTBuilder::buildWhile(ParseTreeNode* whileStmt) {
    WhileStmtNode* node = new WhileStmtNode();
    // Spek revisi: whilesy + expression + dosy + compound-statement + semicolon
    for (size_t i = 0; i < whileStmt->children.size(); i++) {
        auto child = whileStmt->children[i];
        if (child->label == "<expression>")
            node->condition = buildExpression(child);
        else if (child->label == "<compound-statement>")
            node->body = buildCompoundStatement(child);
    }
    return node;
}

ForStmtNode* ASTBuilder::buildFor(ParseTreeNode* forStmt) {
    ForStmtNode* node = new ForStmtNode();
    // Spek revisi: forsy + ident + becomes + expression + (tosy|downtosy) + expression + dosy + compound-statement + semicolon
    int exprCount = 0;
    for (auto child : forStmt->children) {
        if (child->label == "ident")
            node->iterVar = child->value;
        else if (child->label == "downtosy")
            node->isDownto = true;
        else if (child->label == "tosy")
            node->isDownto = false;
        else if (child->label == "<expression>") {
            if (exprCount == 0) node->startExpr = buildExpression(child);
            else                node->endExpr   = buildExpression(child);
            exprCount++;
        }
        else if (child->label == "<compound-statement>")
            node->body = buildCompoundStatement(child);
    }
    return node;
}

// Repeat-until (bonus: tidak ada di spek utama, tapi agar tidak crash)
ASTNode* ASTBuilder::buildRepeat(ParseTreeNode* repeatStmt) {
    WhileStmtNode* node = new WhileStmtNode(); // Representasi dengan WhileNode
    for (auto child : repeatStmt->children) {
        if (child->label == "<expression>")
            node->condition = buildExpression(child);
        else if (child->label == "<statement-list>") {
            CompoundStmtNode* body = new CompoundStmtNode();
            for (auto sc : child->children) {
                if (sc->label == "<statement>") {
                    ASTNode* s = buildStatement(sc);
                    if (s) body->statements.push_back(s);
                }
            }
            node->body = body;
        }
    }
    return node;
}

FuncCallNode* ASTBuilder::buildProcedureFunctionCall(ParseTreeNode* callStmt) {
    FuncCallNode* node = new FuncCallNode();
    for (auto child : callStmt->children) {
        if (child->label == "ident" && node->name.empty())
            node->name = child->value;
        else if (child->label == "<parameter-list>") {
            for (auto param : child->children) {
                if (param->label == "<expression>")
                    node->args.push_back(buildExpression(param));
            }
        }
    }
    return node;
}

// =========================================================
// Ekspresi
// =========================================================

ASTNode* ASTBuilder::buildExpression(ParseTreeNode* expr) {
    if (expr->children.size() == 1)
        return buildSimpleExpression(expr->children[0]);

    if (expr->children.size() == 3) {
        // <simple-expression> <relop> <simple-expression>
        BinaryOpNode* bNode = new BinaryOpNode();
        bNode->left  = buildSimpleExpression(expr->children[0]);
        ParseTreeNode* relop = expr->children[1];
        // Ambil operator dari child relop
        if (!relop->children.empty())
            bNode->op = relop->children[0]->label;
        else
            bNode->op = relop->label;
        bNode->right = buildSimpleExpression(expr->children[2]);
        return bNode;
    }
    return nullptr;
}

ASTNode* ASTBuilder::buildSimpleExpression(ParseTreeNode* simpleExpr) {
    if (simpleExpr->children.empty()) return nullptr;

    ASTNode* currentLeft = nullptr;
    string pendingOp     = "";
    bool isFirstUnary    = false;
    size_t startIndex    = 0;

    // Tangani tanda unary di awal (+/-) 
    if (simpleExpr->children[0]->label == "plus" ||
        simpleExpr->children[0]->label == "minus") {
        pendingOp     = simpleExpr->children[0]->label;
        isFirstUnary  = true;
        startIndex    = 1;
    }

    for (size_t i = startIndex; i < simpleExpr->children.size(); i++) {
        ParseTreeNode* child = simpleExpr->children[i];
        if (child->label == "<term>") {
            ASTNode* termNode = buildTerm(child);
            if (isFirstUnary) {
                UnaryOpNode* uNode = new UnaryOpNode();
                uNode->op      = pendingOp;
                uNode->operand = termNode;
                currentLeft    = uNode;
                isFirstUnary   = false;
            } else if (currentLeft == nullptr) {
                currentLeft = termNode;
            } else {
                BinaryOpNode* bNode = new BinaryOpNode();
                bNode->left  = currentLeft;
                bNode->op    = pendingOp;
                bNode->right = termNode;
                currentLeft  = bNode;
            }
        }
        else if (child->label == "<additive-operator>") {
            if (!child->children.empty())
                pendingOp = child->children[0]->label;
        }
    }
    return currentLeft;
}

ASTNode* ASTBuilder::buildTerm(ParseTreeNode* term) {
    if (term->children.empty()) return nullptr;
    ASTNode* currentLeft = nullptr;
    string pendingOp     = "";

    for (auto child : term->children) {
        if (child->label == "<factor>") {
            ASTNode* factorNode = buildFactor(child);
            if (currentLeft == nullptr) {
                currentLeft = factorNode;
            } else {
                BinaryOpNode* bNode = new BinaryOpNode();
                bNode->left  = currentLeft;
                bNode->op    = pendingOp;
                bNode->right = factorNode;
                currentLeft  = bNode;
            }
        }
        else if (child->label == "<multiplicative-operator>") {
            if (!child->children.empty())
                pendingOp = child->children[0]->label;
        }
    }
    return currentLeft;
}

ASTNode* ASTBuilder::buildFactor(ParseTreeNode* factor) {
    if (factor->children.empty()) return nullptr;
    ParseTreeNode* first = factor->children[0];

    // Literal
    if (first->label == "intcon" || first->label == "realcon" ||
        first->label == "string" || first->label == "charcon") {
        LiteralNode* lit   = new LiteralNode();
        lit->value       = first->value;
        lit->literalType = stringToDataType(first->label);
        return lit;
    }

    // Variabel dengan kemungkinan subscript array
    if (first->label == "<variable>")
        return buildVariable(first);

    // Pemanggilan fungsi/prosedur
    if (first->label == "<procedure/function-call>")
        return buildProcedureFunctionCall(first);

    // Identifier tanpa subscript
    if (first->label == "ident") {
        VarAccessNode* vNode = new VarAccessNode();
        vNode->name = first->value;
        return vNode;
    }

    // Operator not
    if (first->label == "notsy") {
        UnaryOpNode* uNode = new UnaryOpNode();
        uNode->op = "not";
        if (factor->children.size() > 1)
            uNode->operand = buildFactor(factor->children[1]);
        return uNode;
    }

    // Ekspresi dalam kurung: lparent + <expression> + rparent
    if (first->label == "lparent" && factor->children.size() > 1)
        return buildExpression(factor->children[1]);

    return nullptr;
}

VarAccessNode* ASTBuilder::buildVariable(ParseTreeNode* varNode) {
    VarAccessNode* vNode = new VarAccessNode();
    for (auto child : varNode->children) {
        if (child->label == "ident" && vNode->name.empty())
            vNode->name = child->value;
        else if (child->label == "<component-variable>") {
            // Array subscript atau record field access
            for (auto compChild : child->children) {
                if (compChild->label == "<expression>")
                    vNode->indices.push_back(buildExpression(compChild));
                else if (compChild->label == "ident" && !vNode->name.empty())
                    vNode->fieldName = compChild->value; // record.field
            }
        }
        else if (child->label == "<index-list>") {
            for (auto idxChild : child->children) {
                if (idxChild->label == "<expression>")
                    vNode->indices.push_back(buildExpression(idxChild));
            }
        }
    }
    return vNode;
}

// =========================================================
// Helper utilities
// =========================================================

vector<string> ASTBuilder::extractIdentifierList(ParseTreeNode* idList) {
    vector<string> ids;
    for (auto child : idList->children) {
        if (child->label == "ident")
            ids.push_back(child->value);
    }
    return ids;
}

DataType ASTBuilder::stringToDataType(const string& typeStr) {
    // Konversi case-insensitive
    string lower = typeStr;
    for (char& c : lower) c = tolower(c);

    if (lower == "integer" || lower == "intcon") return DataType::INTEGER;
    if (lower == "real"    || lower == "realcon") return DataType::REAL;
    if (lower == "char"    || lower == "charcon") return DataType::CHAR;
    if (lower == "boolean")                       return DataType::BOOLEAN;
    if (lower == "string")                        return DataType::STRING;
    if (lower == "array"   || lower == "arraysy") return DataType::ARRAY;
    if (lower == "record"  || lower == "recordsy")return DataType::RECORD;
    return DataType::NOTYPE;
}