#include "parser.h"

// block -> declaration-part + compound-statement
ParseTreeNode* Parser::parseBlock() {
    ParseTreeNode* node = new ParseTreeNode("<block>");
    node->children.push_back(parseDeclarationPart());
    if (hasError) return node;
    node->children.push_back(parseCompoundStatement());
    return node;
}

// <compound-statement> -> beginsy + statement-list + endsy
ParseTreeNode* Parser::parseCompoundStatement() {
    ParseTreeNode* node = new ParseTreeNode("<compound-statement>");

    if (currentToken().type == TokenType::BEGINSY) {
        node->children.push_back(match(TokenType::BEGINSY));
    } else {
        reportError("beginsy", currentToken().value);
        return node;
    }

    node->children.push_back(parseStatementList());
    if (hasError) return node;

    if (currentToken().type == TokenType::ENDSY) {
        node->children.push_back(match(TokenType::ENDSY));
    } else {
        reportError("endsy", currentToken().value);
    }

    return node;
}

// <statement-list> -> statement (semicolon + statement)*
ParseTreeNode* Parser::parseStatementList() {
    ParseTreeNode* node = new ParseTreeNode("<statement-list>");

    node->children.push_back(parseStatement());

    while (!hasError && currentToken().type == TokenType::SEMICOLON) {
        node->children.push_back(match(TokenType::SEMICOLON));
        node->children.push_back(parseStatement());
    }

    return node;
}

// <statement> -> ( assignment | if | case | while | repeat | for | proc/func-call )?
ParseTreeNode* Parser::parseStatement() {
    ParseTreeNode* node = new ParseTreeNode("<statement>");

    Token tok = currentToken();
    switch (tok.type) {
        case TokenType::IDENT: {
            // Bedakan assignment vs procedure/function call.
            // Variable bisa diikuti '[' (array) atau '.' (field record) sebelum ':='.
            // Procedure/function-call paling banyak punya '(' setelah ident.
            Token next = peek(1);
            if (next.type == TokenType::BECOMES ||
                next.type == TokenType::LBRACK ||
                next.type == TokenType::PERIOD) {
                node->children.push_back(parseAssignmentStatement());
            } else {
                node->children.push_back(parseProcedureFunctionCall());
            }
            break;
        }
        case TokenType::IFSY:
            node->children.push_back(parseIfStatement());
            break;
        case TokenType::CASESY:
            node->children.push_back(parseCaseStatement());
            break;
        case TokenType::WHILESY:
            node->children.push_back(parseWhileStatement());
            break;
        case TokenType::REPEATSY:
            node->children.push_back(parseRepeatStatement());
            break;
        case TokenType::FORSY:
            node->children.push_back(parseForStatement());
            break;
        case TokenType::BEGINSY:
            node->children.push_back(parseCompoundStatement());
            break;
        default:
            // Empty statement (diizinkan oleh aturan produksi yang berakhiran '?')
            break;
    }

    return node;
}

// <assignment-statement> -> variable + becomes + expression
ParseTreeNode* Parser::parseAssignmentStatement() {
    ParseTreeNode* node = new ParseTreeNode("<assignment-statement>");

    node->children.push_back(parseVariable());
    if (hasError) return node;

    if (currentToken().type == TokenType::BECOMES) {
        node->children.push_back(match(TokenType::BECOMES));
    } else {
        reportError("becomes", currentToken().value);
        return node;
    }

    node->children.push_back(parseExpression());
    return node;
}

// <variable> -> ident + (component-variable)*
ParseTreeNode* Parser::parseVariable() {
    ParseTreeNode* node = new ParseTreeNode("<variable>");

    if (currentToken().type == TokenType::IDENT) {
        node->children.push_back(match(TokenType::IDENT));
    } else {
        reportError("ident", currentToken().value);
        return node;
    }

    while (!hasError && (currentToken().type == TokenType::LBRACK ||
                         currentToken().type == TokenType::PERIOD)) {
        node->children.push_back(parseComponentVariable());
    }

    return node;
}

// <component-variable> -> (lbrack + index-list + rbrack) | (period + ident)
ParseTreeNode* Parser::parseComponentVariable() {
    ParseTreeNode* node = new ParseTreeNode("<component-variable>");

    if (currentToken().type == TokenType::LBRACK) {
        node->children.push_back(match(TokenType::LBRACK));
        node->children.push_back(parseIndexList());
        if (hasError) return node;

        if (currentToken().type == TokenType::RBRACK) {
            node->children.push_back(match(TokenType::RBRACK));
        } else {
            reportError("rbrack", currentToken().value);
        }
    } else if (currentToken().type == TokenType::PERIOD) {
        node->children.push_back(match(TokenType::PERIOD));
        if (currentToken().type == TokenType::IDENT) {
            node->children.push_back(match(TokenType::IDENT));
        } else {
            reportError("ident", currentToken().value);
        }
    } else {
        reportError("lbrack atau period", currentToken().value);
    }

    return node;
}

// <index-list> -> ( intcon | charcon | ident ) + ( comma + index-list )*
ParseTreeNode* Parser::parseIndexList() {
    ParseTreeNode* node = new ParseTreeNode("<index-list>");

    Token tok = currentToken();
    if (tok.type == TokenType::INTCON ||
        tok.type == TokenType::CHARCON ||
        tok.type == TokenType::IDENT) {
        node->children.push_back(match(tok.type));
    } else {
        reportError("intcon, charcon, atau ident", currentToken().value);
        return node;
    }

    while (!hasError && currentToken().type == TokenType::COMMA) {
        node->children.push_back(match(TokenType::COMMA));
        node->children.push_back(parseIndexList());
    }

    return node;
}

// <subprogram-declaration> -> procedure-declaration | function-declaration
ParseTreeNode* Parser::parseSubprogramDeclaration() {
    ParseTreeNode* node = new ParseTreeNode("<subprogram-declaration>");

    if (currentToken().type == TokenType::PROCEDURESY) {
        node->children.push_back(parseProcedureDeclaration());
    } else if (currentToken().type == TokenType::FUNCTIONSY) {
        node->children.push_back(parseFunctionDeclaration());
    } else {
        reportError("proceduresy atau functionsy", currentToken().value);
    }

    return node;
}

// <procedure-declaration> ->
//     proceduresy + ident + (formal-parameter-list)? + semicolon + block + semicolon
ParseTreeNode* Parser::parseProcedureDeclaration() {
    ParseTreeNode* node = new ParseTreeNode("<procedure-declaration>");

    node->children.push_back(match(TokenType::PROCEDURESY));

    if (currentToken().type == TokenType::IDENT) {
        node->children.push_back(match(TokenType::IDENT));
    } else {
        reportError("ident (nama prosedur)", currentToken().value);
        return node;
    }

    if (currentToken().type == TokenType::LPARENT) {
        node->children.push_back(parseFormalParameterList());
        if (hasError) return node;
    }

    if (currentToken().type == TokenType::SEMICOLON) {
        node->children.push_back(match(TokenType::SEMICOLON));
    } else {
        reportError("semicolon", currentToken().value);
        return node;
    }

    node->children.push_back(parseBlock());
    if (hasError) return node;

    if (currentToken().type == TokenType::SEMICOLON) {
        node->children.push_back(match(TokenType::SEMICOLON));
    } else {
        reportError("semicolon", currentToken().value);
    }

    return node;
}

// <function-declaration> ->
//     functionsy + ident + (formal-parameter-list)? + colon + ident + semicolon + block + semicolon
ParseTreeNode* Parser::parseFunctionDeclaration() {
    ParseTreeNode* node = new ParseTreeNode("<function-declaration>");

    node->children.push_back(match(TokenType::FUNCTIONSY));

    if (currentToken().type == TokenType::IDENT) {
        node->children.push_back(match(TokenType::IDENT));
    } else {
        reportError("ident (nama fungsi)", currentToken().value);
        return node;
    }

    if (currentToken().type == TokenType::LPARENT) {
        node->children.push_back(parseFormalParameterList());
        if (hasError) return node;
    }

    if (currentToken().type == TokenType::COLON) {
        node->children.push_back(match(TokenType::COLON));
    } else {
        reportError("colon", currentToken().value);
        return node;
    }

    if (currentToken().type == TokenType::IDENT) {
        node->children.push_back(match(TokenType::IDENT));
    } else {
        reportError("ident (tipe kembalian)", currentToken().value);
        return node;
    }

    if (currentToken().type == TokenType::SEMICOLON) {
        node->children.push_back(match(TokenType::SEMICOLON));
    } else {
        reportError("semicolon", currentToken().value);
        return node;
    }

    node->children.push_back(parseBlock());
    if (hasError) return node;

    if (currentToken().type == TokenType::SEMICOLON) {
        node->children.push_back(match(TokenType::SEMICOLON));
    } else {
        reportError("semicolon", currentToken().value);
    }

    return node;
}

// <formal-parameter-list> ->
//     lparent + parameter-group + (semicolon + parameter-group)* + rparent
ParseTreeNode* Parser::parseFormalParameterList() {
    ParseTreeNode* node = new ParseTreeNode("<formal-parameter-list>");

    if (currentToken().type == TokenType::LPARENT) {
        node->children.push_back(match(TokenType::LPARENT));
    } else {
        reportError("lparent", currentToken().value);
        return node;
    }

    node->children.push_back(parseParameterGroup());
    if (hasError) return node;

    while (!hasError && currentToken().type == TokenType::SEMICOLON) {
        node->children.push_back(match(TokenType::SEMICOLON));
        node->children.push_back(parseParameterGroup());
    }

    if (currentToken().type == TokenType::RPARENT) {
        node->children.push_back(match(TokenType::RPARENT));
    } else {
        reportError("rparent", currentToken().value);
    }

    return node;
}

// <parameter-group> -> identifier-list + colon + (ident | array-type)
ParseTreeNode* Parser::parseParameterGroup() {
    ParseTreeNode* node = new ParseTreeNode("<parameter-group>");

    node->children.push_back(parseIdentifierList());
    if (hasError) return node;

    if (currentToken().type == TokenType::COLON) {
        node->children.push_back(match(TokenType::COLON));
    } else {
        reportError("colon", currentToken().value);
        return node;
    }

    if (currentToken().type == TokenType::IDENT) {
        node->children.push_back(match(TokenType::IDENT));
    } else if (currentToken().type == TokenType::ARRAYSY) {
        node->children.push_back(parseArrayType());
    } else {
        reportError("ident atau arraysy", currentToken().value);
    }

    return node;
}

// <procedure/function-call> -> ident + (lparent + parameter-list? + rparent)?
ParseTreeNode* Parser::parseProcedureFunctionCall() {
    ParseTreeNode* node = new ParseTreeNode("<procedure/function-call>");

    if (currentToken().type == TokenType::IDENT) {
        node->children.push_back(match(TokenType::IDENT));
    } else {
        reportError("ident (nama prosedur/fungsi)", currentToken().value);
        return node;
    }

    if (currentToken().type == TokenType::LPARENT) {
        node->children.push_back(match(TokenType::LPARENT));

        // parameter-list opsional (boleh kosong: writeln() )
        if (currentToken().type != TokenType::RPARENT) {
            node->children.push_back(parseParameterList());
            if (hasError) return node;
        }

        if (currentToken().type == TokenType::RPARENT) {
            node->children.push_back(match(TokenType::RPARENT));
        } else {
            reportError("rparent", currentToken().value);
        }
    }

    return node;
}

// <parameter-list> -> expression + (comma + expression)*
ParseTreeNode* Parser::parseParameterList() {
    ParseTreeNode* node = new ParseTreeNode("<parameter-list>");

    node->children.push_back(parseExpression());

    while (!hasError && currentToken().type == TokenType::COMMA) {
        node->children.push_back(match(TokenType::COMMA));
        node->children.push_back(parseExpression());
    }

    return node;
}
