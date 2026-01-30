/*
 * Gemini generated 2026-01-28
 */
#pragma warning(disable:4146)

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <iomanip>
#include <cstring>
#include "u64_chess.h"
#include "u64_polyglot.h"

using namespace std;

// Compact structure for move storage (No Strings)
struct MoveData {
    U64 key;
    unsigned int movePacked; // Packed move: 0-5 bits (from), 6-11 (to), 12-14 (promo)
    int count;
    int checksum;
    unsigned short Wcount;

    // Sorting: Polyglot key ascending, then count descending
    bool operator<(const MoveData& other) const {
        if (key != other.key) return key < other.key;
        return count > other.count;
    }
};

// Polyglot Book Entry (16 bytes)
struct PolyBookEntry {
    U64 key;
    unsigned short move;
    unsigned short weight;
    unsigned int learn;
};

// Helper to convert UCI to Polyglot binary move format
unsigned short encode_move(string uci) {
    if (uci.length() < 4) return 0;
    int f_col = uci[0] - 'a';
    int f_row = uci[1] - '1';
    int t_col = uci[2] - 'a';
    int t_row = uci[3] - '1';
    int promo = 0;
    if (uci.length() == 5) {
        if (uci[4] == 'n') promo = 1;
        else if (uci[4] == 'b') promo = 2;
        else if (uci[4] == 'r') promo = 3;
        else if (uci[4] == 'q') promo = 4;
    }
    return (unsigned short)((promo << 12) | (f_row << 9) | (f_col << 6) | (t_row << 3) | t_col);
}


// Pack UCI string into a small integer
unsigned int pack_move(const char* uci) {
    if (strlen(uci) < 4) return 0;
    unsigned int f = (uci[0] - 'a') + ((uci[1] - '1') << 3);
    unsigned int t = (uci[2] - 'a') + ((uci[3] - '1') << 3);
    unsigned int p = 0;
    if (uci[4] == 'n') p = 1;
    else if (uci[4] == 'b') p = 2;
    else if (uci[4] == 'r') p = 3;
    else if (uci[4] == 'q') p = 4;
    return (f | (t << 6) | (p << 12));
}

// Convert packed move back to string for output
void unpack_move(unsigned int m, char* uci) {
    unsigned int f = m & 0x3F;
    unsigned int t = (m >> 6) & 0x3F;
    unsigned int p = (m >> 12) & 0x7;
    uci[0] = (f & 7) + 'a';
    uci[1] = (f >> 3) + '1';
    uci[2] = (t & 7) + 'a';
    uci[3] = (t >> 3) + '1';
    if (p > 0) {
        const char* promos = " nbrq";
        uci[4] = promos[p];
        uci[5] = '\0';
    }
    else {
        uci[4] = '\0';
    }
}

// Endian swappers for Polyglot binary format
U64 swap64(U64 v) {
    v = ((v & 0x00000000FFFFFFFFULL) << 32) | ((v & 0xFFFFFFFF00000000ULL) >> 32);
    v = ((v & 0x0000FFFF0000FFFFULL) << 16) | ((v & 0xFFFF0000FFFF0000ULL) >> 16);
    return ((v & 0x00FF00FF00FF00FFULL) << 8) | ((v & 0xFF00FF00FF00FF00ULL) >> 8);
}

unsigned short swap16(unsigned short v) { return (v << 8) | (v >> 8); }

int main(int argc, char* argv[]) {
    string ucifile, out_base;
    int min_occ = 1;

    if (argc < 2 || string(argv[1]) == "/h") {
        cout << "A command line tool to make chess polyglot data and book." << endl;
        cout << "It is a middleware to use after pgn2uci tool on ucifile text as result." << endl;
        cout << "Same position occurrences are counted as weight." << endl;
        cout << "Syntax:" << endl;
        cout << " ucitxt_polyglot ucifile=<source> out=<output> min_occ=<min>" << endl;
        return 0;
    }

    for (int i = 1; i < argc; i++) {
        string arg = argv[i];
        if (arg.find("ucifile=") == 0) ucifile = arg.substr(8);
        else if (arg.find("out=") == 0) out_base = arg.substr(4);
        else if (arg.find("min_occ=") == 0) min_occ = stoi(arg.substr(8));
    }

    Init_u64_chess();
    ifstream infile(ucifile);

    // Map using only integers as key (PolyglotKey + PackedMove)
    map<pair<U64, unsigned int>, pair<int, int>> tableMap;
    string line;
    long games = 0;
    while (getline(infile, line)) {
        if (line.empty() || line.substr(0, 2) == "//") continue;
        SetStartPos();
        size_t start = 0, end = 0;

        while ((end = line.find(' ', start)) != string::npos || start < line.size()) {
            string mStr = line.substr(start, (end == string::npos) ? line.size() - start : end - start);
            if (mStr.empty()) break;

            U64 key = getPolyglotKey();
            if (!uciMove((char*)mStr.c_str())) break;

            int mvcount = mg_uci_list[0];
            unsigned int pMove = pack_move(mStr.c_str());

            auto& entry = tableMap[{key, pMove}];
            entry.first++;         // increment count
            entry.second = mvcount; // save checksum

            if (end == string::npos) break;
            start = end + 1;
        }

        games++;
        if (games % 10000 == 0) cout << games << " games scanned..." << endl;

        // --- NEW PRUNING LOGIC ---
        if (games % 600000 == 0) {
            cout << "Pruning table at " << games << " games to save memory..." << endl;
            map<pair<U64, unsigned int>, pair<int, int>> filteredMap;

            for (auto const& [keyMove, data] : tableMap) {
                // Keep entries that had a second hit
                if (data.first > 1) {
                    filteredMap[keyMove] = data;
                }
            }

            cout << "Entries reduced from " << tableMap.size() << " to " << filteredMap.size() << endl;
            tableMap.swap(filteredMap);
            // filteredMap goes out of scope here, freeing memory
        }
    }

    cout << "Preparing sort-table." << endl;

    vector<MoveData> finalTable;
    for (auto const& [keyInfo, valInfo] : tableMap) {
        if (valInfo.first >= min_occ) {
            finalTable.push_back({ keyInfo.first, keyInfo.second, valInfo.first, valInfo.second, 0 });
        }
    }

    sort(finalTable.begin(), finalTable.end());
    cout << finalTable.size() << " positions sorted." << endl;

    cout << "Recalculating and writing." << endl;

    // Wcount Recalculation
    for (size_t i = 0; i < finalTable.size(); ) {
        U64 curKey = finalTable[i].key;
        size_t j = i;
        long long S = 0;
        while (j < finalTable.size() && finalTable[j].key == curKey) {
            S += finalTable[j].count;
            j++;
        }
        float K = 32000.0f / S;
        for (size_t k = i; k < j; k++) {
            finalTable[k].Wcount = (unsigned short)((K >= 1.0f ? finalTable[k].count : (1 + (K * finalTable[k].count)))) & 0xFFFF;
        }
        i = j;
    }

    ofstream txt(out_base + ".txt");
    ofstream bin(out_base + ".bin", ios::binary);
    char uBuf[8];

    for (const auto& e : finalTable) {
        unpack_move(e.movePacked, uBuf);
        txt << "0x" << setfill('0') << setw(16) << hex << uppercase << e.key
            << " " << uBuf << " " << dec << e.count << endl;

        PolyBookEntry be;
        be.key = swap64(e.key);
        be.move = swap16((unsigned short)encode_move(uBuf)); // Using polyglot move encoding
        be.weight = swap16(e.Wcount);
        be.learn = 0;
        bin.write((char*)&be, 16);
    }

    return 0;
}