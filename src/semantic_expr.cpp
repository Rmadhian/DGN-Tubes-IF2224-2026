#include "semantic.h"

void BinaryOpNode::accept(SemanticVisitor* visitor) { visitor->visit(this); }
void UnaryOpNode::accept(SemanticVisitor* visitor)   { visitor->visit(this); }
void LiteralNode::accept(SemanticVisitor* visitor)   { visitor->visit(this); }
void VarAccessNode::accept(SemanticVisitor* visitor)  { visitor->visit(this); }
void FuncCallNode::accept(SemanticVisitor* visitor)   { visitor->visit(this); }

// Visitor: LiteralNode — tipe langsung diambil dari literalType
void SemanticAnalyzer::visit(LiteralNode* node) {
    node->evalType = node->literalType;
    node->lexicalLevel = st.currentLevel;
}

// Visitor: VarAccessNode — lookup identifier dan validasi akses array
void SemanticAnalyzer::visit(VarAccessNode* node) {
    TabEntry* entry = st.lookupTab(node->name);
    if (entry == nullptr) {
        cout << "Error Semantik: Identifier '" << node->name << "' belum dideklarasikan!" << endl;
        node->evalType = DataType::NOTYPE;
        return;
    }

    node->symRef = entry->link;
    node->evalType = entry->type;
    node->lexicalLevel = st.currentLevel;

    // Memeriksa dan mengevaluasi akses indeks array (jika identifier memiliki subscript)
    if (!node->indices.empty()) {
        if (entry->type != DataType::ARRAY) {
            cout << "Error Semantik: '" << node->name << "' bukan sebuah Array!" << endl;
            node->evalType = DataType::NOTYPE;
        } else {
            // Evaluasi setiap ekspresi indeks yang diberikan
            for (ASTNode* idxNode : node->indices) {
                idxNode->accept(this);
                DataType idxt = idxNode->evalType;
                
                // Indeks array pada Pascal hanya boleh berupa tipe ordinal sederhana (Integer/Char/Boolean/Enum)
                if (idxt == DataType::REAL || idxt == DataType::ARRAY || idxt == DataType::RECORD) {
                    cout << "Error Semantik: Indeks Array harus bertipe simple (bukan Real/Komposit)!" << endl;
                }
            }
            // Setelah semua indeks valid, tetapkan tipe hasil evaluasi menjadi tipe elemen dari array tersebut
            ATabEntry* arrayInfo = st.getATab(entry->ref);
            if (arrayInfo != nullptr)
                node->evalType = arrayInfo->etyp;
        }
    }

    // Mengevaluasi akses field pada tipe record (contoh: myRecord.myField)
    if (!node->fieldName.empty()) {
        if (node->evalType != DataType::RECORD) {
            cout << "Error Semantik: '" << node->name << "' bukan sebuah Record!" << endl;
            node->evalType = DataType::NOTYPE;
        } else {
            // Ambil referensi ke block table (btab) yang menyimpan definisi field record ini
            int bIndex = entry->ref; 
            if (bIndex > 0) {
                BTabEntry* recBlock = st.getBTab(bIndex);
                int currField = recBlock->last;
                bool found = false;
                
                // Menelusuri semua field yang terdaftar dalam scope record secara mundur (dari akhir ke awal)
                while (currField > 0) {
                    TabEntry* fieldEntry = st.getTab(currField);
                    
                    // Jika nama field cocok dengan yang diakses oleh node
                    if (fieldEntry->identifiers == node->fieldName) {
                        node->evalType = fieldEntry->type; 
                        found = true;
                        break;
                    }
                    
                    // Lanjut periksa field sebelumnya melalui tautan link
                    currField = fieldEntry->link;
                }
                
                if (!found) {
                    cout << "Error Semantik: Field '" << node->fieldName 
                         << "' tidak ditemukan dalam record '" << node->name << "'!" << endl;
                    node->evalType = DataType::NOTYPE;
                }
            }
        }
    }
}

// Visitor: FuncCallNode — validasi deklarasi dan tipe callable
void SemanticAnalyzer::visit(FuncCallNode* node) {
    TabEntry* entry = st.lookupTab(node->name);
    if (entry == nullptr) {
        cout << "Error Semantik: Prosedur/Fungsi '" << node->name << "' belum dideklarasikan!" << endl;
        node->evalType = DataType::NOTYPE;
        return;
    }

    if (entry->obj != ObjClass::FUNCTION && entry->obj != ObjClass::PROCEDURE) {
        cout << "Error Semantik: '" << node->name << "' bukan Prosedur atau Fungsi!" << endl;
        node->evalType = DataType::NOTYPE;
        return;
    }

    // Tipe evaluasi = tipe kembalian (NOTYPE untuk procedure)
    node->evalType = entry->type;

    for (ASTNode* arg : node->args)
        arg->accept(this);

    // Skip type checking untuk fungsi bawaan
    if (node->name == "writeln" || node->name == "readln" || node->name == "write" || node->name == "read") {
        return; 
    }

    // --- Validasi Parameter ---
    
    // Ambil detail blok prosedur/fungsi dari btab
    BTabEntry* blockInfo = st.getBTab(entry->ref);
    
    if (blockInfo != nullptr) {
        std::vector<TabEntry*> params;
        int currParamIdx = blockInfo->lpar; 
        
        // Kumpulkan parameter dari tabel
        while (currParamIdx != 0) {
            TabEntry* paramEntry = st.getTab(currParamIdx);
            if (paramEntry == nullptr) break;
            
            params.push_back(paramEntry);
            currParamIdx = paramEntry->link; 
        }
        
        // Sesuaikan urutan parameter agar urut dari kiri ke kanan
        std::reverse(params.begin(), params.end());

        // Periksa kesesuaian jumlah parameter
        if (node->args.size() != params.size()) {
            cout << "Error Semantik: Jumlah argumen pada pemanggilan '" << node->name 
                 << "' tidak cocok! Diharapkan " << params.size() 
                 << " argumen, tapi diberikan " << node->args.size() << "." << endl;
            node->evalType = DataType::NOTYPE;
            return;
        }

        // Periksa kompatibilitas tipe argumen
        for (size_t i = 0; i < node->args.size(); ++i) {
            DataType argType = node->args[i]->evalType;
            DataType paramType = params[i]->type;

            if (argType == DataType::NOTYPE) continue;

            bool isCompatible = false;
            
            // Tipe data sama persis
            if (argType == paramType) {
                isCompatible = true;
            } 
            // Assignment compatibility: Integer bisa di-passing ke parameter Real
            else if (paramType == DataType::REAL && argType == DataType::INTEGER) {
                if (params[i]->nrm == 1) { 
                    isCompatible = true;
                } else {
                    cout << "Error Semantik: Argumen ke-" << i+1 
                         << " pada '" << node->name << "' adalah pass-by-reference (VAR), tipe tidak boleh di-cast dan harus sama persis!" << endl;
                }
            }
            
            if (!isCompatible) {
                cout << "Error Semantik: Tipe data argumen ke-" << i+1 
                     << " tidak sepadan dengan deklarasi parameter pada '" << node->name << "'!" << endl;
                node->evalType = DataType::NOTYPE;
            }
        }
    } else {
         cout << "Error Semantik: Definisi blok eksekusi untuk '" << node->name << "' gagal ditemukan di dalam btab!" << endl;
         node->evalType = DataType::NOTYPE;
    }
}

// Visitor: BinaryOpNode — type checking operasi biner
void SemanticAnalyzer::visit(BinaryOpNode* node) {
    node->left->accept(this);
    node->right->accept(this);

    DataType lType = node->left->evalType;
    DataType rType = node->right->evalType;
    string op = node->op;

    // Aritmatika: +, -, *
    if (op == "plus" || op == "minus" || op == "times" || op == "+" || op == "-" || op == "*") {
        if ((lType == DataType::INTEGER || lType == DataType::REAL) &&
            (rType == DataType::INTEGER || rType == DataType::REAL)) {
            // Promosi ke Real jika salah satu operand Real
            node->evalType = (lType == DataType::REAL || rType == DataType::REAL)
                           ? DataType::REAL : DataType::INTEGER;
        } else {
            cout << "Error Semantik: Tipe data tidak cocok untuk operasi aritmatika '" << op << "'!" << endl;
            node->evalType = DataType::NOTYPE;
        }
    }
    // Pembagian real (/)
    else if (op == "rdiv" || op == "/") {
        if ((lType == DataType::INTEGER || lType == DataType::REAL) &&
            (rType == DataType::INTEGER || rType == DataType::REAL)) {
            node->evalType = DataType::REAL;
        } else {
            cout << "Error Semantik: Operasi '/' hanya menerima tipe Integer/Real!" << endl;
            node->evalType = DataType::NOTYPE;
        }
    }
    // Pembagian bulat dan modulo (div, mod) — khusus Integer
    else if (op == "idiv" || op == "imod") {
        if (lType == DataType::INTEGER && rType == DataType::INTEGER) {
            node->evalType = DataType::INTEGER;
        } else {
            cout << "Error Semantik: Operasi '" << op << "' wajib menggunakan dua tipe Integer!" << endl;
            node->evalType = DataType::NOTYPE;
        }
    }
    // Relasional (=, <>, <, >, <=, >=) — hasil Boolean
    else if (op == "eql" || op == "neq" || op == "lss" || op == "gtr" ||
             op == "leq" || op == "geq" || op == "=" || op == "<" || op == ">") {
        if (lType == rType && (lType == DataType::INTEGER || lType == DataType::REAL ||
            lType == DataType::CHAR || lType == DataType::STRING)) {
             node->evalType = DataType::BOOLEAN;
        } else if ((lType == DataType::INTEGER || lType == DataType::REAL) &&
                   (rType == DataType::INTEGER || rType == DataType::REAL)) {
             node->evalType = DataType::BOOLEAN;
        } else {
             cout << "Error Semantik: Tipe tidak sepadan untuk operator relasional '" << op << "'!" << endl;
             node->evalType = DataType::NOTYPE;
        }
    }
    // Logika (and, or) — khusus Boolean
    else if (op == "andsy" || op == "orsy" || op == "and" || op == "or") {
        if (lType == DataType::BOOLEAN && rType == DataType::BOOLEAN) {
            node->evalType = DataType::BOOLEAN;
        } else {
            cout << "Error Semantik: Operasi logika '" << op << "' wajib menggunakan tipe Boolean!" << endl;
            node->evalType = DataType::NOTYPE;
        }
    }
    else {
        node->evalType = DataType::NOTYPE;
    }
}

// Visitor: UnaryOpNode — type checking operasi unary
void SemanticAnalyzer::visit(UnaryOpNode* node) {
    node->operand->accept(this);
    DataType opType = node->operand->evalType;
    string op = node->op;

    if (op == "notsy" || op == "not") {
        if (opType == DataType::BOOLEAN)
            node->evalType = DataType::BOOLEAN;
        else {
            cout << "Error Semantik: Operator 'not' membutuhkan operand Boolean!" << endl;
            node->evalType = DataType::NOTYPE;
        }
    } 
    else if (op == "plus" || op == "minus" || op == "+" || op == "-") {
        if (opType == DataType::INTEGER || opType == DataType::REAL)
            node->evalType = opType;
        else {
            cout << "Error Semantik: Unary sign membutuhkan operand Integer atau Real!" << endl;
            node->evalType = DataType::NOTYPE;
        }
    }
}