#include "semantic.h"

void BinaryOpNode::accept(SemanticVisitor* visitor) {
    visitor->visit(this);
}

void UnaryOpNode::accept(SemanticVisitor* visitor) {
    visitor->visit(this);
}

void LiteralNode::accept(SemanticVisitor* visitor) {
    visitor->visit(this);
}

void VarAccessNode::accept(SemanticVisitor* visitor) {
    visitor->visit(this);
}

void FuncCallNode::accept(SemanticVisitor* visitor) {
    visitor->visit(this);
}

// Validasi Literal (Angka, String, Char, Boolean)
void SemanticAnalyzer::visit(LiteralNode* node) {
    // Literal langsung mendapatkan tipe data aslinya
    node->evalType = node->literalType;
    node->lexicalLevel = st.currentLevel;
}

// Validasi Pemanggilan Variabel (VarAccess)
void SemanticAnalyzer::visit(VarAccessNode* node) {
    // Cek Undeclared Variable
    TabEntry* entry = st.lookupTab(node->name);
    if (entry == nullptr) {
        cout << "Error Semantik: Identifier '" << node->name << "' belum dideklarasikan!" << endl;
        node->evalType = DataType::NOTYPE;
        return;
    }

    // Jika lolos, ambil informasi dari Symbol Table
    node->symRef = entry->link;
    node->evalType = entry->type;
    node->lexicalLevel = st.currentLevel;

    // Tambahan: Jika ini pemanggilan Array (punya index)
    if (!node->indices.empty()) {
        if (entry->type != DataType::ARRAY) {
            cout << "Error Semantik: '" << node->name << "' bukan sebuah Array!" << endl;
            node->evalType = DataType::NOTYPE;
        } else {
            // Evaluasi setiap indeks di dalam kurung siku
            for (ASTNode* idxNode : node->indices) {
                idxNode->accept(this);
                if (idxNode->evalType != DataType::INTEGER) {
                    cout << "Error Semantik: Indeks Array harus bernilai Integer!" << endl;
                }
            }
            // Tipe data hasil akses array adalah tipe elemennya
            ATabEntry* arrayInfo = st.getATab(entry->ref);
            if (arrayInfo != nullptr) {
                node->evalType = arrayInfo->etyp;
            }
        }
    }
}

// Validasi Pemanggilan Fungsi (FuncCall)
void SemanticAnalyzer::visit(FuncCallNode* node) {
    // Cek apakah fungsi ada di tabel
    TabEntry* entry = st.lookupTab(node->name);
    if (entry == nullptr) {
        cout << "Error Semantik: Prosedur/Fungsi '" << node->name << "' belum dideklarasikan!" << endl;
        node->evalType = DataType::NOTYPE;
        return;
    }

    // Cek apakah yang dipanggil benar-benar fungsi/prosedur
    if (entry->obj != ObjClass::FUNCTION && entry->obj != ObjClass::PROCEDURE) {
        cout << "Error Semantik: '" << node->name << "' bukan Prosedur atau Fungsi dan tidak bisa dipanggil!" << endl;
        node->evalType = DataType::NOTYPE;
        return;
    }

    // Tipe hasil evaluasi = tipe kembalian fungsi (Procedure akan bernilai NOTYPE)
    node->evalType = entry->type;

    // Evaluasi semua argumen yang dikirim
    for (ASTNode* arg : node->args) {
        arg->accept(this);
    }
    // (Pengecekan kesesuaian jumlah parameter dan argumen bisa dikembangkan dengan mengecek ke tabel btab)
}

// Validasi Operasi Matematika & Logika Biner (BinaryOp)
void SemanticAnalyzer::visit(BinaryOpNode* node) {
    // Evaluasi anak kiri dan kanan terlebih dahulu (Bottom-Up)
    node->left->accept(this);
    node->right->accept(this);

    DataType lType = node->left->evalType;
    DataType rType = node->right->evalType;
    string op = node->op;

    // Operator Aritmatika Umum (+, -, *)
    if (op == "plus" || op == "minus" || op == "times" || op == "+" || op == "-" || op == "*") {
        if ((lType == DataType::INTEGER || lType == DataType::REAL) &&
            (rType == DataType::INTEGER || rType == DataType::REAL)) {
            // Jika salah satu adalah Real, hasilnya ikut Real
            if (lType == DataType::REAL || rType == DataType::REAL) {
                node->evalType = DataType::REAL;
            } else {
                node->evalType = DataType::INTEGER;
            }
        } else {
            cout << "Error Semantik: Tipe data tidak cocok untuk operasi Aritmatika '" << op << "'!" << endl;
            node->evalType = DataType::NOTYPE;
        }
    }
    // Pembagian Real (/)
    else if (op == "rdiv" || op == "/") {
        if ((lType == DataType::INTEGER || lType == DataType::REAL) &&
            (rType == DataType::INTEGER || rType == DataType::REAL)) {
            node->evalType = DataType::REAL; // Hasil selalu Real
        } else {
            cout << "Error Semantik: Operasi pembagian Real (/) hanya menerima tipe Integer/Real!" << endl;
            node->evalType = DataType::NOTYPE;
        }
    }
    // Pembagian Bulat & Modulo (div, mod)
    else if (op == "idiv" || op == "imod") {
        if (lType == DataType::INTEGER && rType == DataType::INTEGER) {
            node->evalType = DataType::INTEGER;
        } else {
            cout << "Error Semantik: Operasi '" << op << "' wajib menggunakan dua tipe Integer!" << endl;
            node->evalType = DataType::NOTYPE;
        }
    }
    // Operator Relasional (=, <>, <, >, <=, >=)
    else if (op == "eql" || op == "neq" || op == "lss" || op == "gtr" || op == "leq" || op == "geq" || op == "=" || op == "<" || op == ">") {
        // Tipe harus persis sama (khusus Char/String) ATAU sesama angka (Int vs Real diperbolehkan)
        if (lType == rType && (lType == DataType::INTEGER || lType == DataType::REAL || lType == DataType::CHAR || lType == DataType::STRING)) {
             node->evalType = DataType::BOOLEAN;
        } else if ((lType == DataType::INTEGER || lType == DataType::REAL) && (rType == DataType::INTEGER || rType == DataType::REAL)) {
             node->evalType = DataType::BOOLEAN;
        } else {
             cout << "Error Semantik: Tipe data kiri dan kanan tidak sepadan untuk operator relasional '" << op << "'!" << endl;
             node->evalType = DataType::NOTYPE;
        }
    }
    // Operator Logika (and, or)
    else if (op == "andsy" || op == "orsy" || op == "and" || op == "or") {
        if (lType == DataType::BOOLEAN && rType == DataType::BOOLEAN) {
            node->evalType = DataType::BOOLEAN;
        } else {
            cout << "Error Semantik: Operasi Logika '" << op << "' wajib menggunakan tipe Boolean!" << endl;
            node->evalType = DataType::NOTYPE;
        }
    }
    else {
        node->evalType = DataType::NOTYPE;
    }
}

// Validasi Operasi Unary (UnaryOp)
void SemanticAnalyzer::visit(UnaryOpNode* node) {
    node->operand->accept(this);
    DataType opType = node->operand->evalType;
    string op = node->op;

    // Operator Unary Logika (NOT)
    if (op == "notsy" || op == "not") {
        if (opType == DataType::BOOLEAN) {
            node->evalType = DataType::BOOLEAN;
        } else {
            cout << "Error Semantik: Operator 'not' wajib disandingkan dengan tipe Boolean!" << endl;
            node->evalType = DataType::NOTYPE;
        }
    } 
    // Operator Unary Tanda Aritmatika (+, -)
    else if (op == "plus" || op == "minus" || op == "+" || op == "-") {
        if (opType == DataType::INTEGER || opType == DataType::REAL) {
            node->evalType = opType;
        } else {
            cout << "Error Semantik: Tanda unary aritmatika wajib disandingkan dengan Integer atau Real!" << endl;
            node->evalType = DataType::NOTYPE;
        }
    }
}