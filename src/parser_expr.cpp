#include "parser.h"

// Inisialisasi Parser dengan list token hasil dari Lexical Analysis
Parser::Parser(const vector<Token>& tokens) {
  this->tokens = tokens;
  this->currentTokenIndex = 0;
  this->hasError = false;
}

// Entry point untuk memulai proses pemodelan Parse Tree
ParseTreeNode* Parser::parse() {
  ParseTreeNode* root = parseProgram();
  return root;
}

// Mengambil token yang sedang diproses saat ini
Token Parser::currentToken() {
  if(this->currentTokenIndex < this->tokens.size()) {
    return tokens[this->currentTokenIndex];
  }
  return Token(TokenType::END_OF_FILE, "EOF");
}

// Mengintip token di depan tanpa memajukan indeks
Token Parser::peek(int offset) {
  if ((this->currentTokenIndex + offset) < this->tokens.size()) {
    return tokens[this->currentTokenIndex+offset];
  }
  return Token(TokenType::END_OF_FILE, "EOF");
}

// Memajukan indeks ke token selanjutnya
void Parser::advance() {
  if (this->currentTokenIndex < this->tokens.size()) {
    this->currentTokenIndex++;
  }
}

// Validasi token sat ini dengan expectedType. Mengambalikan leaf node jika cocok, atau nullptr jika terjadi syntax error
ParseTreeNode* Parser::match(TokenType expectedType) {
  Token current = this->currentToken();

  Lexer dummyLexer("");
  string currentStr = dummyLexer.tokenTypeToString(current.type);
  string expectedStr = dummyLexer.tokenTypeToString(expectedType);

  if (current.type == expectedType) {
    ParseTreeNode* node = new ParseTreeNode(currentStr, current.value);
    advance();
    return node;
  } else {
    reportError(expectedStr, current.value);
    return nullptr;
  }
}

// Mencetak pesan error pertama yang ditemukan
void Parser::reportError(string expected, string found) {
  if (!hasError) {
    cout << "Syntax error: expected '" << expected << "', but found '" << found << "'" << endl;
    hasError = true;
  }
}

// Mencetak parse tree secara hierarkis
void Parser::printTree(ParseTreeNode* node, ofstream& outFile, string indent, bool isLast, bool isRoot) {
  if (node == nullptr) return;

  string marker ="";
  if (!isRoot) {
    marker = isLast ? "└──" : "├──";
    outFile << indent << marker;
  }

  outFile << node->label;
  if (node->value != "") {
    outFile << " (" << node->value << ") ";
  }
  outFile << endl;

  string newIndent = indent;
  if (!isRoot) {
    newIndent += isLast ? " " : "| ";
  }

  for (size_t i = 0; i < node->children.size(); i++) {
    bool isChildLast = (i == node->children.size() - 1);
    printTree(node->children[i], outFile, newIndent, isChildLast, false);
  }
}

// Grammar: <if-statement> -> ifsy + expression + thensy + statement + (elsesy + statement)?
ParseTreeNode* Parser::parseIfStatement() {
  ParseTreeNode* node = new ParseTreeNode("<if-statement>");

  node->children.push_back(match(TokenType::IFSY));
  if (hasError) return node;

  node->children.push_back(parseExpression());
  if (hasError) return node;

  node->children.push_back(match(TokenType::THENSY));
  if (hasError) return node;

  node->children.push_back(parseStatement());

  // Evaluasi blok 'else' jika token opsional elsesy ditemukan
  if (!hasError && currentToken().type == TokenType::ELSESY) {
    node->children.push_back(match(TokenType::ELSESY));
    node->children.push_back(parseStatement());
  }

  return node;
}

// Grammar: <while-statement> -> whilesy + expression + dosy + statement
ParseTreeNode* Parser::parseWhileStatement() {
  ParseTreeNode* node = new ParseTreeNode("<while-statement>");

  node->children.push_back(match(TokenType::WHILESY));
  if (hasError) return node;

  node->children.push_back(parseExpression());
  if (hasError) return node;

  node->children.push_back(match(TokenType::DOSY));
  if (hasError) return node;

  node->children.push_back(parseStatement());

  return node;
}

// Grammar: <case-statement> -> casesy + expression + ofsy + case-block + endsy
ParseTreeNode* Parser::parseCaseStatement() {
  ParseTreeNode* node = new ParseTreeNode("<case-statement>");

  node->children.push_back(match(TokenType::CASESY));
  node->children.push_back(parseExpression()); 
  node->children.push_back(match(TokenType::OFSY));
  if (hasError) return node;

  node->children.push_back(parseCaseBlock());

  if (!hasError) {
    node->children.push_back(match(TokenType::ENDSY));
  }
  
  return node;
}

// Grammar: <case-block> -> constant + (comma + constant)* + colon + statement + (semicolon + case-block?)*
ParseTreeNode* Parser::parseCaseBlock() {
  ParseTreeNode* node = new ParseTreeNode("<case-block>");

  node->children.push_back(parseConstant());

  // Menangani multiple konstan yang dipisahkan koma (contoh: 1, 2, 3: statement)
  while (!hasError && currentToken().type == TokenType::COMMA) {
    node->children.push_back(match(TokenType::COMMA));
    node->children.push_back(parseConstant());
  }

  if (!hasError) {
    node->children.push_back(match(TokenType::COLON));
    node->children.push_back(parseStatement());
  }

  // Rekursif ke case-block selanjutnya jika dipisahkan oleh titik koma
  while (!hasError && currentToken().type == TokenType::SEMICOLON) {
    node->children.push_back(match(TokenType::SEMICOLON));

    // Jika setelah titik koma adalah endsy, berarti blok case telah selesai
    if (currentToken().type == TokenType::ENDSY) {
      break;
    }
    node->children.push_back(parseCaseBlock());
  }
  return node;
}

// Grammar: <repeat-statement> -> repeatsy + statement-list + untilsy + expression
ParseTreeNode* Parser::parseRepeatStatement() {
  ParseTreeNode* node = new ParseTreeNode("<repeat-statement>");

  node->children.push_back(match(TokenType::REPEATSY));
  if (hasError) return node;

  node->children.push_back(parseStatementList());
  if (hasError) return node;

  node->children.push_back(match(TokenType::UNTILSY));
  if (hasError) return node;

  node->children.push_back(parseExpression());

  return node;
}


// Grammar: <for-statement> -> forsy + ident + becomes + expression + (tosy|downtosy) + expression + dosy + statement
ParseTreeNode* Parser::parseForStatement() {
  ParseTreeNode* node = new ParseTreeNode("<for-statement>");

  node->children.push_back(match(TokenType::FORSY));
  node->children.push_back(match(TokenType::IDENT));
  node->children.push_back(match(TokenType::BECOMES));
  node->children.push_back(parseExpression()); 
  if (hasError) return node;

  if (currentToken().type == TokenType::TOSY) {
    node->children.push_back(match(TokenType::TOSY));
  } else if (currentToken().type == TokenType::DOWNTOSY) {
    node->children.push_back(match(TokenType::DOWNTOSY));
  } else {
    reportError("to atau downto", currentToken().value);
    return node;
  }

  node->children.push_back(parseExpression());
  if (hasError) return node;

  node->children.push_back(match(TokenType::DOSY));
  node->children.push_back(parseStatement());

  return node;
}

// Hierarki terendah, yaitu Identifier, konstanta dasar, ekspresi berkurung, dan operator unary (NOT)
ParseTreeNode* Parser::parseFactor() {
  ParseTreeNode* node = new ParseTreeNode("<factor>");
  TokenType type = this->currentToken().type;

  if (type == TokenType::IDENT) {
    Token next = peek(1);
    // Lookahead untuk membedakan Identifier biasa, Variabel (Array/Record), dan Pemanggilan Fungsi
    if (next.type == TokenType::LBRACK || next.type == TokenType::PERIOD) {
      node->children.push_back(parseVariable());
    } else if (next.type == TokenType::LPARENT) {
      node->children.push_back(parseProcedureFunctionCall());
    } else {
      node->children.push_back(match(TokenType::IDENT));
    }
  } else if (type == TokenType::INTCON || type == TokenType::REALCON || type == TokenType::CHARCON || type == TokenType::STRING) {
    node->children.push_back(match(type));
  } else if (type == TokenType::NOTSY) {
    node->children.push_back(match(TokenType::NOTSY));
    node->children.push_back(parseFactor());
  } else if (type == TokenType::LPARENT) {
    node->children.push_back(match(TokenType::LPARENT));
    node->children.push_back(parseExpression());

    if(!this->hasError) {
      node->children.push_back(match(TokenType::RPARENT));
    }
  } else {
    reportError("factor (angka, string, identifier, atau '(')", currentToken().value);
  }
  return node;
}


// Hierarki kedua, yaitu perkalian (TIMES, RDIV, IDIV, IMOD, ANDSY)
ParseTreeNode* Parser::parseTerm() {
  ParseTreeNode* node = new ParseTreeNode("<term>");

  node->children.push_back(parseFactor());
  if (hasError) return node;

  TokenType type = currentToken().type;
  // Looping untuk menangani ekspresi perkalian berantai
  while (!hasError && (type == TokenType::TIMES || type == TokenType::RDIV || type == TokenType::IDIV || type == TokenType::IMOD || type == TokenType::ANDSY)) {
    ParseTreeNode* opNode = new ParseTreeNode("<multiplicative-operator>");
    opNode->children.push_back(match(type));
    node->children.push_back(opNode);
    node->children.push_back(parseFactor());
    type = currentToken().type;
  }
  return node;
}


// Hierarki ketiga, yaitu penjumlahan dan pengurangan (PLUS, MINUS, ORSY)
ParseTreeNode* Parser::parseSimpleExpression() {
  ParseTreeNode* node = new ParseTreeNode("<simple-expression>");
  TokenType type = currentToken().type;

  // Menangani unary sign di awal ekspresi
  if (type == TokenType::PLUS || type == TokenType::MINUS) {
    node->children.push_back(match(type));
  }

  node->children.push_back(parseTerm());
  if (hasError) return node;

  type = currentToken().type;
  // Looping untuk menangani ekspresi penjumlahan berantai
  while (!hasError && (type == TokenType::PLUS || type == TokenType::MINUS || type == TokenType::ORSY)) {
    ParseTreeNode* opNode = new ParseTreeNode("<additive-operator>");
    opNode->children.push_back(match(type));
    node->children.push_back(opNode);
    node->children.push_back(parseTerm());
    type = currentToken().type;
  }
    return node;
}

// Hierarki tertinggi, yaitu evaluasi relasional/perbandingan (EQL, NEQ, LSS, LEQ, GTR, GEQ)
ParseTreeNode* Parser::parseExpression() {
  ParseTreeNode* node = new ParseTreeNode("<expression>");
  
  node->children.push_back(parseSimpleExpression());
  if (hasError) return node;

  TokenType type = currentToken().type;
  // Operator relasioonal dievaluasi paling akhir untuk menghasilkan boolean
  if (type == TokenType::EQL || type == TokenType::NEQ ||
    type == TokenType::LSS || type == TokenType::LEQ ||
    type == TokenType::GTR || type == TokenType::GEQ) {
    ParseTreeNode* opNode = new ParseTreeNode("<relational-operator>");
    opNode->children.push_back(match(type));
    node->children.push_back(opNode);
    node->children.push_back(parseSimpleExpression());
  }
  return node;
}