/*
 * Gemini generated - 2026-02-01
 * Description: Decodes a chess polyglot book (.bin) into a CSV file.
 * Records are ordered: polyglot_key, uci_moves, move, weight, learn.
 */

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <iomanip>
#include <map>
#include <cstring>
#include "u64_chess.h"
#include "u64_polyglot.h"

#pragma warning(disable:4146)

using namespace std;

// Standard Polyglot Book Entry (16 bytes)
struct PolyBookEntry {
    U64 key;
    unsigned short move;
    unsigned short weight;
    unsigned int learn;
};

// Endian swappers for Polyglot binary format (Big-Endian to Little-Endian)
U64 swap64(U64 v) {
    v = ((v & 0x00000000FFFFFFFFULL) << 32) | ((v & 0xFFFFFFFF00000000ULL) >> 32);
    v = ((v & 0x0000FFFF0000FFFFULL) << 16) | ((v & 0xFFFF0000FFFF0000ULL) >> 16);
    return ((v & 0x00FF00FF00FF00FFULL) << 8) | ((v & 0xFF00FF00FF00FF00ULL) >> 8);
}

unsigned short swap16(unsigned short v) { return (v << 8) | (v >> 8); }

unsigned int swap32(unsigned int v) {
    return ((v >> 24) & 0xff) | ((v << 8) & 0xff0000) | ((v >> 8) & 0xff00) | ((v << 24) & 0xff000000);
}

// Global storage and counters
map<U64, vector<PolyBookEntry>> bookData;
map<U64, bool> visited;
long long variantCounter = 0;
const char prompieces[] = { ' ', 'n', 'b', 'r', 'q' };

// Convert binary move to UCI string
void getUciStr(unsigned short m, char* uci) {
    int to_col = m & 7, to_row = (m >> 3) & 7;
    int from_col = (m >> 6) & 7, from_row = (m >> 9) & 7;
    int promo = (m >> 12) & 7;
    uci[0] = from_col + 'a'; uci[1] = from_row + '1';
    uci[2] = to_col + 'a';   uci[3] = to_row + '1';
    if (promo > 0) { uci[4] = prompieces[promo]; uci[5] = '\0'; }
    else { uci[4] = '\0'; }
}

// Recursive traversal following the samp2_seek pattern
void traverseBook(ofstream& csv, string moveHistory) {
    U64 currentKey = getPolyglotKey();

    // Prevent infinite cycles in the book
    if (visited[currentKey]) return;
    visited[currentKey] = true;

    auto it = bookData.find(currentKey);
    if (it == bookData.end()) {
        visited[currentKey] = false;
        return;
    }

    // Local stack move list for recursion safety
    U8 mlist[4 << 8];
    MoveGen(mlist);
    int moveCount = mlist[0];

    for (const auto& entry : it->second) {
        char bookUci[8];
        getUciStr(entry.move, bookUci);

        // Verify legality of the book move
        bool isLegal = false;
        for (int i = 0; i < moveCount; i++) {
            U8* p = mlist + (1 + (i << 2));
            char legalUci[8];
            legalUci[0] = ((*(p + 1) & 7) + 'a');
            legalUci[1] = ((*(p + 1) >> 3) + '1');
            legalUci[2] = ((*(p + 2) & 7) + 'a');
            legalUci[3] = ((*(p + 2) >> 3) + '1');
            legalUci[4] = ((*(p + 3) == 0) ? 0 : prompieces[*(p + 3)]);
            if (*(p + 3) == 0) legalUci[4] = 0; else legalUci[5] = 0;

            if (strcmp(bookUci, legalUci) == 0) {
                isLegal = true;
                break;
            }
        }

        if (isLegal) {
            variantCounter++;
            if (variantCounter % 100000 == 0) {
                cout << variantCounter << " variants processed, looping " << moveHistory << endl;
            }

            // Write record: polyglot_key, uci_moves, move, weight, learn
            csv << "0x" << setfill('0') << setw(16) << hex << uppercase << currentKey << ", "
                << "\"" << moveHistory << "\", "
                << "\"" << bookUci << "\", "
                << dec << entry.weight << ", "
                << entry.learn << endl;

            // Step deeper
            if (uciMove(bookUci)) {
                string nextHistory = moveHistory;
                if (!nextHistory.empty()) nextHistory += " ";
                nextHistory += bookUci;

                traverseBook(csv, nextHistory);
                UnDoMove();
            }
        }
    }

    visited[currentKey] = false;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        cout << "Description: Decodes chess polyglot book .bin to CSV text file (including Learn data)." << endl;
        cout << "Syntax: poly2csv <.bin file> <.csv file to create>" << endl;
        return 0;
    }

    Init_u64_chess();

    ifstream bin(argv[1], ios::binary);
    if (!bin) { cerr << "Error: Cannot open " << argv[1] << endl; return 1; }

    PolyBookEntry raw;
    while (bin.read((char*)&raw, 16)) {
        PolyBookEntry cooked;
        cooked.key = swap64(raw.key);
        cooked.move = swap16(raw.move);
        cooked.weight = swap16(raw.weight);
        cooked.learn = swap32(raw.learn); // Added Learn swap
        bookData[cooked.key].push_back(cooked);
    }
    bin.close();

    ofstream csv(argv[2]);
    if (!csv) { cerr << "Error: Cannot create " << argv[2] << endl; return 1; }

    // Column headers
    csv << "polyglot_key, uci_moves, move, weight, learn" << endl;

    SetStartPos();
    traverseBook(csv, "");

    csv.close();
    cout << "Success: " << variantCounter << " variants processed into " << argv[2] << endl;

    return 0;
}