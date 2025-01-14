// C program to compile with emscripten
//
// Calculates checkmate cases only for given depth.
// Set position and calculate.
// For Webpage developments.

// https://chessforeva.gitlab.io/pz.htm
//
#include <emscripten.h>

char data[10000];

#include "u64_chess.h"

/*

emcc pz.c -O2 -s WASM=1 -o pz.html -s INITIAL_MEMORY=134217728

*/


// Once on startup from javascript

extern "C" char* EMSCRIPTEN_KEEPALIVE _init() {
  Init_u64_chess();
  return &data[0];		// not needed actually
}

// To set data values from javascript
extern "C" void EMSCRIPTEN_KEEPALIVE _set_data( int i, int value ) {
  data[i] = value;
}

// To set position from javascript, after _set_data
extern "C" void EMSCRIPTEN_KEEPALIVE _set_pos() {
  SetByFEN( data );
}

// To read data buffer (answer with found results) from javascript
extern "C" int EMSCRIPTEN_KEEPALIVE _get( int i ) {
  return (int)data[i];
}

// Max.variants for a case no need for long result strings, unlimited 
int HcMAX=9000;

// To set response buffer variants limits to avoid large sizes
extern "C" void EMSCRIPTEN_KEEPALIVE _set_HcMAX( int limit ) {
  HcMAX = limit;
}

int CDEPTH;		// M1=1, M2=2, M3=3,...
int PLYS;   // 1 for M1, 3 for M2, 5 for M3,...;

int d0;
void ss( char *s ) {		// prepare string output into data buffer
	for(;*s!=0;s++) {
		data[d0++] = *s;
		}
}
	// N < 100
void sN( int N ) {			// Move number string output into data buffer
	int n0 = N/10;
	if(n0) {
		data[d0++]=48+n0;
		data[d0++]=48+(N-(n0*10));
		}
	else data[d0++]=48+N;
	data[d0++]='.';
}


U8 FF, Hc_o, wm;

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

int Hc;		// variants found

struct Q_move {		// as a parameter when processing, contains move found to verify
		U8 f;
		U8 t;
		U8 p;
		} Q;

char disp_buf[1000];		// to print board



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
                ret=1;
                }

            }
        else G[d-1].dmi=PLYS;
        }
    return ret;
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

void MM()
{
    U8 i, ck, sk, f, t, p, cw;
	U8 *ptr;

    wm = (!ToMove);

    MoveGen( G[0].movegen );
	G[0].gc = G[0].movegen[0];

    ck = IsCheckNow();      // updates OCC too
	ToMove^=1;
	ck |= IsCheckNow();
	ToMove^=1;
	if(ck) { FF=1; return; }

    Hc = 0; Hc_o = Hc;
    sk=0;
    for (i = 0; i<G[0].gc; i++)
        {
		G[0].i = i;

		ptr = &G[0].movegen[(1+(i<<2))];
		f = *(ptr+1);
		t = *(ptr+2);
		p = (*(ptr+3)>>2)&3;        // promoted 00=q,01=r,10=b,11=n

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
/*
			// We will call to process M1, then M2, then M3,... so -not needed
            if((CDEPTH>1) && ((G[1].dmi<PLYS-1) || G[0].dma<PLYS-1))
                {
                FF=1;   // if can checkmate faster
                Hc = Hc_o;
                }
*/
            Hc_o = Hc;
            if(Hc)
                {
                if(Q.f!=f || Q.t!=t || Q.p!=p) sk++;

                Q.f = f; Q.t = t; Q.p = p;

                }
            }

         UnDoMove();
        }
    if(sk>1) FF=2;

}

void _dispH()
{
    int i,j,k,u,r,t,g,g0, mi;
    char *w,*s;

	// remove lots of variants found during process, only one is needed
    for(i=0; i<Hc-1; i++)
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


	d0 = 0;
	
    if(Hc>0)            // print result found to data
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
                if(j>0) ss(" \0");
                if(j==0 || wm != (j&1)) sN(++g0);
                if(j==0 && !wm) ss("..\0");
                ss(Hstr[0][j]);
                }

            if(Hc>1)
            {
                ss("\n\0");
                for(i=1; i<Hc; i++)
                    {
                    ss(" (\0");
                    for(g=1,j=1; j<PLYS; j++)
                        {
                        if(j>1) ss(" \0");
                        if(wm != (j&1)) g++;
                        if(j==1 || wm != (j&1)) sN(g);
                        if(j==1 && wm) ss("..\0");
						ss(Hstr[i][j]);
                        }
                    ss(")\n\0");
                    }
            }

            for(j=2; j<PLYS; j++)
                {
                if(j>2 || Hc<2) ss(" \0");
                if(wm != (j&1)) g0++;
                if((Hc>1 && j==2) || wm != (j&1)) sN(g0);
                if((Hc>1 && j==2) && !wm) ss("..\0");
                ss(Hstr[0][j]);
                }

            ss("\n\n\0");
        }

	data[d0]=0;		// final 0
}




// The calculation...
extern "C" int EMSCRIPTEN_KEEPALIVE _pz( int depth ) {
	CDEPTH = depth;
	PLYS =(((CDEPTH-1)*2)+1);
    FF = 0;
	MM();
	_dispH();
	return FF;
}