#include "lexer.h"

#include <cctype>
#include <utility>

Lexer::Lexer(string source)
    : source_code(std::move(source)), current_pos(0), current_char('\0') {
    if (!source_code.empty()) {
        current_char = source_code[0];
    }
}

void Lexer::advance() {
    if (current_pos + 1 < source_code.size()) {
        ++current_pos;
        current_char = source_code[current_pos];
        return;
    }

    current_pos = source_code.size();
    current_char = '\0';
}

char Lexer::peek() {
    const size_t next_pos = current_pos + 1;
    if (next_pos < source_code.size()) {
        return source_code[next_pos];
    }
    return '\0';
}

void Lexer::skipWhitespace() {
    while (current_char != '\0' &&
           isspace(static_cast<unsigned char>(current_char))) {
        advance();
    }
}

Token Lexer::scanComment() {
    string lexeme;

    if (current_char == '{') {
        lexeme += current_char;
        advance();
        while (current_char != '\0' && current_char != '}') {
            lexeme += current_char;
            advance();
        }
        if (current_char == '}') {
            lexeme += current_char;
            advance();
        }
    } else {
        // (* ... *) style
        lexeme += current_char; advance();  // '('
        lexeme += current_char; advance();  // '*'
        while (current_char != '\0') {
            if (current_char == '*' && peek() == ')') {
                lexeme += current_char; advance();  // '*'
                lexeme += current_char; advance();  // ')'
                break;
            }
            lexeme += current_char;
            advance();
        }
    }

    return Token(TokenType::COMMENT, lexeme);
}

Token Lexer::scanNumber() {
    string lexeme;

    while (current_char != '\0' &&
           isdigit(static_cast<unsigned char>(current_char))) {
        lexeme += current_char;
        advance();
    }

    if (current_char == '.' && isdigit(static_cast<unsigned char>(peek()))) {
        lexeme += current_char;
        advance();

        while (current_char != '\0' &&
               isdigit(static_cast<unsigned char>(current_char))) {
            lexeme += current_char;
            advance();
        }

        return Token(TokenType::REALCON, lexeme);
    }

    return Token(TokenType::INTCON, lexeme);
}

TokenType Lexer::checkKeyword(const string& ident) {
    static const unordered_map<string, TokenType> keyword_map = {
        // Keywords
        {"const", TokenType::CONSTSY},
        {"type", TokenType::TYPESY},
        {"var", TokenType::VARSY},
        {"function", TokenType::FUNCTIONSY},
        {"procedure", TokenType::PROCEDURESY},
        {"array", TokenType::ARRAYSY},
        {"record", TokenType::RECORDSY},
        {"program", TokenType::PROGRAMSY},
        {"begin", TokenType::BEGINSY},
        {"if", TokenType::IFSY},
        {"case", TokenType::CASESY},
        {"repeat", TokenType::REPEATSY},
        {"while", TokenType::WHILESY},
        {"for", TokenType::FORSY},
        {"end", TokenType::ENDSY},
        {"else", TokenType::ELSESY},
        {"until", TokenType::UNTILSY},
        {"of", TokenType::OFSY},
        {"do", TokenType::DOSY},
        {"to", TokenType::TOSY},
        {"downto", TokenType::DOWNTOSY},
        {"then", TokenType::THENSY},

        // Operator kata
        {"not", TokenType::NOTSY},
        {"and", TokenType::ANDSY},
        {"or", TokenType::ORSY},
        {"div", TokenType::IDIV},
        {"mod", TokenType::IMOD},
    };

    string lower_ident;
    lower_ident.reserve(ident.size());

    for (const char ch : ident) {
        lower_ident += static_cast<char>(
            tolower(static_cast<unsigned char>(ch)));
    }

    const auto it = keyword_map.find(lower_ident);
    if (it != keyword_map.end()) {
        return it->second;
    }
    return TokenType::IDENT;
}

Token Lexer::scanIdentOrKeyword() {
    string lexeme;

    while (current_char != '\0' &&
           isalnum(static_cast<unsigned char>(current_char))) {
        lexeme += current_char;
        advance();
    }

    const TokenType type = checkKeyword(lexeme);
    return Token(type, lexeme);
}

Token Lexer::scanStringOrChar() {
    string lexeme;
    lexeme += current_char;  // opening quote
    advance();

    while (current_char != '\0' && current_char != '\'') {
        lexeme += current_char;
        advance();
    }

    if (current_char == '\'') {
        lexeme += current_char;  // closing quote
        advance();

        if (lexeme.size() == 3) {
            return Token(TokenType::CHARCON, lexeme);
        }
        return Token(TokenType::STRING, lexeme);
    }

    return Token(TokenType::UNKNOWN, lexeme);
}

Token Lexer::scanSymbol() {
    switch (current_char) {
        case '+':
            advance();
            return Token(TokenType::PLUS, "+");
        case '-':
            advance();
            return Token(TokenType::MINUS, "-");
        case '*':
            advance();
            return Token(TokenType::TIMES, "*");
        case '/':
            advance();
            return Token(TokenType::RDIV, "/");
        case '=':
            advance();
            if (current_char == '=') {
                advance();
                return Token(TokenType::EQL, "==");
            }
            return Token(TokenType::UNKNOWN, "=");
        case '<':
            advance();
            if (current_char == '=') {
                advance();
                return Token(TokenType::LEQ, "<=");
            }
            if (current_char == '>') {
                advance();
                return Token(TokenType::NEQ, "<>");
            }
            return Token(TokenType::LSS, "<");
        case '>':
            advance();
            if (current_char == '=') {
                advance();
                return Token(TokenType::GEQ, ">=");
            }
            return Token(TokenType::GTR, ">");
        case '(':
            advance();
            return Token(TokenType::LPARENT, "(");
        case ')':
            advance();
            return Token(TokenType::RPARENT, ")");
        case '[':
            advance();
            return Token(TokenType::LBRACK, "[");
        case ']':
            advance();
            return Token(TokenType::RBRACK, "]");
        case ',':
            advance();
            return Token(TokenType::COMMA, ",");
        case ';':
            advance();
            return Token(TokenType::SEMICOLON, ";");
        case '.':
            advance();
            return Token(TokenType::PERIOD, ".");
        case ':':
            advance();
            if (current_char == '=') {
                advance();
                return Token(TokenType::BECOMES, ":=");
            }
            return Token(TokenType::COLON, ":");
        default: {
            const string unknown_lexeme(1, current_char);
            advance();
            return Token(TokenType::UNKNOWN, unknown_lexeme);
        }
    }
}

vector<Token> Lexer::tokenize() {
    vector<Token> tokens;

    while (current_char != '\0') {
        skipWhitespace();

        if (current_char == '\0') {
            break;
        }

        if (current_char == '{' ||
            (current_char == '(' && peek() == '*')) {
            tokens.push_back(scanComment());
            continue;
        }

        if (isalpha(static_cast<unsigned char>(current_char))) {
            tokens.push_back(scanIdentOrKeyword());
            continue;
        }

        if (isdigit(static_cast<unsigned char>(current_char))) {
            tokens.push_back(scanNumber());
            continue;
        }

        if (current_char == '\'') {
            tokens.push_back(scanStringOrChar());
            continue;
        }

        tokens.push_back(scanSymbol());
    }

    tokens.push_back(Token(TokenType::END_OF_FILE, ""));
    return tokens;
}

string Lexer::tokenTypeToString(TokenType type) {
    switch (type) {
        case TokenType::INTCON:
            return "intcon";
        case TokenType::REALCON:
            return "realcon";
        case TokenType::CHARCON:
            return "charcon";
        case TokenType::STRING:
            return "string";
        case TokenType::IDENT:
            return "ident";
        case TokenType::NOTSY:
            return "notsy";
        case TokenType::PLUS:
            return "plus";
        case TokenType::MINUS:
            return "minus";
        case TokenType::TIMES:
            return "times";
        case TokenType::IDIV:
            return "idiv";
        case TokenType::RDIV:
            return "rdiv";
        case TokenType::IMOD:
            return "imod";
        case TokenType::ANDSY:
            return "andsy";
        case TokenType::ORSY:
            return "orsy";
        case TokenType::EQL:
            return "eql";
        case TokenType::NEQ:
            return "neq";
        case TokenType::GTR:
            return "gtr";
        case TokenType::GEQ:
            return "geq";
        case TokenType::LSS:
            return "lss";
        case TokenType::LEQ:
            return "leq";
        case TokenType::LPARENT:
            return "lparent";
        case TokenType::RPARENT:
            return "rparent";
        case TokenType::LBRACK:
            return "lbrack";
        case TokenType::RBRACK:
            return "rbrack";
        case TokenType::COMMA:
            return "comma";
        case TokenType::SEMICOLON:
            return "semicolon";
        case TokenType::PERIOD:
            return "period";
        case TokenType::COLON:
            return "colon";
        case TokenType::BECOMES:
            return "becomes";
        case TokenType::CONSTSY:
            return "constsy";
        case TokenType::TYPESY:
            return "typesy";
        case TokenType::VARSY:
            return "varsy";
        case TokenType::FUNCTIONSY:
            return "functionsy";
        case TokenType::PROCEDURESY:
            return "proceduresy";
        case TokenType::ARRAYSY:
            return "arraysy";
        case TokenType::RECORDSY:
            return "recordsy";
        case TokenType::PROGRAMSY:
            return "programsy";
        case TokenType::BEGINSY:
            return "beginsy";
        case TokenType::IFSY:
            return "ifsy";
        case TokenType::CASESY:
            return "casesy";
        case TokenType::REPEATSY:
            return "repeatsy";
        case TokenType::WHILESY:
            return "whilesy";
        case TokenType::FORSY:
            return "forsy";
        case TokenType::ENDSY:
            return "endsy";
        case TokenType::ELSESY:
            return "elsesy";
        case TokenType::UNTILSY:
            return "untilsy";
        case TokenType::OFSY:
            return "ofsy";
        case TokenType::DOSY:
            return "dosy";
        case TokenType::TOSY:
            return "tosy";
        case TokenType::DOWNTOSY:
            return "downtosy";
        case TokenType::THENSY:
            return "thensy";
        case TokenType::COMMENT:
            return "comment";
        case TokenType::UNKNOWN:
            return "unknown";
        case TokenType::END_OF_FILE:
            return "eof";
        default:
            return "unknown";
    }
}
