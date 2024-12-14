// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"

#include "u64_chess.h"

// This is needed for polyglot key generation
#include "u64_polyglot.h"

char lib_buffer[102400];
U8 lib_mv[4 << 8];

U8 lib_ii_mv[(4 << 8) * 32];
U8* lib_ii_po[32];

const int matdifvals[] = { 900, 500, 320, 318, 110, 20000, 0,0, -900, -500, -320, -318, -110, -20000, 0,0 };

//------------------------------------------
//
//	Parse PGN part
//
void parse_pgn_moves(char* pgn) {

    char* s = pgn;
    char* u = lib_buffer;

    int q = 0;
    int mn = 1;

    U8 mv[4 << 8];
    char Pc = 0, Pp = 0;
    int w, i, N, fcastle = 0, prom = 0, found = 0;
    char* P;
    char c, h1 = 0, v1 = 0, h2 = 0, v2 = 0;
    U8 ty = 0, fl = 0, sq1 = 0, sq2 = 0;
    U8* po;

    for (;*s != 0;s++) {
        c = *s;
        if (c == '{') q++;
        if (q > 0) *s = ' ';       // remove comments
        if (c == '}') q--;
        if (c <= 13) *s = ' ';   // remove TABs etc...
        if (*s == 'e' && *(s + 1) == 'p') { *s = ' '; *(s + 1) = ' '; }      // ep case
        if (c == '*') *s = ' ';
    }
    s = pgn;
    *u = 0;

    MoveGen(mv);

    for (;*s != 0;s++) {
        while (*s == ' ') s++;
        if (*s == 0) break;

        // skip number
        c = *s;
        if (c >= '1' && c <= '9') {
            while (*s != 0 && *s != '.' && *s != ' ') s++;
        }
        while (*s == '.' || *s == ' ') s++;
        if (*s == 0) break;

        fcastle = 0;
        if ((strncmp(s, "0-0-0", 5) == 0) || (strncmp(s, "O-O-O", 5) == 0)) {
            fcastle = 2;
        }
        else {
            if ((strncmp(s, "0-0", 3) == 0) || (strncmp(s, "O-O", 3) == 0)) {
                fcastle = 1;
            }
        }

        // obtain chess move from pgn string

        if (!fcastle) {
            Pc = 0; w = 0; Pp = 0;
            h1 = 0; v1 = 0; h2 = 0; v2 = 0;
            while (*s != ' ' && *s != 0) {
                c = *s;
                if (c == '.') w = 0;
                if (Pc == 0) {
                    P = strchr((char *)"QRBNPK", c);
                    if (P != NULL) { Pc = *P; w = 1; }
                }
                if (w < 3 && (c == 'x' || c == '-' || c == ':')) w = 3;

                if (c >= 'a' && c <= 'h') {
                    if (w < 3 && h1 == 0) { h1 = c; w = 2; }
                    else if (w < 6) { h2 = c; w = 4; }
                }
                if (w > 0 && c >= '1' && c <= '8') {
                    if (w < 3 && v1 == 0) { v1 = c; w = 3; }
                    else if (w < 6) { v2 = c; w = 5; }
                }
                if (c == '=') { w = 6; c = *(++s); }
                if (w > 4) {
                    P = strchr((char*)"QRBN", c);
                    if (P != NULL) Pp = (*P) + 32;
                    else {
                        P = strchr((char*)"qrbn", c);
                        if (P != NULL) Pp = *P;
                    }
                }

                s++;
            }
            if (h2 == 0) { h2 = h1; v2 = v1; h1 = 0; v1 = 0; }
            if (h2 == 0 && v2 == 0) continue;
        }

        po = mv;
        N = *(po++);
        found = 0;

        // find this move in the movegen list

        for (i = 0; i < N; i++) {

            ty = (*po) & 7;
            sq1 = *(po + 1);
            sq2 = *(po + 2);
            fl = *(po + 3);

            if (fl & 32) {
                if (sq1 > sq2 && fcastle == 2) found = 1;
                if (sq1 < sq2 && fcastle == 1) found = 1;
            }

            if (!found && ((ty == 4 && Pc == 0) || (pieces[ty] == Pc))) {
                found = 1;
                if (h1 != 0 && h1 != ((sq1 & 7) + 'a')) found = 0;
                if (v1 != 0 && v1 != ((sq1 >> 3) + '1')) found = 0;
                if (h2 != 0 && h2 != ((sq2 & 7) + 'a')) found = 0;
                if (v2 != 0 && v2 != ((sq2 >> 3) + '1')) found = 0;

                prom = 0;
                if (fl & 2) {   // promotion
                    prom = pieces[8 + ((fl >> 2) & 3)];
                    if (Pp != 0 && Pp != prom) found = 0;
                }
            }
            if (found) {
                DoMove(po);
                *(u++) = ((sq1 & 7) + 'a');
                *(u++) = ((sq1 >> 3) + '1');
                *(u++) = ((sq2 & 7) + 'a');
                *(u++) = ((sq2 >> 3) + '1');
                if (prom) *(u++) = prom;
                *(u++) = ' ';
                *u = 0;
                MoveGen(mv);
                if (!ToMove) mn++;
                break;
            }
            po += 4;
        }

    }
}


extern "C" void __declspec(dllexport) setstartpos()
{
    SetStartPos();
}

extern "C" __declspec(dllexport) char* sboard()
{
    sBoard( lib_buffer );
    return lib_buffer;
}


extern "C" __declspec(dllexport) char* uniq() {
    U8 c, j, sq;
    char* s = lib_buffer;
    for (j = 0;j < 64;j++) s[j] = ' ';
    for (j = 0;j < 14;j++) {
        if (j == 6) j = 8;
        c = pieces[j];
        U64 o = *(PIECES[j]);
        while (o) {
            s[trail0(o)] = c;
            o &= o - 1;
        }
    }
    s[64] = (ToMove ? 'b' : 'w');
    s[65] = (((CASTLES & castle_E1H1) == castle_E1H1) ? 'K' : ' ');
    s[66] = (((CASTLES & castle_E1C1) == castle_E1C1) ? 'Q' : ' ');
    s[67] = (((CASTLES & castle_E8H8) == castle_E8H8) ? 'k' : ' ');
    s[68] = (((CASTLES & castle_E8C8) == castle_E8C8) ? 'q' : ' ');

    if (!ENPSQ) {
        s[69] = ' ';
    }
    else {
        sq = trail0(ENPSQ);
        s[69] = (((((sq >> 3) == 2) ? 'A' : 'a') + (sq & 7)));
    }
    s[70] = 0;
    return lib_buffer;
}


// faster position setup from uniq code
extern "C" void __declspec(dllexport) setasuniq(char* ustr) {

    U8 sq, j, v;
    char c;

    WK = WQ = WR = WB = WN = WP = 0LL;
    BK = BQ = BR = BB = BN = BP = 0LL;

    for (sq = 0; sq < 64; sq++)
    {
        c = ustr[sq];
        if (c != ' ') {
            for (j = 0;j < 14;j++) {
                if (j == 6) j = 8;
                if (pieces[j] == c) { *(PIECES[j]) |= (1LL << sq); }
            }
        }
    }
    ToMove = (ustr[64] == 'w' ? 0 : 1);
    CASTLES = 0LL;
    if (ustr[65] == 'K') CASTLES |= castle_E1H1;
    if (ustr[66] == 'Q') CASTLES |= castle_E1C1;
    if (ustr[67] == 'k') CASTLES |= castle_E8H8;
    if (ustr[68] == 'q') CASTLES |= castle_E8C8;;

    if (ustr[69] != ' ') {
        c = ustr[69];
        v = 5;
        if (c < 'a') {
            c += 32;
            v = 2;
        }
        sq = ((v << 3) | ((c - 'a') & 7));
        ENPSQ = (1LL << sq);
    }
    else {
        ENPSQ = 0LL;
    }

    undo_p = undobuffer;
    mg_po = mg_cnt = mg_uci_list;

}

extern "C" __declspec(dllexport) char* getfen() {
    sGetFEN(lib_buffer);
    return lib_buffer;
}

extern "C" void __declspec(dllexport) setfen(char *fenstr) {
    SetByFEN(fenstr);
}

extern "C" int __declspec(dllexport) movegen() {
    MoveGen(lib_mv);
    int count = lib_mv[0];
    return count;
}

extern "C" __declspec(dllexport) char* legalmoves() {
    sLegalMoves(lib_buffer, lib_mv);
    return lib_buffer;
}

extern "C" int __declspec(dllexport) ucimove(char* ucistr) {
    return uciMove(ucistr);
}

extern "C" int __declspec(dllexport) parseucimoves(char* ucisstr) {

    char* s = ucisstr;
    char uci[8];
    int i, r = 0;
    while ((*s) > 13) {
        while ((*s) == 32) s++;
        for (i = 0; ((*s) > 13) && (i < 4); i++) uci[i] = (*(s++));
        if (((*s) > 13) && (*s) != 32) uci[i++] = (*(s++));
        uci[i] = 0;
        r += uciMove(uci);
    }
    return r;
}


extern "C" void __declspec(dllexport) undomove() {
    UnDoMove();
}

extern "C" __declspec(dllexport) char* parsepgn(char* pgnstr) {
    parse_pgn_moves(pgnstr);
    return lib_buffer;
}

extern "C" bool __declspec(dllexport) ischeck() {
    return IsCheckNow();
}

extern "C" bool __declspec(dllexport) ischeckmate() {
    return IsCheckMateNow();
}

extern "C" U64 __declspec(dllexport) polyglotkey() {
    return getPolyglotKey();
}

extern "C" __declspec(dllexport) char* spolyglotkey() {
    U64 pkey = getPolyglotKey();
    char* u = lib_buffer;
    char* p = (char*) &pkey;
    char Z, H, L;
    for (int ip = 8; ip > 0;) {
        Z = p[--ip];
        H = (Z >> 4)&15;
        L = (Z & 15);
        *(u++) = (((H < 10) ? 48 : 55) + H);
        *(u++) = (((L < 10) ? 48 : 55) + L);
    }
    *(u) = 0;
    return lib_buffer;
}


// Freak mode. Iterations in depth


extern "C" int __declspec(dllexport) i_movegen(int depth) {

    U8* dp = &lib_ii_mv[(4 << 8) * depth];

    MoveGen(dp);
    int count = *(dp++);

    lib_ii_po[depth] = dp;

    return count;
}


extern "C" void __declspec(dllexport) i_domove(int depth) {

    DoMove(lib_ii_po[depth]);
    lib_ii_po[depth] += 4;

}


// get more data on last move
extern "C" __declspec(dllexport) char* i_moveinfo(int depth) {

    char* s = lib_buffer;

    U8* p = lib_ii_po[depth];

    U8 t1 = ((*p) & 15);
    U8 t2 = ((*(p++)) >> 4) & 15;
    U8 f_sq = (*(p++));
    U8 t_sq = (*(p++));
    U8 flags = (*p);
    U8 capt = (flags & 1);
    U8 pr = ((flags & (1 << 1)) ? 1 : 0);
    U8 pr_pc = ((flags >> 2) & 3);
    U8 ecapt = ((flags & (1 << 4)) ? 1 : 0);
    U8 cs = ((flags & (1 << 5)) ? 1 : 0);
    U8 ck = ((flags & (1 << 6)) ? 1 : 0);
    U8 cm = ((flags & (1 << 7)) ? 1 : 0);

    s[0] = pieces[t1];
    s[1] = (f_sq & 7) + 'a';
    s[2] = (f_sq >> 3) + '1';
    s[3] = (capt ? 'x' : '-');
    s[4] = (t_sq & 7) + 'a';
    s[5] = (t_sq >> 3) + '1';
    s[6] = (pr ? pieces[((t_sq >> 3) == 0 ? 8 : 0) + pr_pc] : ' ');
    s[7] = (ck ? '+' : ' ');
    s[8] = (cm ? '#' : ' ');
    s[9] = (capt ? pieces[t2] : ' ');
    s[10] = (cs ? 'O' : ' ');
    s[11] = 0;
    return lib_buffer;

}

extern "C" void __declspec(dllexport) i_skipmove(int depth) {
    lib_ii_po[depth] += 4;
}

extern "C" int __declspec(dllexport) swaptomove() {
    ToMove ^= 1;
    return ToMove;
}

extern "C" int __declspec(dllexport) sidetomove() {
    return ToMove;
}



// Counts pieces by given occupancy
int bitCount(U64 o)
{
    int n = 0;
    while (o) {
        n++;
        o &= o - 1;
    }
    return n;
}

// to verify chess position for normality
U8 seemslegit() {
    U8 b = 1;
    U64 pawns = WP | BP;

    b &= (((pawns & ((1LL << 0) | (1LL << 1) | (1LL << 2) | (1LL << 3) | (1LL << 4) | (1LL << 5) | (1LL << 6) | (1LL << 7) |
        (1LL << 56) | (1LL << 57) | (1LL << 58) | (1LL << 59) | (1LL << 60) | (1LL << 61) | (1LL << 62) | (1LL << 63))) == 0) ? 1 : 0);

    b &= (((bitCount(WK) == 1) && (bitCount(BK) == 1) && (WK != BK)) ? 1 : 0);
    b &= (((bitCount(WP) < 9) && (bitCount(BP) < 9) && (bitCount(WN) < 3) && (bitCount(BN) < 3)) ? 1 : 0);
    b &= (((bitCount(WB) < 3) && (bitCount(BB) < 3) && (bitCount(WR) < 3) && (bitCount(BR) < 3)) ? 1 : 0);
    if (b) {
        // avoid same bishop squares
        U64 o;
        int sq, v, h, d, D;
        for (D = 9, o = WB; o;) {
            sq = (U8)trail0(o);
            v = (sq >> 3); h = (sq & 7); d = (v + h) & 1;
            if (D == 9) D = d;
            else if (D == d) b = 0;
            o &= o - 1;
        }
        for (D = 9, o = BB; o;) {
            sq = (U8)trail0(o);
            v = (sq >> 3); h = (sq & 7); d = (v + h) & 1;
            if (D == 9) D = d;
            else if (D == d) b = 0;
            o &= o - 1;
        }
    }

    // can not be both check+
    if (b) {
        if (IsCheckNow()) {
            ToMove ^= 1;
            if (IsCheckNow()) b = 0;
            ToMove ^= 1;
        }
    }
    // should be pawn normality verify too, but not today
    return b;
}

extern "C" bool __declspec(dllexport) seemslegitpos() {

    return seemslegit();

}

int pieces_cnt(int cz) {

    WOCC = WK | WQ | WR | WB | WN | WP;
    BOCC = BK | BQ | BR | BB | BN | BP;
    OCC = WOCC | BOCC;

    U64 o = (cz == 0 ? OCC : (cz == 1 ? WOCC : BOCC));	//occupancies
    return bitCount(o);
}





extern "C" int __declspec(dllexport) piecescount() {
    return pieces_cnt(0);
}
extern "C" int __declspec(dllexport) whitecount() {
    return pieces_cnt(1);
}
extern "C" int __declspec(dllexport) blackcount() {
    return pieces_cnt(2);
}

extern "C" int __declspec(dllexport) materialdiff() {

    int diff = 0;
    for (U8 j = 0;j < 14;j++) {
        if (j == 6) j = 8;
        U64 o = *(PIECES[j]);
        while (o) {
            diff += matdifvals[j];
            o &= o - 1;
        }
    }
    return diff;
}




BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        // attach to process
        // return FALSE to fail DLL load
        Init_u64_chess();
        break;
    case DLL_THREAD_ATTACH:
        // attach to thread
        break;

    case DLL_THREAD_DETACH:
        // detach from process
        break;

    case DLL_PROCESS_DETACH:
        // detach from thread
        break;

    }
    return TRUE;
}

