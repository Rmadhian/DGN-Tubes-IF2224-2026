#include "parser.h"

ParseTreeNode* Parser::parseProgram() {
    ParseTreeNode* node = new ParseTreeNode("<program>");
    
    // memanggil urutan
    node->children.push_back(parseProgramHeader());
    node->children.push_back(parseDeclarationPart());
    node->children.push_back(parseCompoundStatement());
    node->children.push_back(match(TokenType::PERIOD));
    
    return node;
}

ParseTreeNode* Parser::parseProgramHeader() {
    ParseTreeNode* node = new ParseTreeNode("<program-header>");
    
    node->children.push_back(match(TokenType::PROGRAMSY));
    node->children.push_back(match(TokenType::IDENT));
    node->children.push_back(match(TokenType::SEMICOLON));
    
    return node;
}

ParseTreeNode* Parser::parseDeclarationPart() {
    ParseTreeNode* node = new ParseTreeNode("<declaration-part>");
    
    // Perulangan membaca deklarasi sesuai urutan grammar
    while (currentToken().type == TokenType::CONSTSY) {
        node->children.push_back(parseConstDeclaration());
    }
    while (currentToken().type == TokenType::TYPESY) {
        node->children.push_back(parseTypeDeclaration());
    }
    while (currentToken().type == TokenType::VARSY) {
        node->children.push_back(parseVarDeclaration());
    }
    while (currentToken().type == TokenType::PROCEDURESY || currentToken().type == TokenType::FUNCTIONSY) {
        node->children.push_back(parseSubprogramDeclaration());
    }
    
    return node;
}

ParseTreeNode* Parser::parseConstDeclaration() {
    ParseTreeNode* node = new ParseTreeNode("<const-declaration>");
    node->children.push_back(match(TokenType::CONSTSY));
    
    do {
        node->children.push_back(match(TokenType::IDENT));
        node->children.push_back(match(TokenType::EQL));
        node->children.push_back(parseConstant());
        node->children.push_back(match(TokenType::SEMICOLON));
    } while (currentToken().type == TokenType::IDENT);
    
    return node;
}

ParseTreeNode* Parser::parseConstant() {
    ParseTreeNode* node = new ParseTreeNode("<constant>");
    
    if (currentToken().type == TokenType::CHARCON) {
        node->children.push_back(match(TokenType::CHARCON));
    } else if (currentToken().type == TokenType::STRING) {
        node->children.push_back(match(TokenType::STRING));
    } else {
        if (currentToken().type == TokenType::PLUS) {
            node->children.push_back(match(TokenType::PLUS));
        } else if (currentToken().type == TokenType::MINUS) {
            node->children.push_back(match(TokenType::MINUS));
        }
        
        if (currentToken().type == TokenType::IDENT) {
            node->children.push_back(match(TokenType::IDENT));
        } else if (currentToken().type == TokenType::INTCON) {
            node->children.push_back(match(TokenType::INTCON));
        } else if (currentToken().type == TokenType::REALCON) {
            node->children.push_back(match(TokenType::REALCON));
        } else {
            reportError("ident, intcon, realcon, charcon, or string", currentToken().value);
        }
    }
    
    return node;
}

ParseTreeNode* Parser::parseVarDeclaration() {
    ParseTreeNode* node = new ParseTreeNode("<var-declaration>");
    node->children.push_back(match(TokenType::VARSY));
    
    do {
        node->children.push_back(parseIdentifierList());
        node->children.push_back(match(TokenType::COLON));
        node->children.push_back(parseType());
        node->children.push_back(match(TokenType::SEMICOLON));
    } while (currentToken().type == TokenType::IDENT);
    
    return node;
}

ParseTreeNode* Parser::parseIdentifierList() {
    ParseTreeNode* node = new ParseTreeNode("<identifier-list>");
    node->children.push_back(match(TokenType::IDENT));
    
    while (currentToken().type == TokenType::COMMA) {
        node->children.push_back(match(TokenType::COMMA));
        node->children.push_back(match(TokenType::IDENT));
    }
    
    return node;
}

ParseTreeNode* Parser::parseTypeDeclaration() {
    ParseTreeNode* node = new ParseTreeNode("<type-declaration>");
    node->children.push_back(match(TokenType::TYPESY));
    
    do {
        node->children.push_back(match(TokenType::IDENT));
        node->children.push_back(match(TokenType::EQL));
        node->children.push_back(parseType());
        node->children.push_back(match(TokenType::SEMICOLON));
    } while (currentToken().type == TokenType::IDENT);
    
    return node;
}

ParseTreeNode* Parser::parseType() {
    ParseTreeNode* node = new ParseTreeNode("<type>");
    
    if (currentToken().type == TokenType::IDENT) {
        node->children.push_back(match(TokenType::IDENT));
    } else if (currentToken().type == TokenType::ARRAYSY) {
        node->children.push_back(parseArrayType());
    } else if (currentToken().type == TokenType::LPARENT) {
        node->children.push_back(parseEnumerated());
    } else if (currentToken().type == TokenType::RECORDSY) {
        node->children.push_back(parseRecordType());
    } else {
        node->children.push_back(parseRange());
    }
    
    return node;
}

ParseTreeNode* Parser::parseArrayType() {
    ParseTreeNode* node = new ParseTreeNode("<array-type>");
    node->children.push_back(match(TokenType::ARRAYSY));
    node->children.push_back(match(TokenType::LBRACK));
    
    if (currentToken().type == TokenType::IDENT && peek().type == TokenType::RBRACK) {
        node->children.push_back(match(TokenType::IDENT));
    } else {
        node->children.push_back(parseRange());
    }

    node->children.push_back(match(TokenType::RBRACK));
    node->children.push_back(match(TokenType::OFSY));
    node->children.push_back(parseType());
    
    return node;
}

ParseTreeNode* Parser::parseRange() {
    ParseTreeNode* node = new ParseTreeNode("<range>");
    
    node->children.push_back(parseConstant());
    node->children.push_back(match(TokenType::PERIOD));
    node->children.push_back(match(TokenType::PERIOD));
    node->children.push_back(parseConstant());
    
    return node;
}

ParseTreeNode* Parser::parseEnumerated() {
    ParseTreeNode* node = new ParseTreeNode("<enumerated>");
    node->children.push_back(match(TokenType::LPARENT));
    node->children.push_back(match(TokenType::IDENT));
    
    while (currentToken().type == TokenType::COMMA) {
        node->children.push_back(match(TokenType::COMMA));
        node->children.push_back(match(TokenType::IDENT));
    }
    
    node->children.push_back(match(TokenType::RPARENT));
    return node;
}

ParseTreeNode* Parser::parseRecordType() {
    ParseTreeNode* node = new ParseTreeNode("<record-type>");
    node->children.push_back(match(TokenType::RECORDSY));
    node->children.push_back(parseFieldList());
    node->children.push_back(match(TokenType::ENDSY));
    
    return node;
}

ParseTreeNode* Parser::parseFieldList() {
    ParseTreeNode* node = new ParseTreeNode("<field-list>");
    node->children.push_back(parseFieldPart());
    
    while (currentToken().type == TokenType::SEMICOLON) {
        node->children.push_back(match(TokenType::SEMICOLON));
        
        if (currentToken().type == TokenType::IDENT) {
            node->children.push_back(parseFieldPart());
        } else {
            break; 
        }
    }
    
    return node;
}

ParseTreeNode* Parser::parseFieldPart() {
    ParseTreeNode* node = new ParseTreeNode("<field-part>");
    
    node->children.push_back(parseIdentifierList());
    node->children.push_back(match(TokenType::COLON));
    node->children.push_back(parseType());
    
    return node;
}