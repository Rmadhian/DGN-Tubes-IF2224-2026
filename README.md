# Arion Compiler - Tugas Besar IF2224 TBFO (Milestone 4)

## Identitas Kelompok

**Kelompok DongunGanteng (DGN)**

| Nama | NIM |
| --- | --- |
| Benedictus Nelson | 13523150 |
| Rainaldi Pratama F. Sembiring | 13524126 |
| Jonathan Alveraldo Bangun | 13524120 |
| Ramadhian Nabil Firdaus Gumay | 13524126 |

## Deskripsi Program

Program ini merupakan tahapan keempat dan terakhir dari pembuatan compiler Arion, yaitu **Intermediate Code Generation (ICG) & Interpreter**. Program ini melanjutkan fungsionalitas dari Milestone 3 dengan melakukan penerjemahan *Decorated AST* menjadi kode instruksi tingkat rendah, lalu mengeksekusinya layaknya *Virtual Machine*.

Fitur utama pada Milestone 4 ini meliputi:

**Intermediate Code Generation**: Mengubah *Decorated AST* menjadi kumpulan instruksi mesin perantara (LIT, LOD, STO, CAL, INT, JMP, JPC, OPR, RET) menggunakan desain *Visitor Pattern*.

**Stack Machine Interpreter**: Mengeksekusi kumpulan instruksi TAC layaknya sebuah *Virtual Machine* berbasis *stack*, secara otomatis mengelola memori fungsi seperti *Static Link*, *Dynamic Link*, dan *Return Address*.

**Control Flow Flattening**: Meratakan struktur kontrol tingkat tinggi yang bersarang (seperti IF-THEN-ELSE, WHILE-DO, FOR-DO) menjadi instruksi lompatan linear bersyarat dan tak bersyarat (JMP/JPC).

**Vulnerability Handling (Bonus)**: Mencegah dan menangani secara aman kesalahan-kesalahan fatal *runtime* seperti *Stack Overflow*, *Stack Underflow*, akses memori *Out of Bounds*, *Invalid Jump Target*, *Numerical Overflow/Underflow*, dan *Division by Zero*.

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
*(Untuk OS Windows: Jika `make` tidak dikenali, gunakan `mingw32-make` atau kompilasi manual dengan perintah: `g++ -std=c++17 -I src/header src/core/*.cpp -o arion_lexer`)*

### 2. Menjalankan Program
Setelah berhasil dikompilasi, jalankan executable dengan format berikut:
```bash
./arion_lexer <path_ke_file_input> <path_ke_file_output>
```

**Contoh Penggunaan:**
```bash
./arion_lexer test/milestone-4/input-1.txt test/milestone-4/output-1.txt
```
Program akan membaca file input berformat Decorated AST (atau source code/parse tree dari milestone sebelumnya), membangkitkan kumpulan Intermediate Code, mencetaknya, dan mengeksekusi instruksi tersebut menggunakan Interpreter, menghasilkan output program ke layar dan file yang ditentukan.

## Pembagian Tugas
| NIM | Nama | Tugas |
| :--- | :--- | :---|
| 13523150 | Benedictus Nelson | Mengimplementasi Control Flow & Penanganan Keamanan/Vulnerability (`icg_stmt.cpp`, `interpreter_security.cpp`, `interpreter_flow.cpp`) & laporan. |
| 13524117 | Rainaldi Pratama F. Sembiring | - |
| 13524120 | Jonathan Alveraldo Bangun | Mengimplementasi Top-Level, Manajemen Scope & Core VM Engine (`icg_decl.cpp`, `interpreter_core.cpp`) & laporan. |
| 13524126 | Ramadhian Nabil Firdaus Gumay | Mengimplementasi Ekspresi, Assignment & Operasi Memori (`icg_expr.cpp`, `interpreter_expr.cpp`) & laporan. |