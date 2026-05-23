Berikut adalah draf README untuk Milestone 3 berdasarkan spesifikasi yang Anda berikan:

# Arion Compiler - Tugas Besar IF2224 TBFO (Milestone 3)

## Identitas Kelompok

**Kelompok DongunGanteng (DGN)**

| Nama | NIM |
| --- | --- |
| Benedictus Nelson | 13523150 |
| Rainaldi Pratama F. Sembiring | 13524126 |
| Jonathan Alveraldo Bangun | 13524120 |
| Ramadhian Nabil Firdaus Gumay | 13524126 |

## Deskripsi Program

Program ini merupakan tahapan ketiga dari pembuatan compiler Arion, yaitu **Semantic Analysis**. Program ini melanjutkan fungsionalitas dari parser pada Milestone 2 dengan menambahkan pengecekan makna (*semantic check*) terhadap *parse tree* yang telah dihasilkan.

Fitur utama pada Milestone 3 ini meliputi:

**Konversi Parse Tree ke Decorated AST**: Mengubah *parse tree* menjadi *Abstract Syntax Tree* (AST) yang ringkas dan melakukan anotasi informasi tambahan (seperti tipe data dan referensi simbol).

**Symbol Table Management**: Implementasi struktur data *stack* untuk mengelola cakupan (*scope*) variabel/fungsi menggunakan tiga tabel utama: `tab`, `btab`, dan `atab`.


**Type & Scope Checking**: Melakukan verifikasi kompatibilitas tipe data berdasarkan aturan *Type Compatibility* dan memastikan validitas akses identifier berdasarkan hierarki blok kode.


**Recursive Descent Visitor**: Penelusuran *parse tree* secara *top-down* menggunakan fungsi *visit* untuk membangun *Decorated AST*.

## Requirements
Untuk melakukan kompilasi dan menjalankan program ini, pastikan sistem Anda sudah terpasang:
* **G++ Compiler** (Mendukung standar C++11 ke atas).
* **Make** (Opsional, untuk kompilasi otomatis melalui Makefile. Pengguna Windows dengan MinGW dapat menggunakan `mingw32-make`).

## Cara Instalasi dan Penggunaan Program

### 1. Kompilasi Program
Buka terminal pada *root directory* proyek ini, lalu jalankan perintah `make`:
```bash
make
```
*(Untuk OS Windows: Jika `make` tidak dikenali, gunakan `mingw32-make` atau kompilasi manual dengan perintah: `g++ src/main.cpp src/dfa_dinamis.cpp src/dfa_statis.cpp src/lexer.cpp src/parser_expr.cpp src/parser_stmt.cpp src/parser_decl.cpp src/ast_builder.cpp src/semantic_decl.cpp src/semantic_expr.cpp src/semantic_stmt.cpp src/semantic_printer.cpp -o arion_lexer`)*

### 2. Menjalankan Program
Setelah berhasil dikompilasi, jalankan executable dengan format berikut:
```bash
./arion_lexer <path_ke_file_input> <path_ke_file_output>
```

**Contoh Penggunaan:**
```bash
./arion_lexer test/milestone-3/input-1.txt test/milestone-3/output-1.txt
```
Program akan membaca parse tree hasil dari Milestone 2, melakukan analisis semantik, dan menghasilkan Decorated AST serta Symbol Table ke dalam file output yang ditentukan.

## Pembagian Tugas
| NIM | Nama | Tugas |
| :--- | :--- | :---|
| 13523150 | Benedictus Nelson | Mengimplementasi semantic_stmt.cpp & laporan. |
| 13524117 | Rainaldi Pratama F. Sembiring | - |
| 13524120 | Jonathan Alveraldo Bangun | Mengimplementasi semantic_decl.cpp, ast_builder & laporan. |
| 13524126 | Ramadhian Nabil Firdaus Gumay | Mengimplementasi semantic_expr.cpp, semantic_printer.cpp & laporan. |