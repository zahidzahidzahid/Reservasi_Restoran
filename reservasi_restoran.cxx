#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
#include <cstdlib>
#include <sstream>
using namespace std;

// KONSTANTA

const int    MAX_RESERVASI = 200;
const int    TOTAL_MEJA    = 9;
const string FILE_DB       = "reservasi.txt";
const string ADMIN_USER    = "admin";
const string ADMIN_PASS    = "admin";
const int    MAX_LOGIN     = 3;

const string DAFTAR_MEJA[TOTAL_MEJA] = {
    "1A","1B","1C",
    "2A","2B","2C",
    "3A","3B","3C"
};
// STRUCT
struct Reservasi {
    int    id;
    string kodeMeja;
    string tanggal;   // DD/MM/YYYY
    string jam;       // HH:MM
    string namaPelanggan;
    bool   aktif;
};
// GLOBAL DATA
Reservasi data[MAX_RESERVASI];
int jumlah    = 0;
int idCounter = 1;
// UTILITAS UI
void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void garisTebal() { cout << "  ========================================" << endl; }
void garisTipis() { cout << "  ----------------------------------------" << endl; }

void header(const string& judul) {
    garisTebal();
    cout << "    " << judul << endl;
    garisTebal();
}

void pressEnter() {
    cout << "\n  Tekan Enter untuk kembali ke menu...";
    cin.get();
}

string toUpper(string s) {
    for (int i = 0; i < (int)s.size(); i++)
        s[i] = toupper((unsigned char)s[i]);
    return s;
}

string formatBooking(int id) {
    ostringstream oss;
    oss << "RST-" << setfill('0') << setw(4) << id;
    return oss.str();
}
// VALIDASI TANGGAL & JAM

// Cek apakah string hanya digit semua
bool semuaDigit(const string& s) {
    if (s.empty()) return false;
    for (int i = 0; i < (int)s.size(); i++)
        if (s[i] < '0' || s[i] > '9') return false;
    return true;
}

// Validasi format DD/MM/YYYY + range hari 01-31, bulan 01-12
bool tanggalValid(const string& tgl) {
    // Harus tepat 10 karakter: DD/MM/YYYY
    if (tgl.size() != 10) return false;
    if (tgl[2] != '/' || tgl[5] != '/') return false;

    string dd   = tgl.substr(0, 2);
    string mm   = tgl.substr(3, 2);
    string yyyy = tgl.substr(6, 4);

    if (!semuaDigit(dd) || !semuaDigit(mm) || !semuaDigit(yyyy)) return false;

    int hari  = atoi(dd.c_str());
    int bulan = atoi(mm.c_str());
    int tahun = atoi(yyyy.c_str());

    if (hari  < 1 || hari  > 31) return false;
    if (bulan < 1 || bulan > 12) return false;
    // Tidak ada batasan tahun minimal

    // Hari max per bulan (sederhana)
    int maxHari = 31;
    if (bulan == 4 || bulan == 6 || bulan == 9 || bulan == 11) maxHari = 30;
    else if (bulan == 2) {
        bool kabisat = (tahun % 4 == 0 && tahun % 100 != 0) || (tahun % 400 == 0);
        maxHari = kabisat ? 29 : 28;
    }
    if (hari > maxHari) return false;

    return true;
}

// Validasi format HH:MM, jam 00-23, menit 00-59
bool jamValid(const string& j) {
    if (j.size() != 5) return false;
    if (j[2] != ':') return false;

    string hh = j.substr(0, 2);
    string mm = j.substr(3, 2);

    if (!semuaDigit(hh) || !semuaDigit(mm)) return false;

    int jam   = atoi(hh.c_str());
    int menit = atoi(mm.c_str());

    if (jam   < 0 || jam   > 23) return false;
    if (menit < 0 || menit > 59) return false;
    return true;
}

// Input tanggal dengan loop validasi
string inputTanggalValid(const string& label) {
    string tgl;
    while (true) {
        cout << "  " << label << " (DD/MM/YYYY) : ";
        getline(cin, tgl);
        if (tanggalValid(tgl)) break;
        cout << "  [!] Format salah atau tanggal tidak valid." << endl;
        cout << "      Format : DD/MM/YYYY  (contoh: 25/12/2026)" << endl;
        cout << "      Hari   : 01-31  |  Bulan: 01-12" << endl;
    }
    return tgl;
}

// Input jam dengan loop validasi
string inputJamValid(const string& label) {
    string j;
    while (true) {
        cout << "  " << label << " (HH:MM)     : ";
        getline(cin, j);
        if (jamValid(j)) break;
        cout << "  [!] Format salah. Jam: 00-23  |  Menit: 00-59" << endl;
    }
    return j;
}
// CEK MEJA
bool mejaValid(const string& kode) {
    for (int i = 0; i < TOTAL_MEJA; i++)
        if (DAFTAR_MEJA[i] == kode) return true;
    return false;
}

int hitungMejaTerisi(const string& tanggal) {
    int count = 0;
    for (int i = 0; i < jumlah; i++)
        if (data[i].aktif && data[i].tanggal == tanggal)
            count++;
    return count;
}

// Cek meja + jam bentrok
bool mejaTersedia(const string& kodeMeja, const string& tanggal, const string& jam, int kecualiId = -1) {
    for (int i = 0; i < jumlah; i++) {
        if (!data[i].aktif) continue;
        if (data[i].id == kecualiId) continue;
        if (data[i].kodeMeja == kodeMeja &&
            data[i].tanggal  == tanggal  &&
            data[i].jam      == jam)
            return false;
    }
    return true;
}

int cariById(int id) {
    for (int i = 0; i < jumlah; i++)
        if (data[i].id == id && data[i].aktif) return i;
    return -1;
}

// DENAH MEJA
void tampilDenah(const string& tanggal, const string& jam = "") {
    cout << "\n  --- DENAH MEJA (" << tanggal;
    if (!jam.empty()) cout << " " << jam;
    cout << ") ---" << endl;
    cout << "  Seksi    A          B          C" << endl;
    garisTipis();
    for (int row = 1; row <= 3; row++) {
        cout << "    " << row << "  ";
        for (char col = 'A'; col <= 'C'; col++) {
            string kode = "";
            kode += (char)('0' + row);
            kode += col;
            // Cek apakah terisi (jam kosong = cek semua jam di hari itu)
            bool terisi = false;
            if (jam.empty()) {
                for (int i = 0; i < jumlah; i++)
                    if (data[i].aktif && data[i].kodeMeja == kode && data[i].tanggal == tanggal)
                        { terisi = true; break; }
            } else {
                terisi = !mejaTersedia(kode, tanggal, jam);
            }
            if (terisi)
                cout << "  [PENUH]   ";
            else
                cout << "  [" << kode << "]     ";
        }
        cout << endl;
    }
    int terisi = hitungMejaTerisi(tanggal);
    cout << "  Terisi hari ini: " << terisi << "/" << TOTAL_MEJA;
    if (terisi >= TOTAL_MEJA) cout << "  *** SEMUA PENUH ***";
    cout << "\n" << endl;
}

// FILE I/O
void simpanKeFile() {
    ofstream file(FILE_DB.c_str());
    if (!file.is_open()) {
        cout << "  [!] Gagal menyimpan ke file " << FILE_DB << endl;
        return;
    }
    file << idCounter << "\n";
    for (int i = 0; i < jumlah; i++) {
        if (!data[i].aktif) continue; // hanya simpan yang aktif
        file << data[i].id            << "|"
             << data[i].kodeMeja      << "|"
             << data[i].tanggal       << "|"
             << data[i].jam           << "|"
             << data[i].namaPelanggan << "|"
             << 1 << "\n";
    }
    file.close();
}

void muatDariFile() {
    ifstream file(FILE_DB.c_str());
    if (!file.is_open()) return;

    string line;
    if (getline(file, line)) {
        istringstream ss(line);
        ss >> idCounter;
    }

    jumlah = 0;
    while (getline(file, line) && jumlah < MAX_RESERVASI) {
        if (line.empty()) continue;
        istringstream ss(line);
        string token;
        Reservasi r;
        getline(ss, token, '|'); r.id             = atoi(token.c_str());
        getline(ss, token, '|'); r.kodeMeja        = token;
        getline(ss, token, '|'); r.tanggal         = token;
        getline(ss, token, '|'); r.jam             = token;
        getline(ss, token, '|'); r.namaPelanggan   = token;
        getline(ss, token, '|'); r.aktif           = (atoi(token.c_str()) == 1);
        data[jumlah++] = r;
    }
    file.close();
}

// Reset ID: kompres array, assign ulang ID dari 1
void resetIds() {
    // Kompres: buang record tidak aktif
    int newJumlah = 0;
    for (int i = 0; i < jumlah; i++)
        if (data[i].aktif)
            data[newJumlah++] = data[i];
    jumlah = newJumlah;

    // Assign ulang ID berurutan mulai dari 1
    for (int i = 0; i < jumlah; i++)
        data[i].id = i + 1;
    idCounter = jumlah + 1;

    simpanKeFile();
}

// 1. INPUT RESERVASI
void inputReservasi() {
    clearScreen();
    header("INPUT RESERVASI");

    // Input & validasi tanggal
    string tanggal = inputTanggalValid("Tanggal");

    // Input & validasi jam
    string jam = inputJamValid("Jam Reservasi");

    tampilDenah(tanggal, jam);

    int terisi = hitungMejaTerisi(tanggal);
    if (terisi >= TOTAL_MEJA) {
        cout << "  +--------------------------------------+" << endl;
        cout << "  |  RESERVASI PENUH - MAX " << TOTAL_MEJA << " MEJA        |" << endl;
        cout << "  |  Tidak bisa tambah reservasi baru   |" << endl;
        cout << "  +--------------------------------------+" << endl;
        pressEnter();
        return;
    }

    cout << "  Sisa meja tersedia (hari ini): " << (TOTAL_MEJA - terisi) << " meja" << endl;
    garisTipis();

    // Input kode meja
    string kodeMeja;
    while (true) {
        cout << "  Kode Meja (1A-3C) : ";
        getline(cin, kodeMeja);
        kodeMeja = toUpper(kodeMeja);

        if (!mejaValid(kodeMeja)) {
            cout << "  [!] Tidak valid! Gunakan: 1A 1B 1C | 2A 2B 2C | 3A 3B 3C" << endl;
            continue;
        }
        if (!mejaTersedia(kodeMeja, tanggal, jam)) {
            cout << "  [!] Meja " << kodeMeja << " sudah dipesan pada " << tanggal << " jam " << jam << "!" << endl;
            continue;
        }
        break;
    }

    // Input nama
    string nama;
    cout << "  Nama Pelanggan    : ";
    getline(cin, nama);
    if (nama.empty()) {
        cout << "\n  [!] Nama tidak boleh kosong." << endl;
        pressEnter();
        return;
    }

    // Simpan
    Reservasi r;
    r.id            = idCounter++;
    r.kodeMeja      = kodeMeja;
    r.tanggal       = tanggal;
    r.jam           = jam;
    r.namaPelanggan = nama;
    r.aktif         = true;
    data[jumlah++]  = r;
    simpanKeFile();

    cout << "\n  [OK] Reservasi berhasil disimpan!" << endl;
    garisTipis();
    cout << "  Kode Booking : " << formatBooking(r.id) << endl;
    cout << "  Meja         : " << r.kodeMeja << endl;
    cout << "  Tanggal      : " << r.tanggal << endl;
    cout << "  Jam          : " << r.jam << endl;
    cout << "  Nama         : " << r.namaPelanggan << endl;

    int sisa = TOTAL_MEJA - hitungMejaTerisi(tanggal);
    cout << "\n  [INFO] Sisa meja pada " << tanggal << ": "
         << (sisa <= 0 ? 0 : sisa) << " meja" << endl;

    pressEnter();
}

// 2. OUTPUT RESERVASI
void outputReservasi() {
    clearScreen();
    header("DAFTAR RESERVASI");

    int aktifCount = 0;
    for (int i = 0; i < jumlah; i++)
        if (data[i].aktif) aktifCount++;

    if (aktifCount == 0) {
        cout << "  [!] Belum ada data reservasi." << endl;
        pressEnter();
        return;
    }

    cout << left
         << "  " << setw(11) << "Booking"
         << setw(7)  << "Meja"
         << setw(14) << "Tanggal"
         << setw(8)  << "Jam"
         << "Nama Pelanggan" << endl;
    garisTipis();

    for (int i = 0; i < jumlah; i++) {
        if (!data[i].aktif) continue;
        cout << "  " << left << setfill(' ')
             << setw(11) << formatBooking(data[i].id)
             << setw(7)  << data[i].kodeMeja
             << setw(14) << data[i].tanggal
             << setw(8)  << data[i].jam
             << data[i].namaPelanggan << endl;
    }

    garisTebal();
    cout << "  Total reservasi aktif: " << aktifCount << endl;
    pressEnter();
}

// 3. SORTING (Bubble Sort)
void sortingReservasi() {
    clearScreen();
    header("SORTING RESERVASI");
    cout << "  Urutkan berdasarkan:" << endl;
    cout << "  1. Kode Meja" << endl;
    cout << "  2. Nama Pelanggan (Alfabet)" << endl;
    cout << "  3. Tanggal" << endl;
    cout << "  4. Jam" << endl;
    garisTipis();
    cout << "  Pilih : ";
    string pStr; getline(cin, pStr);
    int pilih = atoi(pStr.c_str());

    int idx[MAX_RESERVASI], n = 0;
    for (int i = 0; i < jumlah; i++)
        if (data[i].aktif) idx[n++] = i;

    if (n == 0) {
        cout << "\n  [!] Tidak ada data." << endl;
        pressEnter(); return;
    }

    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            bool sw = false;
            if      (pilih == 1 && data[idx[j]].kodeMeja      > data[idx[j+1]].kodeMeja)      sw = true;
            else if (pilih == 2 && data[idx[j]].namaPelanggan > data[idx[j+1]].namaPelanggan) sw = true;
            else if (pilih == 3 && data[idx[j]].tanggal       > data[idx[j+1]].tanggal)       sw = true;
            else if (pilih == 4 && data[idx[j]].jam           > data[idx[j+1]].jam)           sw = true;
            if (sw) { int t = idx[j]; idx[j] = idx[j+1]; idx[j+1] = t; }
        }
    }

    cout << "\n  Hasil setelah sorting:" << endl;
    garisTipis();
    cout << left
         << "  " << setw(11) << "Booking"
         << setw(7)  << "Meja"
         << setw(14) << "Tanggal"
         << setw(8)  << "Jam"
         << "Nama Pelanggan" << endl;
    garisTipis();

    for (int i = 0; i < n; i++) {
        int k = idx[i];
        cout << "  " << left << setfill(' ')
             << setw(11) << formatBooking(data[k].id)
             << setw(7)  << data[k].kodeMeja
             << setw(14) << data[k].tanggal
             << setw(8)  << data[k].jam
             << data[k].namaPelanggan << endl;
    }
    garisTebal();
    pressEnter();
}

