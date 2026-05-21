#include "semantic.h"
#include <iostream>

using namespace std;

// =========================================================
// Accept dispatch untuk node ekspresi
// =========================================================

void BinaryOpNode::accept(SemanticVisitor* v)  { v->visit(this); }
void UnaryOpNode::accept(SemanticVisitor* v)   { v->visit(this); }
void LiteralNode::accept(SemanticVisitor* v)   { v->visit(this); }
void VarAccessNode::accept(SemanticVisitor* v) { v->visit(this); }
void FuncCallNode::accept(SemanticVisitor* v)  { v->visit(this); }

// =========================================================
// Print AST untuk node ekspresi
// =========================================================

void BinaryOpNode::print(ostream& out, int indent, const SymbolTable* st) const {
    printIndent(out, indent);
    out << "BinaryOp('" << op << "') → type:" << dtStr(evalType) << "\n";
    if (left)  left->print(out, indent + 1, st);
    if (right) right->print(out, indent + 1, st);
}

void UnaryOpNode::print(ostream& out, int indent, const SymbolTable* st) const {
    printIndent(out, indent);
    out << "UnaryOp('" << op << "') → type:" << dtStr(evalType) << "\n";
    if (operand) operand->print(out, indent + 1, st);
}

void LiteralNode::print(ostream& out, int indent, const SymbolTable* st) const {
    printIndent(out, indent);
    out << "Literal(" << value << ") → type:" << dtStr(evalType) << "\n";
}

void VarAccessNode::print(ostream& out, int indent, const SymbolTable* st) const {
    printIndent(out, indent);
    out << "VarAccess('" << name << "')";
    if (!fieldName.empty()) out << "." << fieldName;
    out << " → type:" << dtStr(evalType)
        << ", tab_index:" << symRef
        << ", lev:" << lexicalLevel;
    if (!indices.empty()) out << ", subscripted";
    out << "\n";
    for (auto* idx : indices)
        if (idx) idx->print(out, indent + 1, st);
}

void FuncCallNode::print(ostream& out, int indent, const SymbolTable* st) const {
    printIndent(out, indent);
    out << "FuncCall('" << name << "') → type:" << dtStr(evalType)
        << ", tab_index:" << symRef << "\n";
    for (auto* arg : args)
        if (arg) arg->print(out, indent + 1, st);
}

// =========================================================
// SemanticAnalyzer: Helper type compatibility
// =========================================================

bool SemanticAnalyzer::isAssignmentCompatible(DataType t1, DataType t2) const {
    // NOTYPE bersifat permisif (sudah ada error sebelumnya)
    if (t1 == DataType::NOTYPE || t2 == DataType::NOTYPE) return true;
    // Tipe sama: selalu compatible
    if (t1 == t2) return true;
    // Real menerima Integer (widening / assignment-compatible)
    if (t1 == DataType::REAL && t2 == DataType::INTEGER) return true;
    return false;
}

bool SemanticAnalyzer::isOrdinalType(DataType t) const {
    return t == DataType::INTEGER || t == DataType::CHAR || t == DataType::BOOLEAN;
}

bool SemanticAnalyzer::isNumericType(DataType t) const {
    return t == DataType::INTEGER || t == DataType::REAL;
}

bool SemanticAnalyzer::isComparableType(DataType t) const {
    return t == DataType::INTEGER || t == DataType::REAL  ||
           t == DataType::CHAR    || t == DataType::STRING ||
           t == DataType::BOOLEAN;
}

DataType SemanticAnalyzer::arithmeticResultType(DataType l, DataType r) const {
    if (l == DataType::REAL || r == DataType::REAL) return DataType::REAL;
    return DataType::INTEGER;
}

// =========================================================
// SemanticAnalyzer: visit(LiteralNode)
// Tipe langsung dari literalType
// =========================================================

void SemanticAnalyzer::visit(LiteralNode* node) {
    node->evalType    = node->literalType;
    node->lexicalLevel = st.currentLevel;
    // symRef tidak relevan untuk literal
}

// =========================================================
// SemanticAnalyzer: visit(VarAccessNode)
// Lookup identifier + validasi akses array/record
// =========================================================

void SemanticAnalyzer::visit(VarAccessNode* node) {
    TabEntry* entry = st.lookupTab(node->name);
    if (entry == nullptr) {
        semanticError("Identifier '" + node->name + "' belum dideklarasikan.");
        node->evalType = DataType::NOTYPE;
        return;
    }

    // Cek apakah identifier adalah variabel atau konstanta (bukan tipe/prosedur)
    if (entry->obj == ObjClass::TYPE_DEF) {
        semanticError("'" + node->name + "' adalah nama tipe, bukan variabel.");
        node->evalType = DataType::NOTYPE;
        return;
    }

    // Dapatkan indeks di tab untuk symRef
    // (Cari indeks dengan scan dari belakang — sama dengan lookupTab)
    for (int i = (int)st.tab.size() - 1; i >= 0; i--) {
        if (st.tab[i].identifiers == node->name) {
            node->symRef = i;
            break;
        }
    }

    node->evalType    = entry->type;
    node->lexicalLevel = entry->lev;

    // Validasi akses array (ada subscript)
    if (!node->indices.empty()) {
        if (entry->type != DataType::ARRAY) {
            semanticError("'" + node->name + "' bukan array, tidak bisa disubscript.");
            node->evalType = DataType::NOTYPE;
            return;
        }
        // Kunjungi setiap ekspresi indeks
        for (ASTNode* idxNode : node->indices) {
            idxNode->accept(this);
            if (idxNode->evalType != DataType::INTEGER &&
                idxNode->evalType != DataType::NOTYPE) {
                semanticError("Indeks array pada '" + node->name +
                              "' harus bertipe Integer.");
            }
        }
        // Tipe hasil = tipe elemen array
        ATabEntry* arrInfo = st.getATab(entry->ref);
        if (arrInfo != nullptr)
            node->evalType = arrInfo->etyp;
        else
            node->evalType = DataType::NOTYPE;
    }
}

// =========================================================
// SemanticAnalyzer: visit(FuncCallNode)
// Validasi deklarasi, jumlah argumen, dan tipe argumen
// =========================================================

void SemanticAnalyzer::visit(FuncCallNode* node) {
    TabEntry* entry = st.lookupTab(node->name);
    if (entry == nullptr) {
        semanticError("Prosedur/Fungsi '" + node->name + "' belum dideklarasikan.");
        node->evalType = DataType::NOTYPE;
        // Tetap kunjungi argumen untuk deteksi error lanjutan
        for (ASTNode* arg : node->args) if (arg) arg->accept(this);
        return;
    }

    if (entry->obj != ObjClass::FUNCTION && entry->obj != ObjClass::PROCEDURE) {
        semanticError("'" + node->name + "' bukan prosedur atau fungsi.");
        node->evalType = DataType::NOTYPE;
        for (ASTNode* arg : node->args) if (arg) arg->accept(this);
        return;
    }

    // Dapatkan tab index untuk symRef
    for (int i = (int)st.tab.size() - 1; i >= 0; i--) {
        if (st.tab[i].identifiers == node->name) {
            node->symRef = i;
            break;
        }
    }

    node->evalType    = entry->type;   // NONE untuk procedure, return type untuk function
    node->lexicalLevel = st.currentLevel;

    // Kunjungi semua argumen terlebih dahulu
    for (ASTNode* arg : node->args)
        if (arg) arg->accept(this);

    // Pengecualian built-in variadic (writeln, readln, write, read):
    // tidak divalidasi jumlah/tipe parameternya
    if (node->name == "writeln" || node->name == "readln" ||
        node->name == "write"   || node->name == "read") {
        return;
    }

    // Validasi parameter via btab
    BTabEntry* blockInfo = st.getBTab(entry->ref);
    if (blockInfo == nullptr) {
        // Prosedur/fungsi tanpa btab (predefined non-I/O): skip validasi
        return;
    }

    // Kumpulkan parameter dari linked list (berjalan mundur dari lpar)
    vector<TabEntry*> params;
    int currIdx = blockInfo->lpar;
    while (currIdx > 0) {
        TabEntry* pe = st.getTab(currIdx);
        if (!pe) break;
        params.push_back(pe);
        currIdx = pe->link;
    }
    reverse(params.begin(), params.end());

    // Validasi jumlah argumen
    if (node->args.size() != params.size()) {
        semanticError("Jumlah argumen pada pemanggilan '" + node->name +
                      "' tidak cocok. Diharapkan " + to_string(params.size()) +
                      " argumen, diberikan " + to_string(node->args.size()) + ".");
        node->evalType = DataType::NOTYPE;
        return;
    }

    // Validasi tipe argumen
    for (size_t i = 0; i < node->args.size(); i++) {
        DataType argType   = node->args[i]->evalType;
        DataType paramType = params[i]->type;

        if (argType == DataType::NOTYPE) continue; // Error sudah dicatat sebelumnya

        bool paramByRef = (params[i]->nrm == 0); // 0 = pass-by-reference (VAR param)

        if (paramByRef) {
            // VAR parameter: tipe harus sama persis
            if (argType != paramType) {
                semanticError("Argumen ke-" + to_string(i+1) +
                              " pada '" + node->name +
                              "' adalah VAR parameter, tipe harus sama persis. Diharapkan " +
                              ASTNode::dtStr(paramType) + ", diberikan " +
                              ASTNode::dtStr(argType) + ".");
                node->evalType = DataType::NOTYPE;
            }
        } else {
            // Value parameter: gunakan assignment-compatibility
            if (!isAssignmentCompatible(paramType, argType)) {
                semanticError("Tipe argumen ke-" + to_string(i+1) +
                              " pada '" + node->name + "' tidak kompatibel. Diharapkan " +
                              ASTNode::dtStr(paramType) + ", diberikan " +
                              ASTNode::dtStr(argType) + ".");
                node->evalType = DataType::NOTYPE;
            }
        }
    }
}

// =========================================================
// SemanticAnalyzer: visit(BinaryOpNode)
// Type checking operasi biner sesuai spek Lampiran C
// =========================================================

void SemanticAnalyzer::visit(BinaryOpNode* node) {
    // Kunjungi operand kiri dan kanan
    if (node->left)  node->left->accept(this);
    if (node->right) node->right->accept(this);

    DataType lType = node->left  ? node->left->evalType  : DataType::NOTYPE;
    DataType rType = node->right ? node->right->evalType : DataType::NOTYPE;
    const string& op = node->op;

    // Jika salah satu operand NOTYPE (error sebelumnya), propagasi NOTYPE
    if (lType == DataType::NOTYPE || rType == DataType::NOTYPE) {
        node->evalType = DataType::NOTYPE;
        return;
    }

    // --- Operator Aritmatika: +, -, * ---
    // Operand: Integer atau Real → Hasil: Integer atau Real (promosi ke Real jika perlu)
    if (op == "plus" || op == "minus" || op == "times" ||
        op == "+"    || op == "-"     || op == "*") {
        if (isNumericType(lType) && isNumericType(rType)) {
            node->evalType = arithmeticResultType(lType, rType);
        } else {
            semanticError("Operasi aritmatika '" + op +
                          "' membutuhkan operand Integer atau Real. Diberikan " +
                          ASTNode::dtStr(lType) + " dan " + ASTNode::dtStr(rType) + ".");
            node->evalType = DataType::NOTYPE;
        }
    }

    // --- Operator Pembagian Real: / (rdiv) ---
    // Operand: Integer atau Real → Hasil: Real (selalu)
    else if (op == "rdiv" || op == "/") {
        if (isNumericType(lType) && isNumericType(rType)) {
            node->evalType = DataType::REAL;
        } else {
            semanticError("Operator '/' membutuhkan operand Integer atau Real.");
            node->evalType = DataType::NOTYPE;
        }
    }

    // --- Operator Pembagian Bulat: div, mod ---
    // Operand: Integer saja → Hasil: Integer
    else if (op == "idiv" || op == "imod" || op == "div" || op == "mod") {
        if (lType == DataType::INTEGER && rType == DataType::INTEGER) {
            node->evalType = DataType::INTEGER;
        } else {
            semanticError("Operator '" + op +
                          "' membutuhkan dua operand Integer. Diberikan " +
                          ASTNode::dtStr(lType) + " dan " + ASTNode::dtStr(rType) + ".");
            node->evalType = DataType::NOTYPE;
        }
    }

    // --- Operator Relasional: =, <>, <, >, <=, >= ---
    // Operand: Integer, Real, Char, atau String (kompatibel) → Hasil: Boolean
    else if (op == "eql" || op == "neq" || op == "lss" || op == "gtr" ||
             op == "leq" || op == "geq"  ||
             op == "="   || op == "<>"   || op == "<"   ||
             op == ">"   || op == "<="   || op == ">=") {
        bool leftOk  = isComparableType(lType);
        bool rightOk = isComparableType(rType);
        bool typesCompatible = (lType == rType) ||
                               (isNumericType(lType) && isNumericType(rType));
        if (leftOk && rightOk && typesCompatible) {
            node->evalType = DataType::BOOLEAN;
        } else {
            semanticError("Operator relasional '" + op +
                          "' membutuhkan operand yang kompatibel (Integer/Real/Char/String). "
                          "Diberikan " + ASTNode::dtStr(lType) + " dan " +
                          ASTNode::dtStr(rType) + ".");
            node->evalType = DataType::NOTYPE;
        }
    }

    // --- Operator Logika: and, or ---
    // Operand: Boolean saja → Hasil: Boolean
    else if (op == "andsy" || op == "orsy" || op == "and" || op == "or") {
        if (lType == DataType::BOOLEAN && rType == DataType::BOOLEAN) {
            node->evalType = DataType::BOOLEAN;
        } else {
            semanticError("Operator logika '" + op +
                          "' membutuhkan operand Boolean. Diberikan " +
                          ASTNode::dtStr(lType) + " dan " + ASTNode::dtStr(rType) + ".");
            node->evalType = DataType::NOTYPE;
        }
    }

    else {
        semanticError("Operator tidak dikenal: '" + op + "'.");
        node->evalType = DataType::NOTYPE;
    }

    node->lexicalLevel = st.currentLevel;
}

// =========================================================
// SemanticAnalyzer: visit(UnaryOpNode)
// Type checking operasi unary sesuai spek Lampiran C
// =========================================================

void SemanticAnalyzer::visit(UnaryOpNode* node) {
    if (node->operand) node->operand->accept(this);
    DataType opType = node->operand ? node->operand->evalType : DataType::NOTYPE;

    if (opType == DataType::NOTYPE) {
        node->evalType = DataType::NOTYPE;
        return;
    }

    const string& op = node->op;

    // not: operand harus Boolean → hasil Boolean
    if (op == "notsy" || op == "not") {
        if (opType == DataType::BOOLEAN) {
            node->evalType = DataType::BOOLEAN;
        } else {
            semanticError("Operator 'not' membutuhkan operand Boolean, diberikan " +
                          ASTNode::dtStr(opType) + ".");
            node->evalType = DataType::NOTYPE;
        }
    }
    // Unary + atau -: operand harus Integer atau Real → hasil sama dengan operand
    else if (op == "plus" || op == "minus" || op == "+" || op == "-") {
        if (isNumericType(opType)) {
            node->evalType = opType;
        } else {
            semanticError("Unary '" + op +
                          "' membutuhkan operand Integer atau Real, diberikan " +
                          ASTNode::dtStr(opType) + ".");
            node->evalType = DataType::NOTYPE;
        }
    }
    else {
        semanticError("Operator unary tidak dikenal: '" + op + "'.");
        node->evalType = DataType::NOTYPE;
    }

    node->lexicalLevel = st.currentLevel;
}