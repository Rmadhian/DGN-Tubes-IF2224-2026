# Arion Lexer - Tugas Besar IF2224 TBFO (Milestone 1)

## Identitas Kelompok
**Kelompok DongunGanteng (DGN)**
| Nama | NIM |
| :--- | :--- |
| Benedictus Nelson | 13523150 |
| Rainaldi Pratama F. Sembiring | 13524117 |
| Jonathan Alveraldo Bangun | 13524120 |
| Ramadhian Nabil Firdaus Gumay | 13524126 |

## Deskripsi Program
Program ini adalah sebuah **Lexical Analyzer (Lexer)** untuk bahasa pemrograman simulasi bernama **Arion**. Lexer ini dibangun menggunakan bahasa C++ dan bekerja murni berdasarkan prinsip **Deterministic Finite Automata (DFA)**. 

Program akan membaca *source code* berupa file `.txt` karakter demi karakter (tanpa membaca mundur), mengenali pola-pola karakter (seperti angka, *string*, *identifier*, *keyword*, dan simbol), lalu mengonversinya menjadi sekumpulan *token* yang divalidasi sesuai spesifikasi Milestone 1 IF2224. Program juga sudah dilengkapi dengan *error handling* untuk argumen terminal dan pembacaan file I/O.

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
*(Untuk OS Windows: Jika `make` tidak dikenali, gunakan `mingw32-make` atau kompilasi manual dengan perintah: `g++ src/main.cpp src/lexer.cpp src/dfa_dinamis.cpp src/dfa_statis.cpp -o arion_lexer`)*

### 2. Menjalankan Program
Setelah berhasil dikompilasi, sebuah *executable* bernama `arion_lexer` (atau `arion_lexer.exe` di Windows) akan muncul. Jalankan program dengan format berikut:
```bash
./arion_lexer <path_ke_file_input> <path_ke_file_output>
```

**Contoh Penggunaan:**
```bash
./arion_lexer test/milestone-1/input-1.txt test/milestone-1/output-1.txt
```
Program akan membaca kode Arion dari `input-1.txt` dan menghasilkan daftar token ke dalam `output-1.txt` di dalam direktori test/milestone-1/ .

## Pembagian Tugas
| NIM | Nama | Tugas |
| :--- | :--- | :---|
| 13523150 | Benedictus Nelson | Mengerjakan dfa_statis.cpp. |
| 13524117 | Rainaldi Pratama F. Sembiring | - |
| 13524120 | Jonathan Alveraldo Bangun|Mengerjakan dfa_dinamis.cpp, mengerjakan laporan. |
|13524126| Ramadhian Nabil Firdaus Gumay | Membuat lexer.h, mengerjakan lexer.cpp, main.cpp, Makefile, membuat diagram, dan laporan. |
