#include <stdio.h>
#include "chesslogic.h"

// CM chess generator rewritten for small Win32 old-style C compiler TinyC
// Chessforeva, 2021
// TinyC

// M1=1 (fast), M2=2, M3=3, and  M4=4 (got one), M5=5 (got none and never)
BYTE CDEPTH=1;

// how much games to find
BYTE PGN_COUNT=10;

// can get other results
BYTE RANDOMIZER=1;

// Pieces max.on board
BYTE PcMAX=8;

// Pieces n-times more to mate the king
BYTE TIMES_MORE_PIECES=4;

// Max.variants for a case in file
BYTE HcMAX=6;

BYTE PLYS;   // 1 for M1, 3 for M2, 5 for M3,...;

FILE *fp;

BYTE FF, ST, Hc_o, wm;

struct CDP {		/* structure calc-in-depth */
        BYTE i;
        BYTE dmi;
        BYTE dma;
		BYTE gc;
		BYTE movegen[500];	// (~100 x 5 max.bytes)
	}G[200 /*PLYS*/];

BYTE H[9000 /*result cases found*/][200 /*PLYS*/];		// A table of results (move number i in gList).
char Hstr[9000][200][20];		// moves as strings

BYTE Hc;		// variants found

struct Q_move {		// as a parameter when processing, contains move found to verify
		BYTE f;
		BYTE t;
		BYTE p;
		} Q;

char disp_buf[1000];		// to print board

void s_dispboard() {
 BYTE v,h, p;
 char *s=disp_buf;
 for(v=7;v>=0;v--) {
	for(h=0;h<8;h++) {
		p = Chess.B[ (v<<3)+h ];
		*(s++) = ( p ? chess_pc[p] : '.');
		}
	*(s++)=13;
 }
}

BYTE startswith_slow( char *sstr, char *ssub )
{
 BYTE f=1;
 for(;(*ssub);) if(*(ssub++)!=*(sstr++)) { f=0; break; }
 return f;
}

#define ISCHECK() CkiSq=(Chess.w?Chess.WK:Chess.BK);ck=isCheck();CkiSq=99;
#define ISOPCHECK() if(!ck){Chess.w^=1;ISCHECK();Chess.w^=1;}

void writeMateVariant()
{
 for(BYTE k=0;k<PLYS;k++)
    {
	H[Hc][k] = G[k].i;
    }
    Hc++;
}

void rmvHs( BYTE q )
{
	BYTE k;
    for( ; Hc>0;  Hc--)
        {
         for(k=q; k>=0; k--)
            {
            if( H[Hc-1][k] != G[k].i ) return;
            }
        };
}

BYTE can_escape( BYTE d );    // definition will be after

// returns 1 if checkmate in next moves
BYTE can_matenext( BYTE d )
{
    BYTE i,m,ck,was;
    BYTE LastPly = ((d==PLYS-1)?1:0);
	
	Chess.gList = G[d].movegen;
    MoveGen();
	G[d].gc = Chess.gc;

    // checkmates only
    for (was=0, i = 0; i<Chess.gc; i++)
        {
		G[d].i = i;
		MkMove(i);
		Chess.gList = G[d+1].movegen;
		CkMt1mv = 1;
		MoveGen();
		
		ck = 0;
		if(!Chess.gc) {
			ISCHECK();		// set ck if checkmate
			}
			
		Chess.gList = G[d].movegen;
		Chess.gc = G[d].gc;
		CkMt1mv = 0;
		
       if(ck)	// also checkmate
            {
            was=1;
            if(G[d-1].dmi>d) G[d-1].dmi=d;
            if(LastPly) writeMateVariant();
            }
			
		UnMkMove();
        }
    if(was) return 1;

    BYTE ret=0;

    // other cases
    for (i = 0; i<Chess.gc; i++)
        {
		G[d].i = i;
        if(d<PLYS-1)
            {
            MkMove(i);
            G[d].dma = 0;
            m = can_escape(d+1);
			Chess.gList = G[d].movegen;
			Chess.gc = G[d].gc;
            if(G[d].dma<G[d-1].dmi) G[d-1].dmi = G[d].dma;
            UnMkMove();

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
BYTE can_escape( BYTE d )
{
    BYTE i,m;

	Chess.gList = G[d].movegen;
    MoveGen();
	G[d].gc = Chess.gc;

    if(!Chess.gc) { rmvHs(d-1); return 1; }        // stalemate
    for (i = 0; i<Chess.gc; i++)
        {
		G[d].i = i;
		MkMove(i);

        G[d].dmi = PLYS;
        m = can_matenext(d+1);
		Chess.gList = G[d].movegen;
		Chess.gc = G[d].gc;
        if(G[d-1].dma==0 || G[d-1].dma > G[d].dmi) G[d-1].dma = G[d].dmi;
        UnMkMove();
        if(!m) { rmvHs(d-1); return 1; }
        }
    return 0;
}

void MM( BYTE st )
{
    BYTE i, ck, sk, f, t, p, cw;
	BYTE *ptr;
	
    ST = st;
    wm = (Chess.w);

	Chess.gList = G[0].movegen;
    MoveGen();
	G[0].gc = Chess.gc;
	
    if(ST>8 || ST==1 || ST==2)
    {
		ISCHECK();
		ISOPCHECK();
        if(ck) { FF=1; return; }
    }

    Hc = 0; Hc_o = Hc;
    sk=0;
    for (i = 0; i<Chess.gc; i++)
        {
		G[0].i = i;
		Chess.gList = G[0].movegen;
		
		ptr = Chess.gList + (5*i);
		f = *(ptr);
		t = *(++ptr);
		p = *(++ptr);
	
        if(ST==1 && !(Q.f==f && Q.t==t && Q.p==p)) continue;
        if(ST==2 && (Q.f==f && Q.t==t && Q.p==p)) continue;
		
        G[0].dma = 0;

		ck = 0;

		// verify checkmates
			
		MkMove(i);
		Chess.gList = G[1].movegen;
		CkMt1mv = 1;
		MoveGen();
			
		if(!Chess.gc) {
			ISCHECK();			// set ck if checkmate
			}
		CkMt1mv = 0;
		Chess.gList = G[0].movegen;
		Chess.gc = G[0].gc;
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
			Chess.gc = G[0].gc;
			Chess.gList = G[0].movegen;
			
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
			
         UnMkMove();
		 Chess.gc = G[0].gc;
		 Chess.gList = G[0].movegen;
        }
    if((ST>8 || ST==1) && sk!=1) FF=1;
    if(ST==2 && sk!=0) FF=1;
}

int _AbsDiff(BYTE x,BYTE y) { return (x>y ? x-y : y-x); }
int RND() {
	return (rand() + RANDOMIZER);
}

void _dispbo( BYTE p )
{
char fname[100];
if(p==1)
    {
    sprintf(fname,"M%d_generated.pgn",CDEPTH);
    fp = fopen(fname, "w+");
    printf("Writing to file %s\n",fname);
    }
if(p==2) fclose(fp);
if(p==0)
    {
	s_dispboard();
	fprintf(fp,"{\n%s\n}\n", disp_buf );
    }
}

void _dispFen()
{
char fen[200];
getFEN( fen );
fprintf(fp,"\n[FEN \"%s\"]\n\n", fen);
}

void _dispH()
{
    BYTE i,j,k,u,r,t,g,g0;
	
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
    for(i=0 ;i<Hc-1; i++)
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
     t=( t==1 ? 4 : (t<4 ? t-1 : t+1));
    }

    if(Hc>0)            // print result found to file
        {
			for(i=0 ;i<Hc; i++)
					{					
					for(j=0; j<PLYS; j++)
						{
						Chess.gList = G[j].movegen;
						MoveGen();
						MkMove(H[i][j]);
						
						Chess.gList = G[j+1].movegen;
						MoveToString( Hstr[i][j]);
						}
					while((j--)>0) UnMkMove();
					}
					
			Chess.gList = G[0].movegen;
			CkMt1mv = 0;
			MoveGen();
			G[0].gc = Chess.gc;
		
            // set limit
            if(Hc>HcMAX) Hc = HcMAX;

            for(g0=0,j=0; j<2 && j<PLYS; j++)
                {
                if(j>0) fprintf(fp," ");
                if(j==0 || wm != (j&1)) fprintf(fp,"%d.", ++g0);
                if(j==0 && !wm) fprintf(fp,"..");
                fprintf(fp,"%s", Hstr[0][j]);
                }

            if(Hc>1)
            {
                fprintf(fp,"\n");
                for(i=1; i<Hc; i++)
                    {
                    fprintf(fp," (");
                    for(g=1,j=1; j<PLYS; j++)
                        {
                        if(j>1) fprintf(fp," ");
                        if(wm != (j&1)) g++;
                        if(j==1 || wm != (j&1)) fprintf(fp,"%d.",g);
                        if(j==1 && wm) fprintf(fp,"..");
						fprintf(fp,"%s", Hstr[i][j]);
                        }
                    fprintf(fp,")\n");
                    }
            }
			
            for(j=2; j<PLYS; j++)
                {
                if(j>2 || Hc<2) fprintf(fp," ");
                if(wm != (j&1)) g0++;
                if((Hc>1 && j==2) || wm != (j&1)) fprintf(fp,"%d.", g0);
                if((Hc>1 && j==2) && !wm) fprintf(fp,"..");
                fprintf(fp,"%s", Hstr[0][j]);
                }

            fprintf(fp,"\n\n");
        }
    else fprintf(fp,"{ERROR Hc}\n\n");
}

BYTE Cnt;
BYTE _pieMas[200];
BYTE _pieMc;
const BYTE _pieP[] = {0,1,1,2,2,2,8}; //p...q
BYTE V[16];  // count of pieces by type
BYTE Bi[2];  // two similar bishops

void prepMas()
{
    BYTE i,j,k;
    _pieMc = 0;
    for(i=2; i<7; i++)
    {
     V[7-i] = _pieP[i]; V[17-i] = _pieP[i];
     for(j=0; j<_pieP[i]; j++)
      for(k=0; k<TIMES_MORE_PIECES; k++)
            {
            if(k==0 || (Chess.w)) _pieMas[_pieMc++] = 7-i;
            if(k==0 || (!Chess.w)) _pieMas[_pieMc++] = 17-i;
            }
    }
}

void CM_calculate()
{
    int r, u, f, c, wk, bk, o, opmove, i, q, ck, w, t, m;
	int p=0, px=0,pv=0, kk, kx, kv, kd=0;
    char pc;

    PLYS =(((CDEPTH-1)*2)+1);
	
    _dispbo(1);
    printf("Searching...\n");

    for(Cnt=0;Cnt<PGN_COUNT;)
    {
    for(f=0;!f;)
    {
        setFEN( (char *)"/////// w - - 0 1" );
        Chess.w = ( RND()&1 ? 1 : 0);

        prepMas();
        for(r=0;;r++)
        {
            wk = (BYTE) (RND() & 63);
            bk = (BYTE) (RND() & 63);
            BYTE wx = wk&7, wy=wk>>3, bx=bk&7, by=bk>>3;
            if( _AbsDiff(wx,bx)>1 || _AbsDiff(wy,by)>1 ) break;
            if(r>300)
                {
                fprintf(fp,"RAND ERROR!\n");
                break;
                }
        }

        Chess.B[wk] = 6;	// K Pwk
		Chess.WK = wk;
        Chess.B[bk] = 16;	// k Pbk
		Chess.BK = bk;
        
        kk=(Chess.w ? bk : wk); kx = kk&7; kv = kk>>3;

        r=0;    // repeats unsuccess
        for(c=2; !f && c<PcMAX && r<200;r++)
        {
          if(c<4) { p = (BYTE)(RND() & 63); px = p&7; pv = p>>3; }
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
                p = (BYTE)(RND() & 63); px = p&7; pv = p>>3;
                break;
                }
            }
          }
		  		
          o = RND() % _pieMc;
          q = _pieMas[o];       // put this piece on random square

            if(q==3 || q==13)   // avoid same bishops
            {
             t=(q==3 ? 0 : 1);
             if(V[q]==2) Bi[t] = (px+pv)&1;
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
          opmove = (Chess.w^1);
          if(!Chess.B[p])
          {
            if(V[q]==0) continue;

            if(q==1 || q==11)
            {
             for(m=0,w=1;w<7;w++)
             {
                 t = (w<<3)+px;
                 pc = Chess.B[t];
                 if(pc==q) m++;
                 if(q==1 && pc==11 && p>t) { m=2; break; }
                 if(q==11 && pc==1 && p<t) { m=2; break; }
             }
             if(m>1) continue;
            }

            Chess.B[p]= q;

			ISCHECK();
			ISOPCHECK();

            if(ck) { Chess.B[p] = 0; }
            else
            {
             if(0 && c<5)
                {
                 f=0;
                }
             else
                {
                FF=0;
                Q.f = 99; Q.t = 99; Q.p = 99;
                MM(9);
                f = 1-FF;
                }

             if(f)      // Optimize, remove redundant pieces
             {
                m = 0;
                for(i=0;i<64;i++)
                {
                pc = Chess.B[i];
                if(pc && pc!=6 && pc!=16)	// remove piece, but not kings
                    {
                    Chess.B[i] = 0;
                    FF = 0; MM(1);
                    if(!FF)
                        {
                        MM(2);
                        if(!FF) m=1;
                        }
                    if(FF) Chess.B[i]=pc;		// can not remove
                    }
                }

             }

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
            if(Chess.B[p]!=0) V[q]--;
          }
          }
        }

    }

    }
    _dispbo(2);
}

FILE *cfg;
char buff[1000];
static char *fnCONFIG = (char *)"CM_generator.ini";
static char *cfgtxt[] = {
 (char *)";",
 (char *)"; Configuration file for simple chess checkmate generator.",
 (char *)"; Absolute freeware by Chessforeva, 2021, TinyC version",
 (char *)";",
 (char *)"; Fast for M1(fast),M2(so,so),M3(slow),M4...",
 (char *)"; Each case has only one first move solution!",
 (char *)"; The algorithm reduces amount of needed pieces for each case.",
 (char *)"; Results will be written in PGN file in long notation format,",
 (char *)";  because for other software it's nice to have all data :)",
 (char *)";",
 (char *)";",
 (char *)"; Parameters:",
 (char *)"",
 (char *)"",
 (char *)"; Moves to mate M1=1, M2=2, M3=3, ...",
 (char *)"CDEPTH=1",
 (char *)"",
 (char *)"; Positions till stop, otherwise ctrl-break",
 (char *)"PGN_COUNT=10",
 (char *)"",
 (char *)"; Randomizer (any value) to get other results",
 (char *)"RANDOMIZER=1",
 (char *)"",
 (char *)"; Maximum pieces on board (- faster)",
 (char *)"PcMAX=8",
 (char *)"",
 (char *)"; Pieces n-times more to mate the king (+ faster)",
 (char *)"TIMES_MORE_PIECES=4",
 (char *)"",
 (char *)"; Maximum variants for each case in file",
 (char *)";  (not all, some other if possible)",
 (char *)"HcMAX=6",
 (char *)"",
 (char *)""
 };


void ReadConfig()
{
    BYTE i,a;
	int val;
    cfg = fopen(fnCONFIG,"r");
    if(cfg==NULL)
        {
        cfg = fopen(fnCONFIG,"w");
        for(i=0;i<33;i++) fprintf(cfg,"%s\n", cfgtxt[i]);
        fclose(cfg);
        cfg = fopen(fnCONFIG,"r");
        }
    if(cfg==NULL) printf("Can not read configuration file!\n");
    else
        {
        for(;!feof(cfg);)
            {
            fgets(buff, 1000, cfg);
            for(i=0,a=-1;i<1000;i++)
            {
                char c=buff[i];
                if(c==0 || c==';') break;
                if(c=='=') a=i;
            }
            if(a>=0)
                {
                 val=atoi((char *)buff+(a+1));  // to integer
                 if(startswith_slow(buff,(char *)"CDEPTH")) CDEPTH=val;
                 if(startswith_slow(buff,(char *)"PGN_COUNT")) PGN_COUNT=val;
                 if(startswith_slow(buff,(char *)"RANDOMIZER")) RANDOMIZER=val;
                 if(startswith_slow(buff,(char *)"PcMAX")) PcMAX=val;
                 if(startswith_slow(buff,(char *)"TIMES_MORE_PIECES")) TIMES_MORE_PIECES=val;
                 if(startswith_slow(buff,(char *)"HcMAX")) HcMAX=val;
                }
            }
        fclose(cfg);
        }

}


int main()
{
	OnloadInitChessGame();

    printf("Simple chess checkmate generator\nConfiguration file %s\n", fnCONFIG);
    ReadConfig();
    CM_calculate();
    printf("Finished\n");
    return 0;
}
