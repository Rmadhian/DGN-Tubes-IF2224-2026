#include "semantic.h"

void BinaryOpNode::accept(SemanticVisitor* visitor) { visitor->visit(this); }
void UnaryOpNode::accept(SemanticVisitor* visitor)   { visitor->visit(this); }
void LiteralNode::accept(SemanticVisitor* visitor)   { visitor->visit(this); }
void VarAccessNode::accept(SemanticVisitor* visitor)  { visitor->visit(this); }
void FuncCallNode::accept(SemanticVisitor* visitor)   { visitor->visit(this); }
void WriteStatementNode::accept(SemanticVisitor* visitor) { visitor->visit(this); }

// Mengevaluasi dan menetapkan tipe data untuk literal secara langsung.
void SemanticAnalyzer::visit(LiteralNode* node) {
    node->evalType = node->literalType;
    node->lexicalLevel = st.currentLevel;
}

// Mengevaluasi dan memvalidasi akses variabel, array, maupun record.
void SemanticAnalyzer::visit(VarAccessNode* node) {
    TabEntry* entry = st.lookupTab(node->name);
    if (entry == nullptr) {
        cout << "Error Semantik: Identifier '" << node->name << "' belum dideklarasikan!" << endl;
        node->evalType = DataType::NOTYPE;
        return;
    }

    node->symRef = (int)(entry - &st.tab[0]);
    node->evalType = entry->type;
    node->lexicalLevel = entry->lev;

    // Memvalidasi indeks array secara bertingkat untuk memastikan akses yang aman.
    if (!node->indices.empty()) {
        if (entry->type != DataType::ARRAY) {
            cout << "Error Semantik: '" << node->name << "' bukan sebuah Array!" << endl;
            node->evalType = DataType::NOTYPE;
        } else {
            // Mengevaluasi dan mengamankan setiap level dimensi array.
            for (ASTNode* idxNode : node->indices) {
                idxNode->accept(this);
                DataType idxt = idxNode->evalType;
                
                // Indeks array wajib menggunakan tipe data ordinal sederhana.
                if (idxt == DataType::REAL || idxt == DataType::ARRAY || idxt == DataType::RECORD) {
                    cout << "Error Semantik: Indeks Array harus bertipe simple (bukan Real/Komposit)!" << endl;
                }
            }
            // Memperbarui tipe data evaluasi ke tipe elemen dasar array setelah dimensi terlewati.
            ATabEntry* arrayInfo = st.getATab(entry->ref);
            if (arrayInfo != nullptr)
                node->evalType = arrayInfo->etyp;
        }
    }

    // Mengevaluasi dan memeriksa akses terhadap atribut komposit dalam record.
    if (!node->fieldName.empty()) {
        if (node->evalType != DataType::RECORD) {
            cout << "Error Semantik: '" << node->name << "' bukan sebuah Record!" << endl;
            node->evalType = DataType::NOTYPE;
        } else {
            // Menghubungkan scope pencarian ke tabel lokal record (btab).
            int bIndex = entry->ref; 
            if (bIndex > 0) {
                BTabEntry* recBlock = st.getBTab(bIndex);
                int currField = recBlock->last;
                bool found = false;
                
                // Menelusuri semua deklarasi field menggunakan static link.
                while (currField > 0) {
                    TabEntry* fieldEntry = st.getTab(currField);
                    
                    // Menemukan identifier dan memetakan tipe datanya.
                    if (fieldEntry->identifiers == node->fieldName) {
                        node->evalType = fieldEntry->type; 
                        found = true;
                        break;
                    }
                    
                    // Melakukan traversi mundur sepanjang linked list field.
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

// Mengevaluasi tipe dan argumen pada pemanggilan prosedur atau fungsi.
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

    // Mengembalikan NOTYPE untuk pemanggilan prosedur yang tidak menghasilkan nilai.
    node->evalType = entry->type;

    for (ASTNode* arg : node->args)
        arg->accept(this);

    // Mengabaikan pemeriksaan tipe kustom untuk I/O standar bawaan.
    if (node->name == "writeln" || node->name == "readln" || node->name == "write" || node->name == "read") {
        return; 
    }

    // Validasi kompatibilitas parameter input.
    
    // Menghubungkan dengan block table fungsi dari tab reference.
    BTabEntry* blockInfo = st.getBTab(entry->ref);
    
    if (blockInfo != nullptr) {
        std::vector<TabEntry*> params;
        int currParamIdx = blockInfo->lpar; 
        
        // Memetakan struktur urutan dan tipe parameter aktual.
        while (currParamIdx != 0) {
            TabEntry* paramEntry = st.getTab(currParamIdx);
            if (paramEntry == nullptr) break;
            
            params.push_back(paramEntry);
            currParamIdx = paramEntry->link; 
        }
        
        // Membalik urutan traversal statis untuk menyelaraskan indeks argumen.
        std::reverse(params.begin(), params.end());

        // Memastikan arity (jumlah parameter) sesuai dengan definisi.
        if (node->args.size() != params.size()) {
            cout << "Error Semantik: Jumlah argumen pada pemanggilan '" << node->name 
                 << "' tidak cocok! Diharapkan " << params.size() 
                 << " argumen, tapi diberikan " << node->args.size() << "." << endl;
            node->evalType = DataType::NOTYPE;
            return;
        }

        // Melakukan type checking komprehensif untuk setiap nilai yang disuplai.
        for (size_t i = 0; i < node->args.size(); ++i) {
            DataType argType = node->args[i]->evalType;
            DataType paramType = params[i]->type;

            if (argType == DataType::NOTYPE) continue;

            bool isCompatible = false;
            
            // Menentukan kompatibilitas strict.
            if (argType == paramType) {
                isCompatible = true;
            } 
            // Mengizinkan assignment cast implicit dari Integer ke Real (Pass-by-value).
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

// Mengevaluasi kompatibilitas tipe data untuk setiap jenis operator biner.
void SemanticAnalyzer::visit(BinaryOpNode* node) {
    node->left->accept(this);
    node->right->accept(this);

    DataType lType = node->left->evalType;
    DataType rType = node->right->evalType;
    string op = node->op;

    // Mengelola operasi aritmatika dasar.
    if (op == "plus" || op == "minus" || op == "times" || op == "+" || op == "-" || op == "*") {
        if ((lType == DataType::INTEGER || lType == DataType::REAL) &&
            (rType == DataType::INTEGER || rType == DataType::REAL)) {
            // Mempromosikan presisi operasi otomatis jika mengandung bilangan Real.
            node->evalType = (lType == DataType::REAL || rType == DataType::REAL)
                           ? DataType::REAL : DataType::INTEGER;
        } else {
            cout << "Error Semantik: Tipe data tidak cocok untuk operasi aritmatika '" << op << "'!" << endl;
            node->evalType = DataType::NOTYPE;
        }
    }
    // Memastikan operasi rdiv selalu menghasilkan nilai desimal koma mengambang.
    else if (op == "rdiv" || op == "/") {
        if ((lType == DataType::INTEGER || lType == DataType::REAL) &&
            (rType == DataType::INTEGER || rType == DataType::REAL)) {
            node->evalType = DataType::REAL;
        } else {
            cout << "Error Semantik: Operasi '/' hanya menerima tipe Integer/Real!" << endl;
            node->evalType = DataType::NOTYPE;
        }
    }
    // Menjamin operasi modulo dan pembagian bulat hanya melibatkan Integer murni.
    else if (op == "idiv" || op == "imod") {
        if (lType == DataType::INTEGER && rType == DataType::INTEGER) {
            node->evalType = DataType::INTEGER;
        } else {
            cout << "Error Semantik: Operasi '" << op << "' wajib menggunakan dua tipe Integer!" << endl;
            node->evalType = DataType::NOTYPE;
        }
    }
    // Operasi komparasi relasional dinamis untuk string dan numerik menghasilkan Boolean.
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
    // Validasi ketat operasi logika dengan operand yang murni Boolean.
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

// Mengevaluasi dan memastikan validitas tipe data pada operasi prefix unary.
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