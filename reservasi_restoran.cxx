#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
#include <cstdlib>
#include <sstream>
using namespace std;

// ─────────────────────────────────────────────
//  KONSTANTA
// ─────────────────────────────────────────────
const int    TOTAL_MEJA  = 9;
const string FILE_DB     = "reservasi.txt";
const string ADMIN_USER  = "admin";
const string ADMIN_PASS  = "admin";
const int    MAX_LOGIN   = 3;

// ── SESI DINNER ──────────────────────────────
const int JAM_BUKA  = 18 * 60;   // 18:00 dalam menit
const int JAM_TUTUP = 22 * 60;   // 22:00 dalam menit

const string DAFTAR_MEJA[TOTAL_MEJA] = {
    "1A","1B","1C",
    "2A","2B","2C",
    "3A","3B","3C"
};

// ─────────────────────────────────────────────
//  NODE LINKED LIST
// ─────────────────────────────────────────────
struct Node {
    int    id;
    string kodeMeja;
    string tanggal;        // DD/MM/YYYY
    string jam;            // HH:MM
    string namaPelanggan;
    Node*  next;
};

Node* head      = NULL;
int   idCounter = 1;

// ═════════════════════════════════════════════
//  UTILITAS UI
// ═════════════════════════════════════════════
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

// ═════════════════════════════════════════════
//  VALIDASI
// ═════════════════════════════════════════════
bool semuaDigit(const string& s) {
    if (s.empty()) return false;
    for (int i = 0; i < (int)s.size(); i++)
        if (s[i] < '0' || s[i] > '9') return false;
    return true;
}
bool tanggalValid(const string& tgl) {
    if (tgl.size() != 10) return false;
    if (tgl[2] != '/' || tgl[5] != '/') return false;
    string dd = tgl.substr(0,2), mm = tgl.substr(3,2), yyyy = tgl.substr(6,4);
    if (!semuaDigit(dd)||!semuaDigit(mm)||!semuaDigit(yyyy)) return false;
    int hari=atoi(dd.c_str()), bulan=atoi(mm.c_str()), tahun=atoi(yyyy.c_str());
    if (hari<1||hari>31) return false;
    if (bulan<1||bulan>12) return false;
    int maxHari=31;
    if (bulan==4||bulan==6||bulan==9||bulan==11) maxHari=30;
    else if (bulan==2) {
        bool kab=(tahun%4==0&&tahun%100!=0)||(tahun%400==0);
        maxHari=kab?29:28;
    }
    return hari<=maxHari;
}

// ── VALIDASI JAM: hanya boleh 18:00 - 22:00 ─
bool jamValid(const string& j) {
    if (j.size()!=5) return false;
    if (j[2]!=':') return false;
    string hh=j.substr(0,2), mm=j.substr(3,2);
    if (!semuaDigit(hh)||!semuaDigit(mm)) return false;
    int jam=atoi(hh.c_str()), menit=atoi(mm.c_str());
    if (jam<0||jam>23||menit<0||menit>59) return false;
    int totalMenit = jam * 60 + menit;
    return totalMenit >= JAM_BUKA && totalMenit <= JAM_TUTUP;
}

string inputTanggalValid(const string& label) {
    string tgl;
    while (true) {
        cout << "  " << label << " (DD/MM/YYYY) : ";
        getline(cin, tgl);
        if (tanggalValid(tgl)) break;
        cout << "  [!] Format salah. Contoh: 25/12/2026" << endl;
    }
    return tgl;
}

// ── Input jam dengan info sesi dinner ────────
string inputJamValid(const string& label) {
    string j;
    cout << "  [INFO] Sesi dinner: 18:00 - 22:00" << endl;
    while (true) {
        cout << "  " << label << " (HH:MM)     : ";
        getline(cin, j);
        if (jamValid(j)) break;
        cout << "  [!] Jam harus dalam sesi dinner (18:00 - 22:00)" << endl;
    }
    return j;
}

bool mejaValid(const string& kode) {
    for (int i=0;i<TOTAL_MEJA;i++)
        if (DAFTAR_MEJA[i]==kode) return true;
    return false;
}

// ═════════════════════════════════════════════
//  OPERASI LINKED LIST
// ═════════════════════════════════════════════
int hitungJumlah() {
    int count = 0;
    Node* curr = head;
    while (curr != NULL) { count++; curr = curr->next; }
    return count;
}

Node* pushBack(int id, const string& meja, const string& tgl,
               const string& jam, const string& nama) {
    Node* baru         = new Node();
    baru->id           = id;
    baru->kodeMeja     = meja;
    baru->tanggal      = tgl;
    baru->jam          = jam;
    baru->namaPelanggan= nama;
    baru->next         = NULL;
    if (head == NULL) { head = baru; return baru; }
    Node* curr = head;
    while (curr->next != NULL) curr = curr->next;
    curr->next = baru;
    return baru;
}

Node* cariById(int id) {
    Node* curr = head;
    while (curr != NULL) {
        if (curr->id == id) return curr;
        curr = curr->next;
    }
    return NULL;
}

bool hapusNode(int id) {
    if (head == NULL) return false;
    if (head->id == id) {
        Node* temp = head;
        head = head->next;
        delete temp;
        return true;
    }
    Node* prev = head;
    Node* curr = head->next;
    while (curr != NULL) {
        if (curr->id == id) {
            prev->next = curr->next;
            delete curr;
            return true;
        }
        prev = curr;
        curr = curr->next;
    }
    return false;
}

void resetIds() {
    int newId = 1;
    Node* curr = head;
    while (curr != NULL) { curr->id = newId++; curr = curr->next; }
    idCounter = newId;
}

int hitungMejaTerisi(const string& tanggal) {
    int count = 0;
    Node* curr = head;
    while (curr != NULL) {
        if (curr->tanggal == tanggal) count++;
        curr = curr->next;
    }
    return count;
}

// ── CEK KETERSEDIAAN: 1 meja = 1x per hari ──
//   Logika lama (durasi 4 jam) DIHAPUS.
//   Satu meja hanya bisa dipesan 1x dalam 1 tanggal.
bool mejaTersedia(const string& kodeMeja, const string& tanggal,
                  int kecualiId = -1) {
    Node* curr = head;
    while (curr != NULL) {
        if (curr->id != kecualiId &&
            curr->kodeMeja == kodeMeja &&
            curr->tanggal  == tanggal) {
            return false;
        }
        curr = curr->next;
    }
    return true;
}

void cetakBaris(Node* n) {
    cout << "  " << left << setfill(' ')
         << setw(11) << formatBooking(n->id)
         << setw(7)  << n->kodeMeja
         << setw(14) << n->tanggal
         << setw(8)  << n->jam
         << n->namaPelanggan << endl;
}

// ── Denah meja: status berdasarkan tanggal saja ──
void tampilDenah(const string& tanggal) {
    cout << "\n  --- DENAH MEJA (" << tanggal << ") ---" << endl;
    cout << "  Seksi    A          B          C" << endl;
    garisTipis();
    for (int row=1; row<=3; row++) {
        cout << "    " << row << "  ";
        for (char col='A'; col<='C'; col++) {
            string kode = "";
            kode += (char)('0'+row);
            kode += col;
            bool terisi = !mejaTersedia(kode, tanggal);
            cout << (terisi ? "  [PENUH]   " : ("  ["+kode+"]     "));
        }
        cout << endl;
    }
    int terisi = hitungMejaTerisi(tanggal);
    cout << "  Terisi hari ini: " << terisi << "/" << TOTAL_MEJA;
    if (terisi >= TOTAL_MEJA) cout << "  *** SEMUA PENUH ***";
    cout << "\n" << endl;
}

// ═════════════════════════════════════════════
//  FILE I/O
// ═════════════════════════════════════════════
void simpanKeFile() {
    ofstream file(FILE_DB.c_str());
    if (!file.is_open()) { cout << "  [!] Gagal simpan." << endl; return; }
    file << idCounter << "\n";
    Node* curr = head;
    while (curr != NULL) {
        file << curr->id            << "|"
             << curr->kodeMeja      << "|"
             << curr->tanggal       << "|"
             << curr->jam           << "|"
             << curr->namaPelanggan << "|"
             << 1 << "\n";
        curr = curr->next;
    }
    file.close();
}

void muatDariFile() {
    ifstream file(FILE_DB.c_str());
    if (!file.is_open()) return;
    string line;
    if (getline(file, line)) { istringstream ss(line); ss >> idCounter; }
    while (getline(file, line)) {
        if (line.empty()) continue;
        istringstream ss(line);
        string id, meja, tgl, jam, nama, dummy;
        getline(ss, id,   '|');
        getline(ss, meja, '|');
        getline(ss, tgl,  '|');
        getline(ss, jam,  '|');
        getline(ss, nama, '|');
        pushBack(atoi(id.c_str()), meja, tgl, jam, nama);
    }
    file.close();
}

// ═════════════════════════════════════════════
//  MENU FUNGSI
// ═════════════════════════════════════════════

// 1. INPUT
void inputReservasi() {
    clearScreen();
    header("INPUT RESERVASI");
    string tanggal = inputTanggalValid("Tanggal");

    tampilDenah(tanggal);

    int terisi = hitungMejaTerisi(tanggal);
    if (terisi >= TOTAL_MEJA) {
        cout << "  [!] Semua meja penuh pada tanggal tersebut!" << endl;
        pressEnter(); return;
    }

    string jam = inputJamValid("Jam Reservasi");

    string kodeMeja;
    while (true) {
        cout << "  Kode Meja (1A-3C) : ";
        getline(cin, kodeMeja);
        kodeMeja = toUpper(kodeMeja);
        if (!mejaValid(kodeMeja)) {
            cout << "  [!] Kode meja tidak valid!" << endl; continue;
        }
        if (!mejaTersedia(kodeMeja, tanggal)) {
            cout << "  [!] Meja " << kodeMeja << " sudah dipesan pada tanggal tersebut!" << endl;
            continue;
        }
        break;
    }

    string nama;
    cout << "  Nama Pelanggan    : ";
    getline(cin, nama);
    if (nama.empty()) {
        cout << "  [!] Nama tidak boleh kosong." << endl;
        pressEnter(); return;
    }

    Node* baru = pushBack(idCounter++, kodeMeja, tanggal, jam, nama);
    simpanKeFile();

    cout << "\n  [OK] Reservasi berhasil disimpan!" << endl;
    garisTipis();
    cout << "  Kode Booking : " << formatBooking(baru->id) << endl;
    cout << "  Meja         : " << baru->kodeMeja << endl;
    cout << "  Tanggal      : " << baru->tanggal  << endl;
    cout << "  Jam          : " << baru->jam       << endl;
    cout << "  Nama         : " << baru->namaPelanggan << endl;
    pressEnter();
}

// 2. OUTPUT
void outputReservasi() {
    clearScreen();
    header("DAFTAR RESERVASI");
    if (head == NULL) {
        cout << "  [!] Belum ada data reservasi." << endl;
        pressEnter(); return;
    }
    cout << left
         << "  " << setw(11) << "Booking"
         << setw(7)  << "Meja"
         << setw(14) << "Tanggal"
         << setw(8)  << "Jam"
         << "Nama Pelanggan" << endl;
    garisTipis();
    Node* curr = head;
    int count = 0;
    while (curr != NULL) {
        cetakBaris(curr);
        count++;
        curr = curr->next;
    }
    garisTebal();
    cout << "  Total reservasi: " << count << endl;
    pressEnter();
}

// 3. SORTING
void sortingReservasi() {
    clearScreen();
    header("SORTING RESERVASI");
    if (head == NULL) { cout << "  [!] Tidak ada data." << endl; pressEnter(); return; }

    cout << "  Urutkan berdasarkan:" << endl;
    cout << "  1. Kode Meja" << endl;
    cout << "  2. Nama Pelanggan" << endl;
    cout << "  3. Tanggal" << endl;
    cout << "  4. Jam" << endl;
    garisTipis();
    cout << "  Pilih : ";
    string pStr; getline(cin, pStr);
    int pilih = atoi(pStr.c_str());

    bool swapped;
    do {
        swapped = false;
        Node* curr = head;
        while (curr != NULL && curr->next != NULL) {
            bool sw = false;
            if      (pilih==1 && curr->kodeMeja      > curr->next->kodeMeja)      sw=true;
            else if (pilih==2 && curr->namaPelanggan > curr->next->namaPelanggan) sw=true;
            else if (pilih==3 && curr->tanggal       > curr->next->tanggal)       sw=true;
            else if (pilih==4 && curr->jam           > curr->next->jam)           sw=true;

            if (sw) {
                swap(curr->id,            curr->next->id);
                swap(curr->kodeMeja,      curr->next->kodeMeja);
                swap(curr->tanggal,       curr->next->tanggal);
                swap(curr->jam,           curr->next->jam);
                swap(curr->namaPelanggan, curr->next->namaPelanggan);
                swapped = true;
            }
            curr = curr->next;
        }
    } while (swapped);

    cout << "\n  Hasil setelah sorting:" << endl;
    garisTipis();
    cout << left
         << "  " << setw(11) << "Booking"
         << setw(7)  << "Meja"
         << setw(14) << "Tanggal"
         << setw(8)  << "Jam"
         << "Nama Pelanggan" << endl;
    garisTipis();
    Node* curr = head;
    while (curr != NULL) { cetakBaris(curr); curr = curr->next; }
    garisTebal();
    pressEnter();
}

// 4. SEARCH
void searchReservasi() {
    clearScreen();
    header("SEARCH RESERVASI");
    cout << "  Cari berdasarkan:" << endl;
    cout << "  1. Nama Pelanggan" << endl;
    cout << "  2. Kode Meja" << endl;
    cout << "  3. Kode Booking (angka)" << endl;
    cout << "  4. Tanggal" << endl;
    garisTipis();
    cout << "  Pilih : ";
    string pStr; getline(cin, pStr);
    int pilih = atoi(pStr.c_str());

    bool ketemu = false;
    auto cetakHasil = [&](Node* n) {
        cout << "  Booking : " << formatBooking(n->id) << endl;
        cout << "  Meja    : " << n->kodeMeja << endl;
        cout << "  Tanggal : " << n->tanggal  << endl;
        cout << "  Jam     : " << n->jam       << endl;
        cout << "  Nama    : " << n->namaPelanggan << endl;
        garisTipis();
        ketemu = true;
    };

    cout << "\n  Hasil pencarian:\n"; garisTipis();

    if (pilih == 1) {
        cout << "  Nama : "; string q; getline(cin, q);
        Node* curr = head;
        while (curr) {
            if (curr->namaPelanggan.find(q) != string::npos) cetakHasil(curr);
            curr = curr->next;
        }
    } else if (pilih == 2) {
        cout << "  Kode Meja : "; string q; getline(cin, q); q=toUpper(q);
        Node* curr = head;
        while (curr) { if (curr->kodeMeja==q) cetakHasil(curr); curr=curr->next; }
    } else if (pilih == 3) {
        cout << "  Kode Booking (angka) : "; string q; getline(cin, q);
        Node* n = cariById(atoi(q.c_str()));
        if (n) cetakHasil(n);
    } else if (pilih == 4) {
        string tgl = inputTanggalValid("Tanggal");
        tampilDenah(tgl);
        Node* curr = head;
        while (curr) { if (curr->tanggal==tgl) cetakHasil(curr); curr=curr->next; }
    }

    if (!ketemu) cout << "  [!] Data tidak ditemukan." << endl;
    pressEnter();
}

// 5. DELETE
void deleteReservasi() {
    clearScreen();
    header("DELETE RESERVASI");
    cout << "  Kode Booking yang dihapus (angka): ";
    string idStr; getline(cin, idStr);
    int id = atoi(idStr.c_str());

    Node* target = cariById(id);
    if (!target) {
        cout << "  [!] ID " << id << " tidak ditemukan." << endl;
        pressEnter(); return;
    }

    cout << "\n  Data yang akan dihapus:" << endl; garisTipis();
    cout << "  Booking : " << formatBooking(target->id) << endl;
    cout << "  Meja    : " << target->kodeMeja << endl;
    cout << "  Tanggal : " << target->tanggal  << endl;
    cout << "  Jam     : " << target->jam       << endl;
    cout << "  Nama    : " << target->namaPelanggan << endl;
    garisTipis();
    cout << "  Konfirmasi hapus? (y/n): ";
    string k; getline(cin, k);

    if (k=="y"||k=="Y") {
        string bk = formatBooking(id);
        hapusNode(id);
        resetIds();
        simpanKeFile();
        cout << "  [OK] Reservasi " << bk << " berhasil dihapus." << endl;
        cout << "  [OK] Kode booking telah direset & diperbarui." << endl;
    } else {
        cout << "  [!] Penghapusan dibatalkan." << endl;
    }
    pressEnter();
}

// 6. EDIT
void editReservasi() {
    clearScreen();
    header("EDIT RESERVASI");
    cout << "  Kode Booking yang diedit (angka): ";
    string idStr; getline(cin, idStr);
    int id = atoi(idStr.c_str());

    Node* n = cariById(id);
    if (!n) {
        cout << "  [!] ID " << id << " tidak ditemukan." << endl;
        pressEnter(); return;
    }

    cout << "\n  Data saat ini:" << endl; garisTipis();
    cout << "  Meja    : " << n->kodeMeja << endl;
    cout << "  Tanggal : " << n->tanggal  << endl;
    cout << "  Jam     : " << n->jam       << endl;
    cout << "  Nama    : " << n->namaPelanggan << endl;
    garisTipis();
    cout << "  (Kosongkan = tidak diubah)\n" << endl;

    string inputMeja;
    cout << "  Kode Meja baru [" << n->kodeMeja << "]: ";
    getline(cin, inputMeja);
    if (!inputMeja.empty()) {
        inputMeja = toUpper(inputMeja);
        if (!mejaValid(inputMeja)) {
            cout << "  [!] Kode meja tidak valid!" << endl; pressEnter(); return;
        }
        if (!mejaTersedia(inputMeja, n->tanggal, n->id)) {
            cout << "  [!] Meja " << inputMeja << " sudah dipesan pada tanggal tersebut!" << endl;
            pressEnter(); return;
        }
        n->kodeMeja = inputMeja;
    }

    string inputTanggal;
    cout << "  Tanggal baru [" << n->tanggal << "]: ";
    getline(cin, inputTanggal);
    if (!inputTanggal.empty()) {
        if (!tanggalValid(inputTanggal)) {
            cout << "  [!] Format tanggal tidak valid!" << endl; pressEnter(); return;
        }
        if (!mejaTersedia(n->kodeMeja, inputTanggal, n->id)) {
            cout << "  [!] Meja " << n->kodeMeja << " sudah dipesan pada tanggal tersebut!" << endl;
            pressEnter(); return;
        }
        n->tanggal = inputTanggal;
    }

    string inputJam;
    cout << "  [INFO] Sesi dinner: 18:00 - 22:00" << endl;
    cout << "  Jam baru [" << n->jam << "]: ";
    getline(cin, inputJam);
    if (!inputJam.empty()) {
        if (!jamValid(inputJam)) {
            cout << "  [!] Jam harus dalam sesi dinner (18:00 - 22:00)" << endl;
            pressEnter(); return;
        }
        n->jam = inputJam;
    }

    string inputNama;
    cout << "  Nama baru [" << n->namaPelanggan << "]: ";
    getline(cin, inputNama);
    if (!inputNama.empty()) n->namaPelanggan = inputNama;

    simpanKeFile();
    cout << "\n  [OK] Data berhasil diperbarui!" << endl;
    pressEnter();
}

// ─────────────────────────────────────────────
//  LOGIN
// ─────────────────────────────────────────────
bool login() {
    int attempt = 0;
    while (attempt < MAX_LOGIN) {
        clearScreen();
        garisTebal();
        cout << "    SISTEM RESERVASI RESTORAN " << endl;
        cout << "    Selamat datang! Silahkan login." << endl;
        garisTebal();
        if (attempt > 0) {
            cout << "  [!] Username atau password salah!" << endl;
            cout << "      Kesempatan tersisa: " << (MAX_LOGIN-attempt) << endl;
            garisTipis();
        }
        string user, pass;
        cout << "  username : "; getline(cin, user);
        cout << "  password : "; getline(cin, pass);
        if (user==ADMIN_USER && pass==ADMIN_PASS) {
            clearScreen(); garisTebal();
            cout << "    LOGIN BERHASIL" << endl; garisTebal();
            cout << "  Selamat datang, " << user << "!" << endl;
            cout << "  Tekan Enter untuk masuk ke menu...";
            cin.get(); return true;
        }
        attempt++;
    }
    clearScreen(); garisTebal();
    cout << "    AKSES DITOLAK" << endl; garisTebal();
    cout << "  [X] Program ditutup." << endl; garisTebal();
    cin.get(); return false;
}

// ─────────────────────────────────────────────
//  MAIN
// ─────────────────────────────────────────────
int main() {
    muatDariFile();
    if (!login()) return 1;

    clearScreen(); garisTebal();
    cout << "    SISTEM RESERVASI RESTORAN" << endl; garisTebal();
    cout << "  Records  : " << hitungJumlah() << " reservasi dimuat" << endl;
    cout << "  Struktur : Single Linked List" << endl;
    cout << "  Sesi     : Dinner 18:00 - 22:00" << endl;
    garisTebal();
    cout << "\n  Tekan Enter untuk mulai...";
    cin.get();

    string menuStr; int menu;
    do {
        clearScreen(); garisTebal();
        cout << "         MENU UTAMA" << endl; garisTebal();
        cout << "  1. Input Reservasi"   << endl;
        cout << "  2. Output Reservasi"  << endl;
        cout << "  3. Sorting Reservasi" << endl;
        cout << "  4. Search Reservasi"  << endl;
        cout << "  5. Delete Reservasi"  << endl;
        cout << "  6. Edit Reservasi"    << endl;
        cout << "  7. Keluar"            << endl;
        garisTebal();
        cout << "  Records: " << hitungJumlah() << endl;
        garisTebal();
        cout << "  Masukkan Menu : ";
        getline(cin, menuStr);
        menu = atoi(menuStr.c_str());
        switch (menu) {
            case 1: inputReservasi();   break;
            case 2: outputReservasi();  break;
            case 3: sortingReservasi(); break;
            case 4: searchReservasi();  break;
            case 5: deleteReservasi();  break;
            case 6: editReservasi();    break;
            case 7:
                clearScreen(); garisTebal();
                cout << "  Terima kasih! Data tersimpan di " << FILE_DB << endl;
                garisTebal(); break;
            default:
                cout << "\n  [!] Menu tidak valid.\n"; cin.get();
        }
    } while (menu != 7);

    // Bebaskan semua memori linked list
    Node* curr = head;
    while (curr != NULL) {
        Node* temp = curr;
        curr = curr->next;
        delete temp;
    }
    return 0;
}
