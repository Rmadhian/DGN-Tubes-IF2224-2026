#include "parser.h"

// Top-Level Program

// Aturan: <program> -> <program-header> <declaration-part> <compound-statement> period
ParseTreeNode* Parser::parseProgram() {
    ParseTreeNode* node = new ParseTreeNode("<program>");
    
    node->children.push_back(parseProgramHeader());
    node->children.push_back(parseDeclarationPart());
    node->children.push_back(parseCompoundStatement());
    node->children.push_back(match(TokenType::PERIOD));
    
    return node;
}

// Aturan: <program-header> -> programsy ident semicolon
ParseTreeNode* Parser::parseProgramHeader() {
    ParseTreeNode* node = new ParseTreeNode("<program-header>");
    
    node->children.push_back(match(TokenType::PROGRAMSY));
    node->children.push_back(match(TokenType::IDENT));
    node->children.push_back(match(TokenType::SEMICOLON));
    
    return node;
}

// Aturan: <declaration-part> -> (const-declaration)* (type-declaration)* (var-declaration)* (subprogram-declaration)*
ParseTreeNode* Parser::parseDeclarationPart() {
    ParseTreeNode* node = new ParseTreeNode("<declaration-part>");
    
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

// Variabel & Kontanta

// Aturan: <const-declaration> -> constsy (ident eql constant semicolon)+
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

// Aturan: <constant> -> charcon | string | [(plus | minus)? (ident | intcon | realcon)]
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

// Aturan: <var-declaration> -> varsy (identifier-list colon type semicolon)+
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

// Aturan: <identifier-list> -> ident (comma ident)*
ParseTreeNode* Parser::parseIdentifierList() {
    ParseTreeNode* node = new ParseTreeNode("<identifier-list>");
    node->children.push_back(match(TokenType::IDENT));
    
    while (currentToken().type == TokenType::COMMA) {
        node->children.push_back(match(TokenType::COMMA));
        node->children.push_back(match(TokenType::IDENT));
    }
    
    return node;
}

// Data Types

// Aturan: <type-declaration> -> typesy (ident eql type semicolon)+
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

// Aturan: <type> -> ident | array-type | range | enumerated | record-type
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

// Aturan: <array-type> -> arraysy lbrack (range | ident) rbrack ofsy type
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

// Aturan: <range> -> constant period period constant
ParseTreeNode* Parser::parseRange() {
    ParseTreeNode* node = new ParseTreeNode("<range>");
    
    node->children.push_back(parseConstant());
    node->children.push_back(match(TokenType::PERIOD));
    node->children.push_back(match(TokenType::PERIOD));
    node->children.push_back(parseConstant());
    
    return node;
}

// Aturan: <enumerated> -> lparent ident (comma ident)* rparent
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

// Aturan: <record-type> -> recordsy field-list endsy
ParseTreeNode* Parser::parseRecordType() {
    ParseTreeNode* node = new ParseTreeNode("<record-type>");
    node->children.push_back(match(TokenType::RECORDSY));
    node->children.push_back(parseFieldList());
    node->children.push_back(match(TokenType::ENDSY));
    
    return node;
}

// Aturan: <field-list> -> field-part (semicolon field-part)*
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

// Aturan: <field-part> -> identifier-list colon type
ParseTreeNode* Parser::parseFieldPart() {
    ParseTreeNode* node = new ParseTreeNode("<field-part>");
    
    node->children.push_back(parseIdentifierList());
    node->children.push_back(match(TokenType::COLON));
    node->children.push_back(parseType());
    
    return node;
}