/*

    pgn2uci   - tool to prepare chess games for chess engine analysis

    The middleware:
        pgn-file -> pgn2uci ->
            stockfish position startpos moves e2e4 e7e5 ....

Compiler: gcc



for MSVC
#pragma warning(disable:4996);


*/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "u64_chess.h"

char buf_reader[1<<12];
char buf_header[1<<12];
char buf_moves[1<<12];
char buf_ucis[1<<12];

FILE *f_pgn, *f_out;

int St;     // status
char *h, *m, *u;
int GM;

void parse_pgn_moves() {

    char *s=buf_moves;
    int q=0;
    int mn=1;

    U8 mv[4<<8];
    char Pc=0, Pp=0;
    int w, i, N, fcastle=0, prom=0, found=0;
    char *P;
    char c, h1=0,v1=0,h2=0,v2=0;
    U8 ty=0, fl=0, sq1=0, sq2=0;
    U8 *po;

    for(;*s!=0;s++) {
        c = *s;
        if(c=='{') q++;
        if(q>0) *s=' ';       // remove comments
        if(c=='}') q--;
        if(c<=13) *s=' ';   // remove TABs etc...
        if(*s=='e' && *(s+1)=='p') { *s=' '; *(s+1)=' '; }      // ep case
        if(c=='*') *s=' ';
    }
    s=buf_moves;

    SetStartPos();
    MoveGen(mv);


    for(;*s!=0;s++) {
        while(*s==' ') s++;
        if(*s==0) break;

        // skip number
        c = *s;
        if (c >= '1' && c <= '9') {
            while (*s != 0 && *s != '.' && *s != ' ') s++;
        }
        while (*s == '.' || *s == ' ') s++;
        if (*s == 0) break;

        fcastle = 0;
        if((strncmp(s,"0-0-0",5)==0)||(strncmp(s,"O-O-O",5)==0)) {
            fcastle = 2;
        }
        else {
            if((strncmp(s,"0-0",3)==0)||(strncmp(s,"O-O",3)==0)) {
                fcastle = 1;
            }
        }

        // obtain chess move from pgn

        if(!fcastle) {
            Pc = 0; w = 0; Pp = 0;
            h1 = 0; v1 = 0; h2 = 0; v2 = 0;
            while(*s!=' ' && *s!=0) {
                c = *s;
                if(c=='.') w=0;
                if(Pc==0) {
                    P = strchr("QRBNPK",c);
                    if(P!=NULL) { Pc = *P; w=1; }
                }
                if(w<3 && (c=='x'||c=='-'||c==':')) w=3;

                if(c>='a' && c<='h') {
                    if(w<3 && h1==0) { h1=c; w=2; }
                    else if(w<6) { h2=c; w=4; }
                }
                if(w>0 && c>='1' && c<='8') {
                    if(w<3 && v1==0) { v1=c; w=3; }
                    else if(w<6) { v2=c; w=5; }
                }
                if(c=='=') { w=6; c = *(++s); }
                if(w>4) {
                    P = strchr("QRBN",c);
                    if(P!=NULL) Pp = (*P)+32;
                    else {
                        P = strchr("qrbn",c);
                        if(P!=NULL) Pp = *P;
                    }
                }

                s++;
            }
            if(h2==0) { h2=h1; v2=v1; h1=0; v1=0; }
            if(h2==0 && v2==0) continue;
        }

        po = mv;
        N = *(po++);
        found = 0;

        // find this move in the movegen list

        for(i=0; i<N; i++) {

            ty = (*po) & 7;
            sq1 = *(po+1);
            sq2 = *(po+2);
            fl = *(po+3);

            if(fl&32) {
              if (sq1>sq2 && fcastle==2) found = 1;
              if (sq1<sq2 && fcastle==1) found = 1;
            }

            if(!found && ((ty==4 && Pc==0)||(pieces[ty]==Pc))) {
                found = 1;
                if(h1!=0 && h1!=((sq1&7)+'a')) found=0;
                if(v1!=0 && v1!=((sq1>>3)+'1')) found=0;
                if(h2!=0 && h2!=((sq2&7)+'a')) found=0;
                if(v2!=0 && v2!=((sq2>>3)+'1')) found=0;

                prom = 0;
                if(fl&2) {   // promotion
                    prom = pieces[ 8+((fl>>2)&3)];
                    if(Pp!=0 && Pp != prom) found=0;
                    }
                }
            if(found) {
                    DoMove(po);
                    *(u++) = ((sq1&7)+'a');
                    *(u++) = ((sq1>>3)+'1');
                    *(u++) = ((sq2&7)+'a');
                    *(u++) = ((sq2>>3)+'1');
                    if(prom) *(u++) = prom;
                    *(u++) = ' ';
                    *u=0;
                    MoveGen(mv);
                    if(!ToMove) mn++;
                    break;
                    }
            po += 4;
        }

    }
}

// to output
void printout() {
    GM++;
    parse_pgn_moves();
    fprintf(f_out, "//%s\n%s\n\n", buf_header, buf_ucis);
    if(GM%10000==0) printf("%d\n", GM);
}

// get string from pgn file
void parse_pgn_string() {

    char *s = buf_reader;
    int l = strlen(s);
    if(l) {

    if(*s=='[') {
        if(St==2) printout();
        if(St!=1) {
            h = buf_header;
            m = buf_moves;
            u = buf_ucis;
            *h = 0; *m = 0; *u = 0;
            }
        St = 1;
        while(*s!=0) {
            if(*s>13) *(h++)=*s;
            s++;
            }
        *h=0;
        }
    else {
        if(St==1) {
                if(strchr(s,'.')!=NULL) St = 2;
                }
        if(St==2) {
            *(m++)=' ';
            while(*s!=0) {
                if(*s=='.') *(m++) = ' ';
                *(m++) = *(s++);
                }
            *(m++)=' ';
            *m=0;
            }
        }
    }
}


int main(int argc, char *argv[])
{
    if(argc<3) {
        printf("A middleware tool prepares uci moves lists from pgn files.\n");
        printf("The resulting file contains stockfish ready chess moves in uci format\n");
        printf("  stockfish position startpos moves e2e4 e7e5 ....\n");
        printf("Aimed to analyse chess games and help searching positions.\n");
        printf("Note: pgn should be properly notated, fast chess movegen.\n");
        printf("\nUsage: pgn2uci <pgn-file> <output-file>\n\n");
        printf("Chessforeva, mar.2022,bugfix v.03.2024, opensource, gcc-compiled for Win32\n");
        printf("Also, c1_chess project is much sophisticated pgn parser.\n");
//        printf("\n\nPress a key...\n");
//        getch();
        return 0;
    }

    f_pgn = fopen(argv[1],"rt");
    if(f_pgn==NULL) {
            printf("Error: Can not open pgn-file.");
            return 0;
    }

    f_out = fopen(argv[2],"wt");
    if(f_pgn==NULL) {
            fclose(f_pgn);
            printf("Error: Can not write output-file.");
            return 0;
    }

    Init_u64_chess();

    St = 0;
    GM = 0;

    while(!feof(f_pgn)) {
        fgets(buf_reader, 1<<12, f_pgn);
        parse_pgn_string();
    }
    if(St==2) printout();
    printf("%d\nOk\n", GM);

    fclose(f_pgn);
    fclose(f_out);
    return 0;
}
