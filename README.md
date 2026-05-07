# Arion Parser - Tugas Besar IF2224 TBFO (Milestone 2)

## Identitas Kelompok
**Kelompok DongunGanteng (DGN)**
| Nama | NIM |
| :--- | :--- |
| Benedictus Nelson | 13523150 |
| Rainaldi Pratama F. Sembiring | 13524117 |
| Jonathan Alveraldo Bangun | 13524120 |
| Ramadhian Nabil Firdaus Gumay | 13524126 |

## Deskripsi Program
Program ini adalah kelanjutan dari Milestone 1 yang sekarang bertindak sebagai **Syntax Analyzer (Parser)** untuk bahasa pemrograman simulasi bernama **Arion**. Parser ini dibangun menggunakan bahasa C++ dan mengimplementasikan algoritma **Recursive Descent Parser**. 

Program akan memproses input (baik berupa *source code* murni maupun file hasil tokenisasi M1) menjadi deretan token. Kemudian, Parser akan memeriksa deretan token tersebut berdasarkan aturan tata bahasa (Grammar) spesifikasi Milestone 2 untuk merakit dan mencetak sebuah **Parse Tree**.

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
*(Untuk OS Windows: Jika `make` tidak dikenali, gunakan `mingw32-make` atau kompilasi manual dengan perintah: `g++ src/main.cpp src/lexer.cpp src/dfa_dinamis.cpp src/dfa_statis.cpp src/parser_decl.cpp src/parser_expr.cpp src/parser_stmt.cpp -o arion_lexer`)*

### 2. Menjalankan Program
Setelah berhasil dikompilasi, sebuah *executable* bernama `arion_lexer` (atau `arion_lexer.exe` di Windows) akan muncul. Jalankan program dengan format berikut:
```bash
./arion_lexer <path_ke_file_input> <path_ke_file_output>
```

**Contoh Penggunaan:**
```bash
./arion_lexer test/milestone-2/input-1.txt test/milestone-2/output-1.txt
```
Program akan membaca kode dari `input-1.txt` dan menghasilkan pohon sintaks (Parse Tree) ke dalam file `output-1.txt` di dalam direktori `test/milestone-2/`.

## Pembagian Tugas
| NIM | Nama | Tugas |
| :--- | :--- | :---|
| 13523150 | Benedictus Nelson | Mengimplementasikan parser_stmt.cpp, mengerjakan laporan. |
| 13524117 | Rainaldi Pratama F. Sembiring | - |
| 13524120 | Jonathan Alveraldo Bangun | Mengimplementasikan parser_decl.cpp, membuat laporan. |
| 13524126 | Ramadhian Nabil Firdaus Gumay | Menginisialisasi parser.h, mengimplementasikan parser_expr.cpp, integrasi program, membuat laporan. |