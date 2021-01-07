#include <stdio.h>
#include "c1_chess.h"

// A simple chess checkmate generator
// Chessforeva, 2016
// GCC

// M1=1 (fast), M2=2, M3=3, M4=4 (got some in a long day), M5=5 (got none and never)
int CDEPTH=1;

// how much games to find
int PGN_COUNT=10;

// can get other results
int RANDOMIZER=1;

// Pieces max.on board
int PcMAX=8;

// Pieces n-times more to mate the king
int TIMES_MORE_PIECES=4;

// Max.variants for a case in file
int HcMAX=6;

int PLYS;   // 1 for M1, 3 for M2, 5 for M3,...;

c1_chess B;


FILE *fp;

int FF, ST, Hc_o, wm;

struct CDP {		/* structure calc-in-depth */
        c1_chess B;
        char mv[50];
        int i;
        int dmi;
        int dma;
	}G[200 /*PLYS*/];

struct mo
    {
        int f;
        int t;
        char p;
        char mv[12];
    };

struct Hs
    {
     mo m[200 /*PLYS*/];
    }H[9000 /*result cases found*/] ;
int Hc;

mo Q;

#define strcpy_slow B.setString

int startswith_slow( char *sstr, char *ssub )
{
 int f=1;
 for(;(*ssub);) if(*(ssub++)!=*(sstr++)) { f=0; break; }
 return f;
}

void writeMateVariant()
{
 for(int k=0;k<PLYS;k++)
    {
    H[Hc].m[k].f = G[k].B.genml[G[k].i].f;
    H[Hc].m[k].t = G[k].B.genml[G[k].i].t;
    H[Hc].m[k].p = G[k].B.genml[G[k].i].p;
    strcpy_slow(H[Hc].m[k].mv,G[k].mv);
    //printf("%s ",G[k].mv);
    }
    Hc++;
    //printf("\n");
}

void rmvHs( int q )
{
    for( ; Hc>0;  Hc--)
        {
         for(int k=q; k>=0; k--)
            {
            if(!( H[Hc-1].m[k].f == G[k].B.genml[G[k].i].f &&
                    H[Hc-1].m[k].t == G[k].B.genml[G[k].i].t &&
                    H[Hc-1].m[k].p == G[k].B.genml[G[k].i].p )) return;
            }
        };
}

int can_escape( int d );    // definition will be after

// returns 1 if checkmate in next moves
int can_matenext( int d )
{
    int i,m,cm,was;
    int LastPly = ((d==PLYS-1)?1:0);

    G[d].B.copy_pos_only(&G[d-1].B);
    G[d].B.moveGen();

    // checkmates only
    for (was=0, G[d].i = 0; G[d].i<G[d].B.gc; G[d].i++)
        {
        i = G[d].i;
        G[d].B.I2long(i,G[d].mv);
        cm = (G[d].B.genml[i].c=='#' ? 1:0);
        if(cm)
            {
            was=1;
            if(G[d-1].dmi>d) G[d-1].dmi=d;
            if(LastPly) writeMateVariant();
            }

        }
    if(was) return 1;

    int ret=0;

    // other cases
    for (G[d].i = 0; G[d].i<G[d].B.gc; G[d].i++)
        {
        i = G[d].i;
        G[d].B.I2long(i,G[d].mv);
        cm = (G[d].B.genml[i].c=='#' ? 1:0);
        if(d<PLYS-1)
            {
            G[d].B.mkmove(i);
            G[d].dma = 0;
            m = can_escape(d+1);
            if(G[d].dma<G[d-1].dmi) G[d-1].dmi = G[d].dma;
            G[d].B.unmkmove();

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
int can_escape( int d )
{
    int i,m;
    G[d].B.copy_pos_only(&G[d-1].B);
    G[d].B.moveGen();
    if(!G[d].B.gc) { rmvHs(d-1); return 1; }        // stalemate
    for (G[d].i = 0; G[d].i<G[d].B.gc; G[d].i++)
        {
        i = G[d].i;
        G[d].B.I2long(i,G[d].mv);
        G[d].B.mkmove(i);
        G[d].dmi = PLYS;
        m = can_matenext(d+1);
        if(G[d-1].dma==0 || G[d-1].dma > G[d].dmi) G[d-1].dma = G[d].dmi;
        G[d].B.unmkmove();
        if(!m) { rmvHs(d-1); return 1; }
        }
    return 0;
}

void MM( int st )
{
    int i, ch, sk, f, t, p, cm;

    ST = st;
    wm = (B.sm>0 ? 1: 0);
    G[0].B.copy_pos_only(&B);
    G[0].B.moveGen();
    if(ST>8 || ST==1 || ST==2)
    {
        ch = G[0].B.isCheck();
        if(!ch)
            {
            G[0].B.sm=-G[0].B.sm; ch=G[0].B.isCheck(); G[0].B.sm=-G[0].B.sm;
            }
        if(ch) { FF=1; return; }
    }

    Hc = 0; Hc_o = Hc;
    sk=0;
    for (G[0].i = 0;(!FF) &&  G[0].i<G[0].B.gc; G[0].i++)
        {
        i = G[0].i;
        G[0].B.I2long(i,G[0].mv);
        f = G[0].B.genml[i].f;
        t = G[0].B.genml[i].t;
        p = G[0].B.genml[i].p;
        cm = (G[0].B.genml[i].c=='#' ? 1:0);
        if(cm && CDEPTH>1) { FF=1; break; }
        if(ST==1 && !(Q.f==f && Q.t==t && Q.p==p)) continue;
        if(ST==2 && (Q.f==f && Q.t==t && Q.p==p)) continue;
        G[0].dma = 0;
        int cw = 0;
        if(CDEPTH==1)
            {
            if(cm)
                {
                writeMateVariant(); cw = 1;
                }
            }
        else
            {
            G[0].B.mkmove(i);
            G[1].dmi = PLYS-1;
            }

        if( cw || (CDEPTH>1 && (!can_escape(1))))
            {
            if((CDEPTH>1) && ((G[1].dmi<PLYS-1) || G[0].dma<PLYS-1))
                {
                FF=1;   // if can checkmate faster
                Hc = Hc_o;
                }

                // print result found
            if(0)
             for(;Hc_o!=Hc; Hc_o++)
                {
                for(int j=0; j<PLYS; j++) printf("%s ",H[Hc_o].m[j].mv);
                printf("\n");
                }


            Hc_o = Hc;
            if(Hc)
                {
                if(ST<=8 || (Q.f!=f || Q.t!=t || Q.p!=p)) sk++;

                if(ST>8) { Q.f = f; Q.t = t; Q.p = p; }

                if(ST>8 && sk>1) break;
                }
            }
        if(CDEPTH>1)
            {
            G[0].B.unmkmove();
            }
        }
    if((ST>8 || ST==1) && sk!=1) FF=1;
    if(ST==2 && sk!=0) FF=1;
}

int _AbsDiff(int x,int y) { return (x>y ? x-y : y-x); }

static unsigned int r_x = 30903, r_y = 30903, r_z = 30903, r_w = 30903, r_carry = 0;
	unsigned int rand32() {   unsigned int t;
	   r_x = r_x * 69069 + RANDOMIZER;   r_y ^= r_y << 13;
	   r_y ^= r_y >> 17;   r_y ^= r_y << 5;   t = (r_w << 1) + r_z + r_carry;
	   r_carry = ((r_z >> 2) + (r_w >> 3) + (r_carry >> 2)) >> 30;
	   r_z = r_w;   r_w = t;   return r_x + r_y + r_w;
}

void _dispbo( int p )
{
char bs[200];
char fname[100];
if(p==1)
    {
    sprintf(fname,"M%d_generated.pgn",CDEPTH);
    fp = fopen(fname, "w+");
    printf("Writing to file %s\n",fname);
    }
if(p==-1) fclose(fp);
if(p==0)
    {
    B.sPrintBoard( bs );
	fprintf(fp,"{\n%s\n}\n", bs );
    }
}

void _dispFen()
{
char fen[200];
B.mc = 1;
B.getFEN( fen );
fprintf(fp,"\n[FEN \"%s\"]\n\n", fen);
//printf("\n\n%s\n\n", fen);
}

void _dispH()
{
    int i,j,k,u,r,t,g,g0;
    if(1)
     for(i=0 ; i<Hc-1; i++)
      for(j=i+1 ;j<Hc; j++)
        {
        for(r=1,k=0; r && k<PLYS; k+=2)
            if(!( H[i].m[k].f == H[j].m[k].f &&
                    H[i].m[k].t == H[j].m[k].t &&
                    H[i].m[k].p == H[j].m[k].p)) r=0;

        if(r)
            {           // remove j-variant
            for(u=j+1; u<Hc; u++)
             for(k=0; k<PLYS; k++)
                {
                H[u-1].m[k].f = H[u].m[k].f;
                H[u-1].m[k].t = H[u].m[k].t;
                H[u-1].m[k].p = H[u].m[k].p;
                strcpy_slow(H[u-1].m[k].mv,H[u].m[k].mv);
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
        if( H[i].m[t].f == H[j].m[t].f &&
            H[i].m[t].t == H[j].m[t].t &&
            H[i].m[t].p == H[j].m[t].p )
            {           // remove j-variant
            for(u=j+1; u<Hc; u++)
             for(k=0; k<PLYS; k++)
                {
                H[u-1].m[k].f = H[u].m[k].f;
                H[u-1].m[k].t = H[u].m[k].t;
                H[u-1].m[k].p = H[u].m[k].p;
                strcpy_slow(H[u-1].m[k].mv,H[u].m[k].mv);
                }
            Hc--;
            j--;
            }
        }
     t=( t==1 ? 4 : (t<4 ? t-1 : t+1));
    }

    if(Hc>0)            // print result found to file
        {
            // set limit
            if(Hc>HcMAX) Hc = HcMAX;

            for(g0=0,j=0; j<2 && j<PLYS; j++)
                {
                if(j>0) fprintf(fp," ");
                if(j==0 || wm != (j&1)) fprintf(fp,"%d.", ++g0);
                if(j==0 && !wm) fprintf(fp,"..");
                fprintf(fp,"%s", H[0].m[j].mv);
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
                        fprintf(fp,"%s", H[i].m[j].mv);
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
                fprintf(fp,"%s", H[0].m[j].mv);
                }

            fprintf(fp,"\n\n");
        }
    else fprintf(fp,"{ERROR Hc}\n\n");
}

int Cnt;
int _pieMas[200];
int _pieMc;
const int _pieP[] = {0,1,1,2,2,2,8}; //p...q
int V[13];  // count of pieces by type
int Bi[2];  // two similar bishops

void prepMas()
{
    int i,j,k;
    _pieMc = 0;
    for(i=2; i<7; i++)
    {
     V[i] = _pieP[i]; V[i+6] = _pieP[i];
     for(j=0; j<_pieP[i]; j++)
      for(k=0; k<TIMES_MORE_PIECES; k++)
            {
            if(k==0 || (B.sm>0)) _pieMas[_pieMc++] = i;
            if(k==0 || (B.sm<0)) _pieMas[_pieMc++] = i+6;
            }
    }
}

void CM_calculate()
{
    int r, u, f, c, wk, bk, p=0, px=0,pv=0, o, opmove, i, q, cz, w, t, m;
    int kk, kx, kv, kd=0;
    char pc;

    PLYS =(((CDEPTH-1)*2)+1);

    _dispbo(1);
    printf("Searching...\n");

    for(Cnt=0;Cnt<PGN_COUNT;)
    {
    for(f=0;!f;)
    {
        B.setFEN( (char *)"/////// w - - 1 0" );
        B.sm = ( rand32()&1 ? 1 : -1);

        prepMas();
        for(r=0;;r++)
        {
            wk = rand32() & 63;
            bk = rand32() & 63;
            int wx = wk&7, wy=wk>>3, bx=bk&7, by=bk>>3;
            if( _AbsDiff(wx,bx)>1 || _AbsDiff(wy,by)>1 ) break;
            if(r>300)
                {
                fprintf(fp,"RAND ERROR!\n");
                break;
                }
        }

        B.B[wk] = 'K'; //B.Pwk
        B.B[bk] = 'k'; //B.Pbk
        kk=(B.sm>0 ? bk : wk); kx = kk&7; kv = kk>>3;


        r=0;    // repeats unsuccess
        for(c=2; !f && c<PcMAX && r<200;r++)
        {
          if(c<4) { p = rand32() & 63; px = p&7; pv = p>>3; }
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
                p = rand32() & 63; px = p&7; pv = p>>3;
                break;
                }
            }
          }

          o = rand32() % _pieMc;
          q = _pieMas[o];       // put this piece on random square

            if(q==4 || q==10)   // avoid same bishops
            {
             t=(B.sm>0 ? 0 : 1);
             if(V[q]==2) Bi[t] = (px+pv)&1;
             else if(V[q]==1) if(Bi[t]==((px+pv)&1))
             {
                 p++; if(p==64) p=1;
                 px = p&7; pv = p>>3;
             }
            }

          if((q!=6 && q!=12 ) || (pv!=0 && pv!=7 && pv!=(q==6?6:1)))
          {
          opmove = -B.sm;
          if(!B.B[p])
          {
            if(V[q]==0) continue;

            if(q==6 || q==12)
            {
             for(m=0,w=1;w<7;w++)
             {
                 t = (w<<3)+px;
                 pc = B.B[t];
                 if(pc==c1_chess_S[q]) m++;
                 if(q==6 && pc=='p' && p>t) { m=2; break; }
                 if(q==12 && pc=='P' && p<t) { m=2; break; }
             }
             if(m>1) continue;
            }

            B.B[p]= c1_chess_S[q];

            cz = B.isCheck();
            if(!cz)
                {
                B.sm = -B.sm;
                cz = B.isCheck();
                B.sm = -B.sm;
                }
            if(cz) { B.B[p] = 0; }
            else
            {
             if(0 && c<5)
                {
                 f=0;
                }
             else
                {
                FF=0;
                Q.f = -1; Q.t = -1; Q.p = -1;
                MM(9);
                f = 1-FF;
                }

             if(f)      // Optimize, remove redundant pieces
             {
                m = 0;
                for(i=0;i<64;i++)
                {
                pc = B.B[i];
                if(pc && pc!='K' && pc!='k')
                    {
                    B.B[i] = 0;
                    FF = 0; MM(1);
                    if(!FF)
                        {
                        MM(2);
                        if(!FF) m=1;
                        }
                    if(FF) B.B[i]=pc;
                    }
                }

             }

            // Verify once again after
            if(f)
                {
                FF=0;
                Q.f = -1; Q.t = -1; Q.p = -1;
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
            if(B.B[p]!=0) V[q]--;
          }
          }
        }

    }

    }
    _dispbo(-1);
}

FILE *cfg;
char buff[1000];
static char *fnCONFIG = (char *)"CM_generator.ini";
static char *cfgtxt[] = {
 (char *)";",
 (char *)"; Configuration file for simple chess checkmate generator.",
 (char *)"; Absolute freeware by Chessforeva, 2016",
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
    int i,a,val;
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
            for(i=0,a=-1;;i++)
            {
                char c=buff[i];
                if(c==0 || c==';') break;
                if(c=='=') a=i;
            }
            if(a>=0)
                {
                 val=B.toUInt(&buff[a+1]);  // to integer

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
    printf("Simple chess checkmate generator\nConfiguration file %s\n", fnCONFIG);
    ReadConfig();
    CM_calculate();
    printf("Finished\n");
    return 0;
}
