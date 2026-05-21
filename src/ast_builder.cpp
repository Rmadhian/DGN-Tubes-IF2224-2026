#include "ast_builder.h"

ProgramNode* ASTBuilder::build(ParseTreeNode* root) {
    if (!root || root->label != "<program>") return nullptr;
    
    ProgramNode* progNode = new ProgramNode();
    
    for (auto child : root->children) {
        if (child->label == "<program-header>") {
            if (child->children.size() > 1) {
                progNode->name = child->children[1]->value;
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

void ASTBuilder::buildDeclarations(ParseTreeNode* declPart, vector<ASTNode*>& declList) {
    for (auto child : declPart->children) {
        if (child->label == "<var-declaration>")
            buildVarDeclaration(child, declList);
        else if (child->label == "<const-declaration>")
            buildConstDeclaration(child, declList);
        else if (child->label == "<subprogram-declaration>")
            buildSubprogramDeclaration(child, declList);
    }
}

void ASTBuilder::buildVarDeclaration(ParseTreeNode* varDecl, vector<ASTNode*>& declList) {
    // Parse tree: varsy + (<identifier-list> colon <type> semicolon)*
    for (size_t i = 1; i < varDecl->children.size(); i++) {
        if (varDecl->children[i]->label == "<identifier-list>") {
            VarDeclNode* vNode = new VarDeclNode();
            vNode->idents = extractIdentifierList(varDecl->children[i]);
            
            // Tipe ada 2 posisi setelah identifier-list (lewat colon)
            if (i + 2 < varDecl->children.size() && varDecl->children[i+2]->label == "<type>") {
                ParseTreeNode* typeNode = varDecl->children[i+2];
                if (!typeNode->children.empty()) {
                    vNode->type = stringToDataType(typeNode->children[0]->value); 
                }
            }
            declList.push_back(vNode);
        }
    }
}

void ASTBuilder::buildConstDeclaration(ParseTreeNode* constDecl, vector<ASTNode*>& declList) {
    // Parse tree: constsy + (ident eql <constant> semicolon)*
    for (size_t i = 1; i < constDecl->children.size(); i++) {
        if (constDecl->children[i]->label == "ident") {
            ConstDeclNode* cNode = new ConstDeclNode();
            cNode->name = constDecl->children[i]->value;
            
            if (i + 2 < constDecl->children.size() && constDecl->children[i+2]->label == "<constant>") {
                ParseTreeNode* valNode = constDecl->children[i+2];
                if (!valNode->children.empty()) {
                    cNode->value = valNode->children[0]->value;
                    cNode->type = stringToDataType(valNode->children[0]->label);
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
        funcNode->retType = DataType::NONE;
    } else {
        funcNode->isFunction = true;
    }

    for (auto child : subNode->children) {
        if (child->label == "ident" && funcNode->name.empty()) {
            funcNode->name = child->value;
        }
        else if (child->label == "<block>") {
            for (auto blockChild : child->children) {
                if (blockChild->label == "<declaration-part>")
                    buildDeclarations(blockChild, funcNode->params);
                else if (blockChild->label == "<compound-statement>")
                    funcNode->block = buildCompoundStatement(blockChild);
            }
        }
    }
    declList.push_back(funcNode);
}

CompoundStmtNode* ASTBuilder::buildCompoundStatement(ParseTreeNode* compStmt) {
    CompoundStmtNode* cStmtNode = new CompoundStmtNode();
    for (auto child : compStmt->children) {
        if (child->label == "<statement-list>") {
            for (auto stmtChild : child->children) {
                if (stmtChild->label == "<statement>") {
                    ASTNode* s = buildStatement(stmtChild);
                    if (s) cStmtNode->statements.push_back(s);
                }
            }
        }
    }
    return cStmtNode;
}

ASTNode* ASTBuilder::buildStatement(ParseTreeNode* stmt) {
    if (stmt->children.empty()) return nullptr;
    ParseTreeNode* realStmt = stmt->children[0];

    if (realStmt->label == "<assignment-statement>") return buildAssignment(realStmt);
    if (realStmt->label == "<if-statement>") return buildIf(realStmt);
    if (realStmt->label == "<while-statement>") return buildWhile(realStmt);
    if (realStmt->label == "<for-statement>") return buildFor(realStmt);
    if (realStmt->label == "<procedure/function-call>") return buildProcedureFunctionCall(realStmt);
    if (realStmt->label == "<compound-statement>") return buildCompoundStatement(realStmt);

    return nullptr;
}

AssignStmtNode* ASTBuilder::buildAssignment(ParseTreeNode* assignStmt) {
    AssignStmtNode* node = new AssignStmtNode();
    for (auto child : assignStmt->children) {
        if (child->label == "<variable>") node->left = buildVariable(child);
        else if (child->label == "<expression>") node->right = buildExpression(child);
    }
    return node;
}

IfStmtNode* ASTBuilder::buildIf(ParseTreeNode* ifStmt) {
    IfStmtNode* node = new IfStmtNode();
    for (size_t i = 0; i < ifStmt->children.size(); i++) {
        if (ifStmt->children[i]->label == "<expression>")
            node->condition = buildExpression(ifStmt->children[i]);
        else if (ifStmt->children[i]->label == "thensy") {
            if (i + 1 < ifStmt->children.size()) 
                node->thenStmt = buildStatement(ifStmt->children[i+1]);
        }
        else if (ifStmt->children[i]->label == "elsesy") {
            if (i + 1 < ifStmt->children.size()) 
                node->elseStmt = buildStatement(ifStmt->children[i+1]);
        }
    }
    return node;
}

WhileStmtNode* ASTBuilder::buildWhile(ParseTreeNode* whileStmt) {
    WhileStmtNode* node = new WhileStmtNode();
    for (size_t i = 0; i < whileStmt->children.size(); i++) {
        if (whileStmt->children[i]->label == "<expression>")
            node->condition = buildExpression(whileStmt->children[i]);
        else if (whileStmt->children[i]->label == "<compound-statement>")
            node->body = buildCompoundStatement(whileStmt->children[i]);
    }
    return node;
}

ForStmtNode* ASTBuilder::buildFor(ParseTreeNode* forStmt) {
    ForStmtNode* node = new ForStmtNode();
    int exprCount = 0;
    for (auto child : forStmt->children) {
        if (child->label == "ident") node->iterVar = child->value;
        else if (child->label == "downtosy") node->isDownto = true;
        else if (child->label == "tosy") node->isDownto = false;
        else if (child->label == "<expression>") {
            if (exprCount == 0) node->startExpr = buildExpression(child);
            else node->endExpr = buildExpression(child);
            exprCount++;
        }
        else if (child->label == "<compound-statement>")
            node->body = buildCompoundStatement(child);
    }
    return node;
}

FuncCallNode* ASTBuilder::buildProcedureFunctionCall(ParseTreeNode* callStmt) {
    FuncCallNode* node = new FuncCallNode();
    for (auto child : callStmt->children) {
        if (child->label == "ident")
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

ASTNode* ASTBuilder::buildExpression(ParseTreeNode* expr) {
    if (expr->children.size() == 1)
        return buildSimpleExpression(expr->children[0]);

    if (expr->children.size() == 3) {
        // simple-expr <relop> simple-expr
        BinaryOpNode* bNode = new BinaryOpNode();
        bNode->left = buildSimpleExpression(expr->children[0]);
        ParseTreeNode* op = expr->children[1];
        if (!op->children.empty()) bNode->op = op->children[0]->label; 
        bNode->right = buildSimpleExpression(expr->children[2]);
        return bNode;
    }
    return nullptr;
}

ASTNode* ASTBuilder::buildSimpleExpression(ParseTreeNode* simpleExpr) {
    if (simpleExpr->children.empty()) return nullptr;

    ASTNode* currentLeft = nullptr;
    string pendingOp = "";
    bool isFirstUnary = false;
    
    // Tangani sign unary di awal (misal: -5, +x)
    size_t startIndex = 0;
    if (simpleExpr->children[0]->label == "plus" ||
        simpleExpr->children[0]->label == "minus") {
        pendingOp = simpleExpr->children[0]->label;
        isFirstUnary = true;
        startIndex = 1;
    }
    
    for (size_t i = startIndex; i < simpleExpr->children.size(); i++) {
        ParseTreeNode* child = simpleExpr->children[i];
        if (child->label == "<term>") {
            ASTNode* termNode = buildTerm(child);
            if (isFirstUnary) {
                UnaryOpNode* uNode = new UnaryOpNode();
                uNode->op = pendingOp;
                uNode->operand = termNode;
                currentLeft = uNode;
                isFirstUnary = false;
            } else if (currentLeft == nullptr) {
                currentLeft = termNode;
            } else {
                BinaryOpNode* bNode = new BinaryOpNode();
                bNode->left = currentLeft;
                bNode->op = pendingOp;
                bNode->right = termNode;
                currentLeft = bNode;
            }
        } else if (child->label == "<additive-operator>") {
            if (!child->children.empty())
                pendingOp = child->children[0]->label;
        }
    }
    return currentLeft;
}

ASTNode* ASTBuilder::buildTerm(ParseTreeNode* term) {
    if (term->children.empty()) return nullptr;
    ASTNode* currentLeft = nullptr;
    string pendingOp = "";

    for (auto child : term->children) {
        if (child->label == "<factor>") {
            ASTNode* factorNode = buildFactor(child);
            if (currentLeft == nullptr) {
                currentLeft = factorNode;
            } else {
                BinaryOpNode* bNode = new BinaryOpNode();
                bNode->left = currentLeft;
                bNode->op = pendingOp;
                bNode->right = factorNode;
                currentLeft = bNode;
            }
        } else if (child->label == "<multiplicative-operator>") {
            if (!child->children.empty())
                pendingOp = child->children[0]->label;
        }
    }
    return currentLeft;
}

ASTNode* ASTBuilder::buildFactor(ParseTreeNode* factor) {
    if (factor->children.empty()) return nullptr;
    ParseTreeNode* first = factor->children[0];

    if (first->label == "intcon" || first->label == "realcon" || 
        first->label == "string" || first->label == "charcon") {
        LiteralNode* lit = new LiteralNode();
        lit->value = first->value;
        lit->literalType = stringToDataType(first->label);
        return lit;
    } 
    else if (first->label == "<variable>")
        return buildVariable(first);
    else if (first->label == "<procedure/function-call>")
        return buildProcedureFunctionCall(first);
    else if (first->label == "ident") {
        VarAccessNode* vNode = new VarAccessNode();
        vNode->name = first->value;
        return vNode;
    }
    else if (first->label == "notsy") {
        UnaryOpNode* uNode = new UnaryOpNode();
        uNode->op = "not";
        if (factor->children.size() > 1)
            uNode->operand = buildFactor(factor->children[1]);
        return uNode;
    }
    else if (first->label == "lparent" && factor->children.size() > 1)
        return buildExpression(factor->children[1]);

    return nullptr;
}

VarAccessNode* ASTBuilder::buildVariable(ParseTreeNode* varNode) {
    VarAccessNode* vNode = new VarAccessNode();
    for (auto child : varNode->children) {
        if (child->label == "ident")
            vNode->name = child->value;
        else if (child->label == "<component-variable>") {
            // TODO: implementasi akses array subscript dan record field
        }
    }
    return vNode;
}

vector<string> ASTBuilder::extractIdentifierList(ParseTreeNode* idList) {
    vector<string> ids;
    for (auto child : idList->children) {
        if (child->label == "ident")
            ids.push_back(child->value);
    }
    return ids;
}

DataType ASTBuilder::stringToDataType(string typeStr) {
    if (typeStr == "integer" || typeStr == "intcon") return DataType::INTEGER;
    if (typeStr == "real"    || typeStr == "realcon") return DataType::REAL;
    if (typeStr == "char"    || typeStr == "charcon") return DataType::CHAR;
    if (typeStr == "boolean") return DataType::BOOLEAN;
    if (typeStr == "string")  return DataType::STRING;
    return DataType::NOTYPE;
}