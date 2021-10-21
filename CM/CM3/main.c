#include <stdio.h>
#include <memory.h>

#include "buzz_movegen.h"


// A better simple chess checkmate generator,
// uses BuzzChess magic bitboards by Pradu Kannan (thanks)
// Chessforeva, 2021
//    (GCC compiler v.6.3.0 on Win32 - ok)



// M1=1 (fast), M2=2, M3=3, M4=4, M5=5 (got some)
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

// chess board to process
char T[64];     // pieces as (A8) "rnb....BoNR" (H1)
unsigned int sm;  // side to move 0 (white), or 1 (black);
board Bo;       // board object
char fen_str[1024];         // contains current FEN

FILE *fp;

int FF, ST, Hc_o, wm;

struct CDP {		/* structure calc-in-depth */
        board Bo;
        moveList ml;
        char mv[50];
        int i;
        int dmi;
        int dma;
	}G[200 /*PLYS*/];

typedef struct _MO
    {
        unsigned int f;
        unsigned int t;
        unsigned int p;
        char mv[12];
    }MO;

struct Hs
    {
     MO m[200 /*PLYS*/];
    }H[9000 /*result cases found*/] ;
int Hc;

MO Qz;

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
    move m = G[k].ml.moves[G[k].i].m;
    unsigned int from=extractFrom(m), to=extractTo(m), promotion=extractPromotion(m);
    MO *mo = &H[Hc].m[k];

    mo->f = from;
    mo->t = to;
    mo->p = promotion;
    }
    Hc++;
}

void rmvHs( int q )
{
    for( ; Hc>0;  Hc--)
        {
         for(int k=q; k>=0; k--)
            {
            move m = G[k].ml.moves[G[k].i].m;
            unsigned int from=extractFrom(m), to=extractTo(m), promotion=extractPromotion(m);
            MO *mo = &H[Hc-1].m[k];

            if(!( mo->f == from &&
                    mo->t == to &&
                    mo->p == promotion )) return;
            }
        };
}

int can_escape( int d );    // definition will be after

// returns 1 if checkmate in next moves
int can_matenext( int d )
{
    int i,m,was;
    int LastPly = ((d==PLYS-1)?1:0);
    board *b = &G[d+1].Bo, *g = &G[d].Bo;
    moveList *ml=&G[d].ml, *ml2=&G[d+1].ml;

    genMoves(g,ml);

    // checkmates only
    for (was=0, i=0; i<ml->moveCount; i++)
        {
        G[d].i = i;
        copyboardTo(b,g);
        makemove(b,ml->moves[i].m);
        if(inCheck(*b,b->side)) {
            genMoves(b,ml2);
            if(ml2->moveCount==0) {   // checkmate
                was=1;
                if(G[d-1].dmi>d) G[d-1].dmi=d;
                if(LastPly) writeMateVariant();
                }
            }
        }
    if(was) return 1;

    int ret=0;

    // other cases
    for (i=0; i<ml->moveCount; i++)
        {
        G[d].i = i;
        if(d<PLYS-1)
            {
            copyboardTo(b,g);
            makemove(b,ml->moves[i].m);
            G[d].dma = 0;
            m = can_escape(d+1);
            if(G[d].dma<G[d-1].dmi) G[d-1].dmi = G[d].dma;

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
    board *b = &G[d+1].Bo, *g = &G[d].Bo;
    moveList *ml=&G[d].ml;

    genMoves(g,ml);

    if(!ml->moveCount) { rmvHs(d-1); return 1; }        // stalemate
    for (i=0; i<ml->moveCount; i++)
        {
        G[d].i = i;
        copyboardTo(b,g);
        makemove(b,ml->moves[i].m);
        G[d].dmi = PLYS;
        m = can_matenext(d+1);
        if(G[d-1].dma==0 || G[d-1].dma > G[d].dmi) G[d-1].dma = G[d].dmi;
        if(!m) { rmvHs(d-1); return 1; }
        }
    return 0;
}

void MM( int st )
{
    int i, sk, f, t, p, cm;
    board *b = &G[0].Bo, *u = &G[1].Bo;

    ST = st;
    wm = 1-sm;

    copyboardTo(b, &Bo);
    genMoves(b,&G[0].ml);

    if(ST>8 || ST==1 || ST==2)
    {
        if(inCheck(*b,b->side) || inCheck(*b,b->xside))
            { FF=1; return; }
    }

    Hc = 0; Hc_o = Hc;
    sk=0;
    for (i = 0;(!FF) &&  i<G[0].ml.moveCount; i++)
        {
        G[0].i = i;

        copyboardTo(b, &Bo);

        move m = G[0].ml.moves[i].m;
       	f=extractFrom(m);
       	t=extractTo(m);
       	p=extractPromotion(m);

        moveToString(b,m,G[0].mv);
        cm = (m2strSgn == '#');

        if(cm && CDEPTH>1) { FF=1; break; }
        if(ST==1 && !(Qz.f==f && Qz.t==t && Qz.p==p)) continue;
        if(ST==2 && (Qz.f==f && Qz.t==t && Qz.p==p)) continue;
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
            copyboardTo(u,b);
            makemove(u,m);
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
                if(ST<=8 || (Qz.f!=f || Qz.t!=t || Qz.p!=p)) sk++;

                if(ST>8) { Qz.f = f; Qz.t = t; Qz.p = p; }

                if(ST>8 && sk>1) break;
                }
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
    dispboard_s( &Bo, bs );
	fprintf(fp,"{\n%s\n}\n", bs );
    }
}

void _dispFen()
{
fprintf(fp,"\n[FEN \"%s\"]\n\n", fen_str);
}

void _dispH()
{
    int i,j,k,u,r,t,g,g0,w;
    MO *mo, *mo2;
    board *b,*b2;
    moveList *ml;

    if(1)
     for(i=0 ; i<Hc-1; i++)
      for(j=i+1 ;j<Hc; j++)
        {
        for(r=1,k=0; r && k<PLYS; k+=2) {
            mo = &H[i].m[k];
            mo2 = &H[j].m[k];
            if(!( mo->f == mo2->f && mo->t == mo2->t && mo->p == mo2->p)) r=0;
            }
        if(r)
            {           // remove j-variant
            for(u=j+1; u<Hc; u++)
             for(k=0; k<PLYS; k++)
                {
                mo = &H[u-1].m[k];
                mo2 = &H[u].m[k];

                mo->f = mo2->f;
                mo->t = mo2->t;
                mo->p = mo2->p;
                }
            Hc--;
            j--;
            }
        }

    for(t=3; t==3 || (t<PLYS && Hc>HcMAX);)        // remove some more
    {
    for(i=0 ;i<Hc-1; i++) {
     for(j=i+1 ;j<Hc; j++)
        {
        mo = &H[i].m[t];
        mo2 = &H[j].m[t];

        if(mo->f == mo2->f && mo->t == mo2->t && mo->p == mo2->p)
            {           // remove j-variant
            for(u=j+1; u<Hc; u++)
             for(k=0; k<PLYS; k++)
                {
                mo = &H[u-1].m[k];
                mo2 = &H[u].m[k];

                mo->f = mo2->f;
                mo->t = mo2->t;
                mo->p = mo2->p;
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
            copyboardTo(&G[0].Bo, &Bo);
			for(j=0; j<PLYS; j++)
				{
                mo = &H[i].m[j];
                b = &G[j].Bo;
                ml = &G[j].ml;
                genMoves(b,ml);
                for (w=0; w<ml->moveCount; w++)
                    {
                    G[j].i = w;

                    move m = G[j].ml.moves[w].m;
                    unsigned int from=extractFrom(m), to=extractTo(m), promotion=extractPromotion(m);

                    if(mo->f==from && mo->t==to && mo->p==promotion) {
                        moveToString(b,m,mo->mv);      // prepare movestring
                        b2 = &G[j+1].Bo;
                        copyboardTo(b2,b);
                        makemove(b2,m);
                        break;
                    }
                    }
				}
			}

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
static char *c1_chess_S = (char *)" KQRBNPkqrbnp\0";

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
            if(k==0 || (!sm)) _pieMas[_pieMc++] = i;
            if(k==0 || (sm)) _pieMas[_pieMc++] = i+6;
            }

    }
}


void prepareBo() {

    char *s = fen_str;
    char pc;
    int v,h,e=0,i=0;
    for(v=7;v>=0;v--) {
        for(h=0;h<8;h++) {
            pc = T[ (v<<3)+h ];
            if(!pc) e++;
            else {
                if(e) { s[i++]=('0'+e); e=0; }
                s[i++]=pc;
            }
        }
        if(e) { s[i++]=('0'+e); e=0; }
        if(v) s[i++]='/';
    }

    sprintf( &s[i]," %c - - 0 1", (sm?'b':'w')) ;
    setboard(&Bo,fen_str);
}

void CM_calculate()
{
    int r, u, f, c, wk, bk, p=0, px=0,pv=0, o, i, q, w, t, m;
    int kk, kx, kv, kd=0;
    char pc;
    board *b = &Bo;

    PLYS =(((CDEPTH-1)*2)+1);

    printf("Searching...\n");

    for(Cnt=0;Cnt<PGN_COUNT;)
    {
    for(f=0;!f;)
    {
        memset( T, 0, 64);
        sm = ( rand32()&1 );
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

        T[wk] = 'K';
        T[bk] = 'k';
        kk=(!sm ? bk : wk); kx = kk&7; kv = kk>>3;


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
             t=(sm>0 ? 0 : 1);
             if(V[q]==2) Bi[t] = (px+pv)&1;
             else if(V[q]==1) if(Bi[t]==((px+pv)&1))
             {
                 p++; if(p==64) p=1;
                 px = p&7; pv = p>>3;
             }
            }

          if((q!=6 && q!=12 ) || (pv!=0 && pv!=7 && pv!=(q==6?6:1)))
          {
          if(!T[p])
          {
            if(V[q]==0) continue;

            if(q==6 || q==12)
            {
             for(m=0,w=1;w<7;w++)
             {
                 t = (w<<3)+px;
                 pc = T[t];
                 if(pc==c1_chess_S[q]) m++;
                 if(q==6 && pc=='p' && p>t) { m=2; break; }
                 if(q==12 && pc=='P' && p<t) { m=2; break; }
             }
             if(m>1) continue;
            }

            T[p]= c1_chess_S[q];

            prepareBo();

          if(inCheck(*b,b->side)||inCheck(*b,b->xside)) {
                T[p] = 0;
                }
            else
            {
             if(0 && c<5)
                {
                 f=0;
                }
             else
                {
                FF=0;
                Qz.f = -1; Qz.t = -1; Qz.p = -1;
                MM(9);
                f = 1-FF;
                }

             if(f)      // Optimize, remove redundant pieces
             {
                m = 0;
                for(i=0;i<64;i++)
                {
                pc = T[i];
                if(pc && pc!='K' && pc!='k')
                    {
                    T[i] = 0;

                    FF = 0;
                    prepareBo();
                    MM(1);
                    if(!FF)
                        {
                        MM(2);
                        if(!FF) m=1;
                        }
                    if(FF) {
                            T[i]=pc;
                            }
                    }
                }

             }

            // Verify once again after
            if(f)
                {
                FF=0;
                Qz.f = -1; Qz.t = -1; Qz.p = -1;
                prepareBo();
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
            if(T[p]!=0) V[q]--;
          }
          }
        }

    }

    }

}

FILE *cfg;
char buff[1000];
static char *fnCONFIG = (char *)"CM_generator.ini";
static char *cfgtxt[] = {
 (char *)";",
 (char *)"; Configuration file for simple chess checkmate generator",
 (char *)"; (fast magic-bitboards movegen by Pradu Kannan, 2007)",
 (char *)"; Absolute freeware by Chessforeva, 2021",
 (char *)";",
 (char *)"; Fast for M1(fast),M2,M3,M4...(got same),M5,..",
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
                 sscanf(&buff[a+1],"%d",&val);  // to integer

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
    initMoveGen();
    startingpos(&Bo);

    printf("Simple chess checkmate generator\nConfiguration file %s\n", fnCONFIG);
    ReadConfig();

    _dispbo(1);
    CM_calculate();
    _dispbo(-1);

    printf("Finished\n");
    return 0;
}
