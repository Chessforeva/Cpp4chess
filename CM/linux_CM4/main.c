//
// for linux gcc usage
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "u64_chess.h"

// Chessforeva, feb.2022

// M1=1, M2=2 (fast), M3=3 (so,so), M4=4 (slow), M5=5 (got one)
int CDEPTH=4;

// how much games to find
int PGN_COUNT=3;

// can get other results
int RANDOMIZER=156;

// Pieces max.on board
int PcMAX=8;

// Pieces n-times more to mate the king
int TIMES_MORE_PIECES=4;

// Max.variants for a case in file
int HcMAX=6;

// Pattern on/off
int PATTERN=0;

int PLYS;   // 1 for M1, 3 for M2, 5 for M3,...;

U8 FF, ST, Hc_o, wm;

struct CDP {		/* structure calc-in-depth */
        U8 i;
        U8 dmi;
        U8 dma;
		U8 gc;
		U8 movegen[4<<8];
		char moves_str[2048];
	}G[200 /*PLYS*/];

int H[9000 /*result cases found*/][200 /*PLYS*/];		// A table of results (move number i).
char Hstr[9000][200][20];		// moves as strings

U64 Pattern = 0LL;
int pattern_row = 7;

int Hc;		// variants found

struct Q_move {		// as a parameter when processing, contains move found to verify
		U8 f;
		U8 t;
		U8 p;
		} Q;

char disp_buf[1000];		// to print board

U8 startswith_slow( char *sstr, char *ssub )
{
 U8 f=1;
 for(;(*ssub);) if(*(ssub++)!=*(sstr++)) { f=0; break; }
 return f;
}


void writeMateVariant()
{
 for(int k=0;k<PLYS;k++)
    {
	H[Hc][k] = G[k].i;
    }
 Hc++;
}

void rmvHs( U8 q )
{
	int k;
    for( ; Hc>0;  Hc--)
        {
         for(k=q; k>=0; k--)
            {
            if( H[Hc-1][k] != G[k].i ) return;
            }
        };
}

U8 can_escape( U8 d );    // definition will be after

// returns 1 if checkmate in next moves
U8 can_matenext( U8 d )
{
    U8 i,m,was;
    U8 LastPly = ((d==PLYS-1)?1:0);

    MoveGen( G[d].movegen );
	G[d].gc = G[d].movegen[0];

    // checkmates only
    for (was=0, i = 0; i<G[d].gc; i++)
        {
        G[d].i = i;
        if( G[d].movegen[1+(i<<2)+3] & 128) {   // if checkmate flag is set
            was=1;
            if(G[d-1].dmi>d) G[d-1].dmi=d;
            if(LastPly) writeMateVariant();
            }
        }

    if(was) return 1;

    U8 ret=0;

    // other cases
    for (i = 0; i<G[d].gc; i++)
        {
		G[d].i = i;
        if(d<PLYS-1)
            {
            DoMove( &G[d].movegen[1+(i<<2)] );
            G[d].dma = 0;
            m = can_escape(d+1);
			if(G[d].dma<G[d-1].dmi) G[d-1].dmi = G[d].dma;
            UnDoMove();

            if(!m)
                {
                if(G[d].dma<PLYS-1) rmvHs(d-1);     // not these results
                if(ST==10) ret=1;
                else return 1;
                }

            }
        else G[d-1].dmi=PLYS;
        }
    return (ST==10 ? ret : 0);
}

// returns 1 if can escape or later
U8 can_escape( U8 d )
{
    U8 i,m;

	MoveGen( G[d].movegen );
	G[d].gc = G[d].movegen[0];

    if(!G[d].gc) { rmvHs(d-1); return 1; }        // stalemate
    for (i = 0; i<G[d].gc; i++)
        {
		G[d].i = i;
		DoMove( &G[d].movegen[1+(i<<2)] );
        G[d].dmi = PLYS;
        m = can_matenext(d+1);
		if(G[d-1].dma==0 || G[d-1].dma > G[d].dmi) G[d-1].dma = G[d].dmi;
        UnDoMove();
        if(!m) { rmvHs(d-1); return 1; }
        }
    return 0;
}

void MM( U8 st )
{
    U8 i, ck, sk, f, t, p, cw;
	U8 *ptr;

    ST = st;
    wm = (!ToMove);

    MoveGen( G[0].movegen );
	G[0].gc = G[0].movegen[0];

    if(ST>8 || ST==1 || ST==2)
    {
        ck = IsCheckNow();      // updates OCC too
		ToMove^=1;
		ck |= IsCheckNow();
		ToMove^=1;
		if(ck) { FF=1; return; }
    }

    Hc = 0; Hc_o = Hc;
    sk=0;
    for (i = 0; i<G[0].gc; i++)
        {
		G[0].i = i;

		ptr = &G[0].movegen[(1+(i<<2))];
		f = *(ptr+1);
		t = *(ptr+2);
		p = (*(ptr+3)>>2)&3;        // promoted 00=q,01=r,10=b,11=n

        if(ST==1 && !(Q.f==f && Q.t==t && Q.p==p)) continue;
        if(ST==2 && (Q.f==f && Q.t==t && Q.p==p)) continue;

        G[0].dma = 0;

		ck = (*(ptr+3)&128);    // if checkmate in 1 move

		// verify checkmates

		DoMove(ptr);
		cw = 0;

        if(CDEPTH==1)
            {
            if(ck)
                {
                writeMateVariant(); cw = 1;
                }
            }
        else
            {
			if(ck) {
				FF = 1;		// if already checkmate
				break;
				}
            G[1].dmi = PLYS-1;
            }

        if( cw || (CDEPTH>1 && (!can_escape(1))))
            {

            if((CDEPTH>1) && ((G[1].dmi<PLYS-1) || G[0].dma<PLYS-1))
                {
                FF=1;   // if can checkmate faster
                Hc = Hc_o;
                }

            Hc_o = Hc;
            if(Hc)
                {
                if(ST<=8 || (Q.f!=f || Q.t!=t || Q.p!=p)) sk++;

                if(ST>8) { Q.f = f; Q.t = t; Q.p = p; }

                if(ST>8 && sk>1) break;

                }
            }

         UnDoMove();
        }
    if((ST>8 || ST==1) && sk!=1) FF=1;
    if(ST==2 && sk!=0) FF=1;
}

int _AbsDiff(U8 x,U8 y) { return (x>y ? x-y : y-x); }

U32 r_x = 30903, r_y = 30903, r_z = 30903, r_w = 30903, r_carry = 0;
U32 RND() {
    U32 t;
    r_x = r_x * 69069 + RANDOMIZER;   r_y ^= r_y << 13;
    r_y ^= r_y >> 17;   r_y ^= r_y << 5;   t = (r_w << 1) + r_z + r_carry;
    r_carry = ((r_z >> 2) + (r_w >> 3) + (r_carry >> 2)) >> 30;
    r_z = r_w;   r_w = t;   return r_x + r_y + r_w;
}

void _dispbo( U8 p )
{
//char fname[100];
//if(p==1)
//    {
//    sprintf(fname,"M%d_generated.pgn",CDEPTH);
//    fp = fopen(fname, "w+");//
//    printf("Writing to file %s\n",fname);
//    }
//if(p==2) fclose(fp);

if(p==0)
    {
	sBoard(disp_buf);
	printf("{\n%s\n}\n", disp_buf );
    }
}

void _dispFen()
{
char fen[200];
sGetFEN( fen );
printf("\n[FEN \"%s\"]\n\n", fen);
}

void _dispH()
{
    int i,j,k,u,r,t,g,g0, mi;
    char *w,*s;

    if(1)
     for(i=0 ; i<Hc-1; i++)
      for(j=i+1 ;j<Hc; j++)
        {
        for(r=1,k=0; r && k<PLYS; k+=2)
            if(H[i][k]!=H[j][k]) r=0;

        if(r)
            {           // remove j-variant
            for(u=j+1; u<Hc; u++)
             for(k=0; k<PLYS; k++)
                {
                H[u-1][k] = H[u][k];
                }
            Hc--;
            j--;
            }
        }

    for(t=3; t==3 || (t<PLYS && Hc>HcMAX);)        // remove some more
    {
    for(i=0 ;i<Hc-1; i++){
     for(j=i+1 ;j<Hc; j++)
        {
        if( H[i][t]==H[j][t] )
            {           // remove j-variant
            for(u=j+1; u<Hc; u++)
             for(k=0; k<PLYS; k++)
                {
                H[u-1][k] = H[u][k];
                }
            Hc--;
            j--;
            }
        }
        }
     t=( t==1 ? 4 : (t<4 ? t-1 : t+1));
    }

    if(Hc>0)            // print result found to file
        {

			for(i=0 ;i<Hc; i++)
					{
					for(j=0; j<PLYS; j++)
						{
						MoveGen( G[j].movegen );

						sLegalMoves( G[j].moves_str, G[j].movegen );
						DoMove( &G[j].movegen[1+(H[i][j]<<2)] );

                        w = G[j].moves_str;
                        s = Hstr[i][j];
                        mi = 0;
                        while(mi<H[i][j]) {
                            while(*w!=' ') w++;
                            w++;
                            mi++;
                            }
                        while(*w!=' ') {
                            *(s++)=*(w++);
                            }
                        *s=0;
						}
					while((j--)>0) UnDoMove();
					}

            // set limit
            if(Hc>HcMAX) Hc = HcMAX;

            for(g0=0,j=0; j<2 && j<PLYS; j++)
                {
                if(j>0) printf(" ");
                if(j==0 || wm != (j&1)) printf("%d.", ++g0);
                if(j==0 && !wm) printf("..");
                printf("%s", Hstr[0][j]);
                }

            if(Hc>1)
            {
                printf("\n");
                for(i=1; i<Hc; i++)
                    {
                    printf(" (");
                    for(g=1,j=1; j<PLYS; j++)
                        {
                        if(j>1) printf(" ");
                        if(wm != (j&1)) g++;
                        if(j==1 || wm != (j&1)) printf("%d.",g);
                        if(j==1 && wm) printf("..");
						printf("%s", Hstr[i][j]);
                        }
                    printf(")\n");
                    }
            }

            for(j=2; j<PLYS; j++)
                {
                if(j>2 || Hc<2) printf(" ");
                if(wm != (j&1)) g0++;
                if((Hc>1 && j==2) || wm != (j&1)) printf("%d.", g0);
                if((Hc>1 && j==2) && !wm) printf("..");
                printf("%s", Hstr[0][j]);
                }

            printf("\n\n");
        }
    else printf("{ERROR Hc}\n\n");
}

int Cnt;
U8 _pieMas[200];
U64 *PIEC_MAS[] = { 0LL,
    &WP, &WN, &WB, &WR, &WQ, &WK, 0LL, 0LL, 0LL, 0LL,
    &BP, &BN, &BB, &BR, &BQ, &BK, 0LL, 0LL, 0LL, 0LL
    };

U8 _pieMc;
const U8 _pieP[] = {0,1,1,2,2,2,8}; //p...q
U8 V[16];  // count of pieces by type
U8 Bi[2];  // two similar bishops

void prepMas()
{
    int i,j,k;
    _pieMc = 0;
    for(i=2; i<7; i++)
    {
     V[7-i] = _pieP[i]; V[17-i] = _pieP[i];
     for(j=0; j<_pieP[i]; j++)
      for(k=0; k<TIMES_MORE_PIECES; k++)
            {
            if(k==0 || (!ToMove)) _pieMas[_pieMc++] = 7-i;
            if(k==0 || (ToMove)) _pieMas[_pieMc++] = 17-i;
            }
    }
}



void CM_calculate()
{
    int r, u, f, c, wk, bk, o, i, j, q, ck, w, t, m;
	int p=0, px=0,pv=0, kk, kx, kv, kd=0;
    U8 pc;
    U64 tb;

    PLYS =(((CDEPTH-1)*2)+1);

    _dispbo(1);
    printf("Searching...\n");

    for(Cnt=0;Cnt<PGN_COUNT;)
    {
    for(f=0;!f;)
    {
        // clear board
        WK=WQ=WR=WB=WN=WP=0LL;
        BK=BQ=BR=BB=BN=BP=0LL;
        ENPSQ = CASTLES = OCC = 0LL;
        undo_p = undobuffer;
        mg_po = mg_cnt = mg_uci_list;

        ToMove = ( RND()&1 ? 1 : 0);

        prepMas();
        for(r=0;;r++)
        {
            wk = (U8) (RND() & 63);
            bk = (U8) (RND() & 63);
            U8 wx = wk&7, wy=wk>>3, bx=bk&7, by=bk>>3;
            if( _AbsDiff(wx,bx)>1 || _AbsDiff(wy,by)>1 ) break;
            if(r>300)
                {
                printf("RAND ERROR!\n");
                break;
                }
        }

        WK = (1LL<<wk);
        BK = (1LL<<bk);
        OCC|= (WK|BK);

        kk=(!ToMove ? bk : wk); kx = kk&7; kv = kk>>3;

        r=0;    // repeats unsuccess
        for(c=2; !f && c<PcMAX && r<999;r++)
        {
          if(c<4) { p = (U8)(RND() & 63); px = p&7; pv = p>>3; }
          else
          {

                     // p - try closer to king
            for(u=0;;u++)
            {
            kd = (c<6 ? 3 : 1);
            px=((p++)& kd); pv=((p++) & kd);
            px= kx + (u&1 ? -kd : (kk&1 ? 0 : kd));
            pv= kv + ((p+kk)&1 ? kd : ((u+kk)&1 ? 0 : -kd));
            if((px>=0) && (px<8) && (pv>=0) && (pv<8))
                {
                p=(pv<<3)+px;
                break;
                }
            if(u>30)
                {
                p = (U8)(RND() & 63); px = p&7; pv = p>>3;
                break;
                }
            }
          }

          o = RND() % _pieMc;
          q = _pieMas[o];       // put this piece on random square p

            if(q==3 || q==13)   // avoid same bishops
            {
             t= (q>10 ? 1 : 0);
             if(V[q]==2) {
                    Bi[t] = (px+pv)&1;
                    }
             else {
                    if(V[q]==1) {
                        while(Bi[t]==((px+pv)&1))
                            {
                            p++; if(p==64) p=1;
                            px = p&7; pv = p>>3;
                            }
                    }
                }
            }

          if((q!=1 && q!=11 ) || (pv!=0 && pv!=7 && pv!=(q==1?6:1)))
          {

          if(!( OCC&(1LL<<p)))
          {
            if(V[q]==0) continue;

            if(PATTERN) {

                if(!(Pattern & (1LL<<p))) {
                        continue;
                }
            }

            if(q==1 || q==11)
            {
             for(m=0,w=1;w<7;w++)
             {
                 t = (w<<3)+px;
                 tb = (1LL<<t);
                 if(q==1 && (WP&tb)) m++;
                 if(q==11 && (BP&tb)) m++;
                 if(q==1 && (BP&tb) && p>t) { m=2; break; }
                 if(q==11 && (WP&tb) && p<t) { m=2; break; }
             }
             if(m>1) continue;
            }

            *(PIEC_MAS[q])|=(1LL<<p);
            ck = IsCheckNow();          // this updates OCC too
            ToMove^=1;
            ck |= IsCheckNow();
            ToMove^=1;

            V[q]--;

            if(ck) { *(PIEC_MAS[q])&=~(1LL<<p);
                        V[q]++;
                    }
            else
            {

             if(PATTERN && Pattern!=OCC)
                {
                continue;
                }
             else
                {
                FF=0;
                Q.f = 99; Q.t = 99; Q.p = 99;
                MM(9);
                f = 1-FF;
                }

             if(f && (!PATTERN))      // Optimize, remove redundant pieces
             {
                m = 0;
                for(i=0;i<64;i++)
                {
                pc = 0;
                tb = (1LL<<i);
                for(j=1; j<7; j++) {
                    if( *(PIEC_MAS[j]) & tb ) { pc=j; break; }
                }
                if(!pc) {
                    for(j=11; j<17; j++) {
                        if( *(PIEC_MAS[j]) & tb ) { pc=j; break; }
                    }
                }
                if(pc && pc!=6 && pc!=16)	// remove piece, but not kings
                    {
                    *(PIEC_MAS[pc])&=~tb;
                    FF = 0; MM(1);
                    if(!FF)
                        {
                        MM(2);
                        if(!FF) m=1;
                        }
                    if(FF) {
                            		// can not remove
                            *(PIEC_MAS[pc])|=tb;
                            }
                    }
                }

             }

            WOCC = WK|WQ|WR|WB|WN|WP;
            BOCC = BK|BQ|BR|BB|BN|BP;
            OCC = WOCC|BOCC;

            // Verify once again after
            if(f)
                {

                FF=0;
                Q.f = 99; Q.t = 99; Q.p = 99;
                MM(10);
                f = 1-FF;
                }

             if(f)
                {
                printf("%d\n",++Cnt);

                _dispFen();
                _dispH();
                _dispbo(0);
                r=0;
                }
                c++;

            }
          }
          }
        }

    }

    }
    _dispbo(2);
}

void addPattern( char *rowbits ) {
    int h=0, sq;
    char *s=rowbits;
    while(*s!=0) {
        if(*s=='+' && h<7) h++;
        if(*s>='A' && *s<='Z') {
            sq = (pattern_row<<3)|h;
            Pattern|=(1LL<<sq);
            if(h<7) h++;
        }
        s++;
    }
    if(pattern_row>0) pattern_row--;
}


char buff[1000];


int main()
{
    Init_u64_chess();

    printf("A fast chess checkmate generator\n\n");
    CM_calculate();
    printf("Finished\n");
    return 0;
}
