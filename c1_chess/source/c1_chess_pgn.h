// CHESS PGN class - to work with pgn-notated chess games
//  (analyses variants in comments, reads from .pgn file, outputs to html-file, boards to txt file)
//
// Designed on MS Studio C++
//
// Purpose:
//		1. Create a browsable web-page from pgn-file (optional sorting by rounds)
//		2. Create a txt-file of all boards and moves for chess games from pgn-file
//			(to see all information for positions processing) 
//
#ifndef _INC_C1_PGN_CHESS
#define _INC_C1_PGN_CHESS

#include "c1_chess.h"		// uses c1_chess class for chess logic

#include <stdio.h>
#include <string.h>
#include <malloc.h>

static char *c1_pgn_evPtr = "[Event \0",
	*c1_pgn_hdr_event = "Event \0", *c1_pgn_hdr_date = "Date \0",
	*c1_pgn_hdr_white = "White \0", *c1_pgn_hdr_black = "Black \0",
	*c1_pgn_hdr_round = "Round \0", *c1_pgn_hdr_site = "Site \0",
	*c1_pgn_hdr_result = "Result \0", *c1_pgn_hdr_FEN = "FEN \0";
static char *c1_pgn_p1s = "\\ ,.+=-/\0", *c1_pgn_ctwrd = " .,;)(}{][\0", *c1_pgn_Z0 = "Z0\0",
	*c1_pgn_p2stm = "  1:0 1-0 0:1 0-1 1/2-1/2 1/2:1/2 1/2 \0",
	*c1_pgn_mtry = "O0NBRQKabcdefgh\0", *c1_pgn_mtrpc = "NBRQK\0",
	*c1_pgn_mtr0 = "+#!?\0", *c1_pgn_mtr1 = "qrbn\0", *c1_pgn_mtr2 = "Xx-\0", *c1_pgn_ep = " (ep)",
	*c1_elim_br = "<br>\0", *c1_elim_nspc = "&nbsp;\0",
	*c1_elim_div0 = "<div\0", *c1_elim_div1 = "</div>\0"; 

static char *c1_domain_c = "https://chessforeva.gitlab.io/\0",
	*c1_scrBegin = "<script type=\"text/javascript\" \0", *c1_scrEnd = "</script>",
	*c1_htmcss0 = "<link rel=\"StyleSheet\" href=\"\0", *c1_htmcss1 = "c1_chess.css\" type=\"text/css\">\0";
char c1_domain[1024];

	/* Configuration values, assigned on object creation */
long c1_pgn_buffer_games_cnt = 200;
		/* maximum games to parse and sort for this object (about 1 megaevent to sort by rounds),
		200 is just fast without sorting */
long c1_pgn_filereader_buffer_size = 64 * 1024;	// 64Kb per-one-reading from file
long c1_pgn_avrg_positions_per_game = 200;		// average count of positions per game (+variants)
long c1_pgn_avrg_header_buffer_size = 1 * 1024;	// 1Kb average buffer for header [Event,White,Black,...] data
long c1_pgn_avrg_pgn_buffer_size = 5 * 1024;	// 5Kb average buffer for pgn data 1.e4..., also comments
long c1_pgn_file_size = 2 * 1024 * 1024;
		/* 2Mb is buffer in memory to compose the html result, txt file is not limited */
long c1_pgn_variants_cnt = 20;					// maximum variants in depth


//--------Numeric Annotation Glyphs. Standard NAGs only
static const char *c1_pgn_ngWrds[]={
  "has", "Black", "White", "advantage", "the", "played", "control", "well",
   "placement", "very", "poorly", "moderate", "decisive", "slight", "good", "poor",
    "pawn", "time", "ending", "middlegame", "opening", "structure", "king", "move",
     "counterplay", "protected", "queenside", "kingside", "center", "deficit", "material",
      "for", "compensation", "(development)", "space", "Bishops", "pressure", "coordination",
       "piece", "queen", "rook", "bishop", "knight", "strong", "moderately", "weak", "placed",
        "rank", "first", "initiative", "position", "pawns", "color", "pair", "severe", "a5)",
         "(DR:x", "vulnerable", "(<<)", "(>>)", "(#)", "adequate", "than", "more", "(=/&)",
          "sufficient", "insufficient", "(->)", "attack", "lasting", "(^)", "(@)", "(O)",
		    "(zz)", "zugzwang", "resign)", "should", "crushing", "chances,", "equal"};

static const char *c1_pgn__ng[] = { "", "!", "?", "!!", "??", "speculative %17 (\"!?\")",
	"questionable %17 (\"?!\")","forced %17 (all others lose quickly)",
	"singular %17 (no reasonable alternatives)", "worst %17","drawish %32", "%4F %4E quiet %32 (=)",
	"%4F %4E active %32 (ECO ->/<-)", "unclear %32 (emerging &)","%2 %0 a %D %3 (+=)",
	"%1 %0 a %D %3 (=+)", "%2 %0 a %B %3 (+/-)", "%1 %0 a %B %3 (-/+)","%2 %0 a %C %3 (+-)",
	"%1 %0 a %C %3 (-+)", "%2 %0 a %4D %3 (%1 %4C %4B (+--)","%1 %0 a %4D %3 (%2 %4C %4B (--+)",
	"%2 is in %4A %49", "%1 is in %4A %49", "%2 %0 a %D %22 %3","%1 %0 a %D %22 %3",
	"%2 %0 a %B %22 %3 %48", "%1 %0 a %B %22 %3 %48", "%2 %0 a %C %22 %3","%1 %0 a %C %22 %3",
	"%2 %0 a %D %11 %21 %3", "%1 %0 a %D %11 %21 %3", "%2 %0 a %B %11 %21 %3 %47",
	"%1 %0 a %B %11 %21 %3 %47", "%2 %0 a %C %11 %21 %3", "%1 %0 a %C %11 %21 %3", "%2 %0 %4 %31 %46",
	"%1 %0 %4 %31 %46", "%2 %0 a %45 %31", "%1 %0 a %45 %31", "%2 %0 %4 %44 %43", "%1 %0 %4 %44 %43",
	"%2 %0 in%41 %20 %1F %1E %1D",
	"%1 %0 in%41 %20 %1F %1E %1D", "%2 %0 %41 %20 %1F %1E %1D %40","%1 %0 %41 %20 %1F %1E %1D %40",
	"%2 %0 %3F %3E %3D %20 %1F %1E %1D", "%1 %0 %3F %3E %3D %20 %1F %1E %1D","%2 %0 a %D %1C %6 %3",
	"%1 %0 a %D %1C %6 %3", "%2 %0 a %B %1C %6 %3 %3C", "%1 %0 a %B %1C %6 %3 %3C","%2 %0 a %C %1C %6 %3",
	"%1 %0 a %C %1C %6 %3", "%2 %0 a %D %1B %6 %3", "%1 %0 a %D %1B %6 %3","%2 %0 a %B %1B %6 %3 %3B",
	"%1 %0 a %B %1B %6 %3 %3B", "%2 %0 a %C %1B %6 %3", "%1 %0 a %C %1B %6 %3",
	"%2 %0 a %D %1A %6 %3", "%1 %0 a %D %1A %6 %3",
	"%2 %0 a %B %1A %6 %3 %3A", "%1 %0 a %B %1A %6 %3 %3A ","%2 %0 a %C %1A %6 %3", "%1 %0 a %C %1A %6 %3",
	"%2 %0 a %39 %30 %2F", "%1 %0 a %39 %30 %2F","%2 %0 a %7 %19 %30 %2F", "%1 %0 a %7 %19 %30 %2F",
	"%2 %0 a %A %19 %16", "%1 %0 a %A %19 %16","%2 %0 a %7 %19 %16", "%1 %0 a %7 %19 %16",
	"%2 %0 a %A %2E %16", "%1 %0 a %A %2E %16","%2 %0 a %7 %2E %16", "%1 %0 a %7 %2E %16",
	"%2 %0 a %9 %2D %10 %15", "%1 %0 a %9 %2D %10 %15", "%2 %0 a %2C %2D %10 %15 %38 %37",
	"%1 %0 a %2C %2D %10 %15 %38 %37",
	"%2 %0 a %2C %2B %10 %15","%1 %0 a %2C %2B %10 %15", "%2 %0 a %9 %2B %10 %15",
	"%1 %0 a %9 %2B %10 %15", "%2 %0 %F %2A %8","%1 %0 %F %2A %8", "%2 %0 %E %2A %8", "%1 %0 %E %2A %8",
	"%2 %0 %F %29 %8", "%1 %0 %F %29 %8","%2 %0 %E %29 %8 (diagonal)", "%1 %0 %E %29 %8",
	"%2 %0 %F %28 %8", "%1 %0 %F %28 %8","%2 %0 %E %28 %8 (%2F <=> file ||)", "%1 %0 %E %28 %8",
	"%2 %0 %F %27 %8", "%1 %0 %F %27 %8", "%2 %0 %E %27 %8", "%1 %0 %E %27 %8", "%2 %0 %F %26 %25",
	"%1 %0 %F %26 %25", "%2 %0 %E %26 %25","%1 %0 %E %26 %25", "%2 %0 %5 %4 %14 %9 %A",
	"%1 %0 %5 %4 %14 %9 %A", "%2 %0 %5 %4 %14 %A","%1 %0 %5 %4 %14 %A", "%2 %0 %5 %4 %14 %7",
	"%1 %0 %5 %4 %14 %7", "%2 %0 %5 %4 %14 %9 %7","%1 %0 %5 %4 %14 %9 %7", "%2 %0 %5 %4 %13 %9 %A",
	"%1 %0 %5 %4 %13 %9 %A", "%2 %0 %5 %4 %13 %A","%1 %0 %5 %4 %13 %A", "%2 %0 %5 %4 %13 %7",
	"%1 %0 %5 %4 %13 %7", "%2 %0 %5 %4 %13 %9 %7","%1 %0 %5 %4 %13 %9 %7",
	"%2 %0 %5 %4 %12 %9 %A", "%1 %0 %5 %4 %12 %9 %A", "%2 %0 %5 %4 %12 %A","%1 %0 %5 %4 %12 %A",
	"%2 %0 %5 %4 %12 %7", "%1 %0 %5 %4 %12 %7", "%2 %0 %5 %4 %12 %9 %7","%1 %0 %5 %4 %12 %9 %7",
	"%2 %0 %D %18", "%1 %0 %D %18", "%2 %0 %B %18 (->/<-)", "%1 %0 %B %18","%2 %0 %C %18",	"%1 %0 %C %18",
	"%2 %0 %B %11 %6 %24", "%1 %0 %B %11 %6 %24", "%2 %0 %36 %11 %6 %24","%1 %0 %36 %11 %6 %24",
	"With %4 idea", "Aimed against", "Better %17", "Worse %17",
	"Equivalent %17","Editors Remark (\"RR\")", "Novelty (\"N\")", "Weak point", "Endgame", "Line",
	"Diagonal","%2 %0 a %35 of %23", "%1 %0 a %35 of %23", "%23 of opposite %34", "%23 of same %34",
	"","","","","","","","","","","","","","","","","","","","","","","","","",
	"","","","","","","","","","","Etc.", "Doubled %33", "Isolated %10",
	"Connected %33", "Hanging %33", "Backwards %10","","","","", "","Diagram (\"D\", \"#\")" };

	// helping function
int c1_getHx(char c) { return ( (c>='0' && c<='9') ? c-'0': ((c>='A' && c<='F') ? c-'A'+10 : -1 )); };

void c1_getNAGdescr(int nag, char *outp)		// puts description for nag to buffer
	{
		char *p, *w, c;
		int i=0, j=0, k, k2;
		if(nag>=0 && nag<=201)
			{
			p = (char *)c1_pgn__ng[nag];
			for(;;i++)
				{
				c=p[i];
				if(c=='%')		// decrypt whole word
					{
					c=p[++i]; k = c1_getHx(c);
					c=p[++i]; k2 = c1_getHx(c);
					if(k2>=0) k=(k<<4) + k2;
					else i--;
					w = (char *)c1_pgn_ngWrds[k];
					for(;;)
						{
						c=*(w++);
						if(c==0) break;
						outp[j++]=c;
						}
					}
				else
					{
					if(c==0) break;
					outp[j++]=c;
					}
				}
			}
		outp[j]=0;
	}

static const int c1_cnt_em = 18;
static char *c1_em[] = { "&nbsp;&nbsp;&nbsp;&nbsp;", "<br>", "0-0-0","1/2-1/2",")\">",
  "\"_pgn","_pOc(","</div>","onclick=","<div class=","\" id=\"_pI",
  " width=\"", "height=\"", "style=\"float:", "<canvas id=\"_pgn_D_", "></canvas>",
  "'", "@" };
static const int c1_ln_em[] = { 24, 4, 5, 7, 3, 5, 5, 6, 8, 11, 9, 8, 8, 13, 19, 10, 1, 1 };

void c1_encodedata( char *data, char *wrkbuf ) {     /* encode compressed html */
	char c=1,*p,*w=wrkbuf;
	for(p=data;c!=0;p++)
		{
		c = (*p);
		for(int i=c1_cnt_em-1; i>=0; i--)
			{
			if( strncmp( p, c1_em[i], c1_ln_em[i] )==0 )
				{
				*(w++)='@';
				c = (i<10 ? '0' : 'a'-10) + i;
				p += (c1_ln_em[i]-1);
				break;
				}
			}
		*(w++)=c;
		}
	};

void c1_elimspaces( char *data ) {	/*remove spaces*/
	char c=1,*p,*w, *y, *z;
	int j,l,d,f;
	for(p=data;c!=0;p++)
		{
		c = (*p);
		if( strncmp( p, c1_elim_br, 4 )==0 )
			{
				y = &p[4];
				for(w=y; c!=0;)
					{
						c = (*w);
						if(c==' ') w++;
						else if( strncmp( w, c1_elim_nspc, 6 )==0 ) w+=6;
						else if( strncmp( w, c1_elim_br, 4 )==0 )
							{
							for(w+=4;c!=0;*(y++)=c) c = *(w++);
							break;
							}
						else break;
					}
			}

		for(j=0;j<2;j++)
			{
			z = (j==0 ? c1_elim_div0 : c1_elim_div1 );
			l = strlen(z);
			if( strncmp( p, z, l )==0 )
				{
				d = (j==0 ? 19 : 6);
				y = &p[d];
				for(c=1,w=y,f=0; c!=0;)
					{
						c = (*w);
						if(c==' ') { w++; f=1; }
						else if( strncmp( w, c1_elim_nspc, 6 )==0 ) { w+=6; f=1; }
						else if( strncmp( w, c1_elim_br, 4 )==0 ) { w+=4; f=1; }
						else if( strncmp( w, z, l )==0 )
							{
							if(f>0)
								for(;c!=0;*(y++)=c) c = *(w++);
							break;
							}
						else break;
					}
				}
			}
		}
	};

class c1_pgn {			// ------------------start of c1_pgn class

	public:
		int ns;				// starting move number
		char *startFEN;		// pointer to starting FEN

		struct Ttree {		/* structure - tree of moves in pgn data */
		int k;			// which move from the generated by k
		char uci[6];	// uci string of move, fxmp. "a2a1q\0"
		int N;			// current move number
		int sm;			// which side to move
		long pstruI;	// pointer to pstru

		// pointers of Ttree chain elements
		long T_sub;		// pointer to first element of sub-trees - next moves after this) (or 0)
		long T_next;	// pointer to next element of variation move in the same position (or 0)
		long T_pre;		// pointer to pre-element (opposite T_next)
		long T_up;		// pointer to parent (opposite T_sub)

		};	// pgn-tree for game

		Ttree *Tr;		// the main buffer for games
		long tc;		// usage of buffer
		char *Hb;		// the main buffer for headers and pgn
		long hc;		// usage of buffer
		char *Pb;		// the main buffer for pgns
		long hpc;		// usage of buffer

		struct Gstru {	/* structure of game data */
			long Ti;	// pointer to tree data (moves,variants)
			char *H;	// pointer to header data
			int rl;		// length of all header data
			char *pgn;	// pointer to pgn
			int sl;		// length of pgn data
			long idkN;	// object ID number for html
			int srnd;	// round data
			bool srtd;	// sorted flag
		};

		Gstru *G;		// Games list
		long gc;		// count of games
		long *Sort;		// array of sorted indexes
		bool fsort;		// can sort or not
			
		char *reader_buf;	// buffer for reading files

		int rf;			// result flags for ParsePgn function
						// 1 = to browsable html file
						// &2 = boards to text file (3-long notation only)

		bool ucdsymb;	// use unicode symbols (longer)

						// buffers where to compose
		char *Bhtm, *Bht2, *Bht3, *Btbl, *Benc;

		char *be;		// current end-pointer for this buffer
		char *ptbe;		// where to write _pgn_Pt datas
		char *pobe;		// ..
		char *tbbe;		// for board outputs

		int dist;		// distance for board table in html
		int canvCnt;	// count of canvas elements already
		int canvSize;	// size of canvas element
		bool tabs;		// should add tabs or not
		bool tooLarge;	// if too large result
		long bLength;	// length of html-result

		struct Vstru {	/* structure of variant data */
			char c;		// '{' or '('
			int i;		// position
			long Tv;	// current tree position
			int N;		// current move number
			int sm;		// which side to move
			int f;		// numb-flag 
		};

		Vstru *Vr;		// list of current variants being processed
		int vc;			// count of current variants

		struct Pstru {		/* structure of points for mouse clicks */
			long idk;		// object ID number for html
			char uci[6];	// uci string of move, fxmp. "a2a1q\0"
			char FEN[100];	// fen string
			int curmv;		// moves cntr.
			int v,vf;		// kinda flags for autoplay mode
		};

		Pstru *Pt;		// points to enter in browsing
		int pc;			// count of current point
		long pc_total;	// total for file

		int rnDk;		// random number simply avoid html-bugs

		/*Some tree functions*/
	long AddEmptySubTreeArray( long addto ) {
		Ttree *t = &Tr[tc];
		(*t).k = (*t).N = (*t).sm = 0;
		(*t).uci[0] = 0;
		(*t).T_sub = (*t).T_next = (*t).T_pre = NULL;
		(*t).pstruI = 0;
		(*t).T_up = addto;
		if(addto!=0) Tr[addto].T_sub = tc;
		return (tc++);
	};

	long GoLastElementTreeArray( long curr ) {
		long i = curr, n;
		for(;i!=0;)
			{
			n = Tr[i].T_next;
			Ttree t = Tr[i];
			if(n==0) break;
			i = n;
			}
		return i;
	};

	bool IsEmptyTreeArray( long curr ) {
		return (Tr[curr].T_next == Tr[curr].T_pre);	//==0
	};


	long AddElementToTreeArray( long addto ) {
		long i = GoLastElementTreeArray(addto);
		Tr[i].T_next = tc;
		Ttree *t = &Tr[tc];
		(*t).T_sub = (*t).T_up = (*t).T_next = 0;
		(*t).T_pre = i;
		return (tc++);
	};

	/* for single game very simple usage */
	void fromhist( c1_chess B ) {      /* generates tree from history */

	int i=0,r=0,N=1;
	c1_chess B2;
	B2.setFEN( c1_chess_sFEN );
	long j = AddEmptySubTreeArray(0);
	G[gc].Ti = j;
	Ttree *t = &Tr[j];
    for(int n=0; r>=0 && n<B.mc; n++)
		{
		c1_chess::hlist *h = &B.hist[n];
		B2.moveGen();
		r=-1;
		for(int k=0;r<0 && k<B2.gc;k++)
			{
			c1_chess::glist *o = &B2.genml[k];
			if((*o).f==(*h).m.f && (*o).t==(*h).m.t && ((*h).m.p==0 || (*o).p==(*h).m.p)) r=k;
			}
		if(r>=0)
			{
			j = AddElementToTreeArray(j);
			t = &Tr[j];
			(*t).k = r;
			B2.m2uci( h, (*t).uci );
			(*t).N = N;
			(*t).sm = B2.sm;
			(*t).pstruI = 0;
			B2.mkmove(r);
			if(B2.sm>0) N++;
			}
		}
	gc++; 
    };

	void uciMoves( char *s, int gameNr ) {       /*tree to uci move list */
    
	int k=0,i=0;
	int gNr = (gameNr==NULL ? 0 : gameNr);
	long j = G[gNr].Ti;
	Ttree *t = &Tr[j];
    for(j = (*t).T_next; j!=0;j = (*t).T_next)
		{
		t = &Tr[j];
		if(k>0) s[i++]=' '; k=1;
		strcpy(&s[i], (*t).uci);
		i+=strlen(&s[i]);
		}
    };

	void cutword( char *s, char *wrd ) {	// current word
		char *p = wrd, *q = s, c=' ';
		for(;c!=0;)
			{
			c = *(q++);
			if(strchr( c1_pgn_ctwrd,c)!=NULL) c=0;
			*(p++)=c;
			}
	};

	void setPosAndMove( c1_chess *bo, Ttree To ) {  /* set FEN & move */
		if(To.sm==0)
			{
			(*bo).setFEN( startFEN );
			}
		else
			{
			(*bo).setFEN( Pt[ To.pstruI ].FEN );
			(*bo).mkmove( To.k );
			}
        (*bo).moveGen();
    };

    void someSpaces ( char *s ) {		// this adds tab-spaces

		char *p = s;
		sprintf(p,"<br>"); p+=strlen(p);
		for(int l=0;l<vc;l++)
			{
			sprintf(p,"&nbsp;&nbsp;&nbsp;&nbsp;"); p+=strlen(p);
			}
    };

	/*in chess board*/ 
	bool c_is18( char c ) { return ( c>='1' && c<='8' ); }
	bool c_isAH( char c ) { return ( c>='a' && c<='h' ); }
	char c_toUpper( char c ) { return ( (c<'a') ? c : c-'a'+'A' ); }

	int moveTry(c1_chess B, char *mv) {      /* mv=notated move, tries to move */
    
	char c=mv[0];
	int ln = strlen(mv);
    if(ln<2 || ln>=20) return -1;
    else if(strchr(c1_pgn_mtry,c)==NULL) return -1;

	char s[20],qf[2],qt[2],*z;
    int r=-1,j=0,k=0,f,w,y;
	bool cont;

	c1_chess::glist *o;

    for(;;)
		{
		c=mv[j++];
		if(c=='O') s[k++]='0';
		else if (c=='e' && s[j]=='p') j++;
		else s[k++]=c;
		if(c==0) break;
		}
	ln = strlen(s);

    if(s[0]=='0' && s[1]=='-' && s[2]=='0')
		{
		for(k=0;k<B.gc;k++)
			{
			o = &B.genml[k];
			if(((*o).k==1 && ln==3) || ((*o).k==2 && ln>4)) return k;
			}
		return -1;   
		}

    for(k=0;r<0 && k<B.gc;k++)
        {
		o = &B.genml[k];

		B.i2sq( (*o).f, qf );
		B.i2sq( (*o).t, qt );
                        
        c= c_toUpper( (*o).z );
		f=0;

        if(c=='P')
			{
			if(strchr(c1_pgn_mtrpc,s[0])!=NULL) continue;
			}
		else
			{
			if(c!=s[0]) continue;
			f++;
			}

        for(cont=false,y=0,w=0; w<4 && f<ln; f++)
			{
			c=s[f];
			if(strchr(c1_pgn_mtr0,c)!=NULL) {/*skip*/}
			else if((*o).p!=0 && w>2 && (c=='=' || strchr(c1_pgn_mtr1,c)!=NULL )) w=4;
			else if(strchr(c1_pgn_mtr2,c)!=NULL) { if(w<2) w=2; }          
			else if(c_isAH(c) && w<3)
				{
				if(w==1) w++;
				if(w==0 && qt[0]==s[f] && qt[1]==s[f+1]) w=4;
				if(w==0) if(qf[0]!=c) { cont=true; break; }
				if(w==2) if(qt[0]!=c) { cont=true; break; }
				w++;          
				}
			else if(c_is18(c))
				{
				if(w==0 || w==2) w++;
				if(w==1) if(qf[1]!=c) { cont=true; break; }
				if(w==3) if(qt[1]!=c) { cont=true; break; }
				w++;                  
				}
			else { cont=true; break; }
			if(w>2) y=f;          
			}
		if(cont) continue; 
         
        if((*o).p!=0)
			{
			cont=true;
			z=strchr(s,'='); if(z!=NULL) y=(z-s)+1;
			for(j=y;;)
				{
				c = s[j++];
				if(c==0) break;
				c = c_toUpper(c);
				if(c==(*o).p) { cont=false; break; }
				}
			if(cont) continue;
			}       
        
		r=k;
		}
        
     return r;
     
    };

	void move2unicode(bool unicoded, char *mto, char *m, char c, int k) {       /* converts to unicode symbols */

	if(!unicoded) strcpy(mto,m);
    else
	 {
	  int i, j, a=c1_chess::indexOfchar(c1_chess_S,c);
	  sprintf(mto,"&#98%d;%s",11+a,
		  &m[ (k>0||((c=='P'||c=='p')&&m[1]!='x') ? 0 : 1 ) ]);
	  i=c1_chess::indexOfchar(mto,'=');
	  if(i>0)	// if promotion then convert promoted piece too
		{
		strcpy(&mto[35],&mto[i+2]);		// 35+...
		j=c1_chess::indexOfchar(c1_chess_S, mto[i+1] );
		sprintf(&mto[i+1],"&#98%d;%s",11+j+(a<7?0:6), &mto[35]);
		}
	
	 }
    };

	//=============================== PARSE PGN OF A GAME

	void ParsePgn_prepare( char *pgn ) {

	char c, *p, *wi=NULL, *z;

	for(p=pgn;;p++)			// convert TAB,CR,LF to ' '
		{
		c = *p;
		if(c==0) break;
		else if(c<=13) *p = ' ';
		}
	p=pgn;

	G[gc].H = NULL;
	G[gc].srtd = false;

	int q=0,f=0;
	for(;;p++)
		{
		c=(*p);
		if(c==0) { p=pgn; break; }
		else if(c=='[')
			{
			if(f==0) { f=1; wi=p; }
			q++;
			}
		else if(c==']') q--;
		else if(q==0 && c!=' ') break;
		}

	if(f==0) G[gc].H = NULL;
	else
		{
		c = (*p);
		(*p) = 0;
		strcpy(&Hb[hc],wi);
		(*p) = c;
		G[gc].H = &Hb[hc];
		hc+=(p-wi+1);
		}

	G[gc].rl = strlen( G[gc].H );		// before we split the string
	char *p_round = getHeaderData( gc, c1_pgn_hdr_round, G[gc].rl );

	G[gc].srnd = c1_chess::toUInt(p_round) << 10;
	for(z=p_round;;)
		{
		c = *(z++);
		if(c==0) break;
		if(c=='.' || c==',')
			{
			G[gc].srnd += c1_chess::toUInt(z);
			break;
			}
		}

	int ln = G[gc].sl = strlen(p);
	char *m = G[gc].pgn = &Pb[hpc];

	strcpy( m, p );
	hpc += ln;

	};
	

	void SortGamesByRounds() {	// sorting by rounds, before parsing chess logic
		long i,n=0,j;
		for(;n<gc; )
			{
			if(fsort)
				{
				int r = 1<<14;
				for(i=0;i<gc;i++)
					{
					if(!G[i].srtd)
						{
						if(G[i].srnd<r) { j=i; r=G[i].srnd; }
						}
					}
				}
			else j=n;
			G[j].srtd = true;
			Sort[n++] = j;
			}
	};

	void ParsePgn( int gm )	{	// do the parsing

	Vstru *v1;
	vc = 0;
	
	sprintf(be, "<tr><td>"); be+=strlen(be);

	canvCnt = 0;
	int slen = G[gm].sl;
	char c, *p = G[gm].pgn;
	char c_last = p[slen];
	p[slen] = 0;

	long j = AddEmptySubTreeArray(0);
	G[gm].Ti = j;

	char *be_0 = &Bhtm[ strlen(Bhtm) ], *tbbe_0 = Btbl;

	int q=0,f=0,Mn = 0,ff=1;
	bool vf = false;
	Ttree Tw;
	int i,u,w,k,nI, cId=0;
	int rl = G[gm].rl;
	char *p_event = getHeaderData( gm, c1_pgn_hdr_event, rl );
	char *p_date = getHeaderData( gm, c1_pgn_hdr_date, rl );
	char *p_white = getHeaderData( gm, c1_pgn_hdr_white, rl );
	char *p_black = getHeaderData( gm, c1_pgn_hdr_black, rl );
	char *p_round = getHeaderData( gm, c1_pgn_hdr_round, rl );
	char *p_site = getHeaderData( gm, c1_pgn_hdr_site, rl );
	char *p_result = getHeaderData( gm, c1_pgn_hdr_result, rl );
	char *p_FEN = getHeaderData( gm, c1_pgn_hdr_FEN, rl );

	if(strlen(p_white)>0)
		{
		sprintf(be,"<div class=\"_pgn_header\">%s %s ", p_white, p_result); be+=strlen(be);
		sprintf(be,"%s </div><div class=\"_pgn_header2\">(%s", p_black, p_date ); be+=strlen(be);
		sprintf(be,"%s %s%s %s", (strlen(p_event)>0 ? "," : "" ), p_event, (strlen(p_site)>0 ? "," : "" ), p_site);  be+=strlen(be);
		sprintf(be,"%s %s )</div><br>", (strlen(p_round)>0 ? ", round " : "" ), p_round);  be+=strlen(be);

		if((rf&2)>0)
			{
			sprintf(tbbe,"%s vs. %s (%s)\n", p_white, p_black, p_result); tbbe+=strlen(tbbe);
			sprintf(tbbe,"%s", p_date ); tbbe+=strlen(tbbe);
			sprintf(tbbe,"%s %s", (strlen(p_event)>0 ? "," : "" ), p_event); tbbe+=strlen(tbbe);
			sprintf(tbbe,"%s %s", (strlen(p_site)>0 ? "," : "" ), p_site);  tbbe+=strlen(tbbe);
			sprintf(tbbe,"%s %s\n", (strlen(p_round)>0 ? ", round " : "" ), p_round);  tbbe+=strlen(tbbe);
			}
		}

	c1_chess B2;
	startFEN = (strlen(p_FEN)>0 ? p_FEN : c1_chess_sFEN);
	B2.setFEN( startFEN );
	ns = 0;

	G[gm].idkN = rnDk + pc_total+pc;

	char tabspaces[1024<<2], m[50], nags[254], fen[100];

	for(i=0;i<slen;i++, *(be)=0)
		{
		if((i & 0xf)==0)
			{
			cId = (strlen(be_0)/dist);
			if((++cId)>canvCnt)
				{
				sprintf(be,"<canvas id=\"_pgn_D_%d%d",G[gm].idkN,cId); be+=strlen(be);
				sprintf(be,"\" width=\"%d\" height=\"%d\" style=\"float:%s\"></canvas>",
					canvSize, canvSize, (cId&1 ? "left" : "right")); be+=strlen(be);
				canvCnt=cId;
				}
			}
		c = p[i];
		if(strchr(c1_pgn_p1s,c)!=NULL) { *(be++)=c; continue; }
		if(c=='<') { sprintf(be,"&lt;"); be+=strlen(be); continue; }
		if(c=='>') { sprintf(be,"&gt;"); be+=strlen(be); continue; }
		char t[50], th[50], tu[50];
		cutword( &p[i], t );
		int t1=strlen(t);
		sprintf(th," %s ",t);

		if(c=='$' && t1>1)
			{
				nI = c1_chess::toUInt(&p[i+1]);
				c1_getNAGdescr(nI,nags);
				if( strlen(nags) == 0 )
					{
					sprintf(be,"%s",t); be+=strlen(be);
					}
				else
					{
					sprintf(be,"<div class=\"_pgn_nag\">"); be+=strlen(be); 
					if(nI<5) sprintf(be,"<b>"); be+=strlen(be);
					for(q=0;q<strlen(nags);)
						{
						c = nags[q++];
						if(c=='<') sprintf(be,"&lt;");
						else if(c=='>') sprintf(be,"&gt;");
						else sprintf(be,"%c",c);
						be+=strlen(be);
						}
					if(nI<5) sprintf(be,"</b>"); be+=strlen(be);
					sprintf(be,"</div> "); be+=strlen(be); 
					}
				i+=(t1-1);    // skip
			}
		else if( (strcmp(t,c1_pgn_Z0)==0) || c=='%' || (t1>2 && strstr(c1_pgn_p2stm,th)!=NULL))
				{
				sprintf(be,"%s",t); be+=strlen(be);
				i+=(t1-1);    // skip
				}
		else
			{ 
     
			if(c=='{' || c=='(')
				{
				Vr[vc].c = c;		// new variant
				Vr[vc].i = i;
				Vr[vc].Tv = j;
				Vr[vc].N = Mn;
				Vr[vc].sm = B2.sm;
				Vr[vc].f = 0;
				vc++;

				if(tabs)
					{	someSpaces( tabspaces ); sprintf(be,"%s",tabspaces); be+=strlen(be); }

				sprintf(be,"<div class=\"_pgn%c\">",(c=='{' ? '2' : '3')); be+=strlen(be);
				if(!tabs) *(be++)=c;

				continue;  
				}

			if(vc>0)
				{
				v1 = &Vr[vc-1];   
				if(( (*v1).c=='(' && c==')') || ( (*v1).c=='{' && c=='}'))
					{

					j = (*v1).Tv;
					Tw = Tr[j];
					setPosAndMove( &B2, Tw );
					Mn=Tw.N;
					if(B2.sm>0) Mn++;
  
					vc--;        // end of last variant

					vf=true;

					if(!tabs) *(be++)=c;  
					sprintf(be,"</div>"); be+=strlen(be);
					if(tabs)
						{	someSpaces( tabspaces ); sprintf(be,"%s",tabspaces); be+=strlen(be); }

					continue; 
					}
				}

				if(c>'0' && c<='9')    // move number
					{
					char col = ' ';
					for(w=i+1;w<slen;w++)
						{
						c=p[w];
						if(!(c==' ' || (c>='0' && c<='9'))) break;
						}
					if(w<slen)
					{ 
					if(c=='.')
						{
						if(vc==0 || (*v1).f<3)
							{
							int Was_Mn = Mn, Was_sm = B2.sm;
							p[w]=0; Mn = c1_chess::toUInt(&p[i]); p[w]=c;

							if(vc==0 && ns==0) ns = Mn;   // number starts from
							if(vc>0 && (*v1).f<2)
								{
								if(p[w]=='.' && p[w+1]=='.') { if(B2.sm>0) B2.sm=-B2.sm; }
								else { if(B2.sm<0) B2.sm=-B2.sm; }
								if((*v1).f==0)
									{
									(*v1).N = Mn; (*v1).sm = B2.sm; (*v1).f=1;    // starting point for variant
									}
								if((Was_Mn==Mn && Was_sm>B2.sm) || (Was_Mn<Mn)) (*v1).f=3; //move point ahead
								long j_pre = j;
								for(u=vc-1;u>=0 && (*v1).f<2;u--)
									{
									v1 = &Vr[u];
									j = (*v1).Tv;
									if(!IsEmptyTreeArray(j))
										for(j = GoLastElementTreeArray(j); j!=0; )
											{
											Tw = Tr[j];
											if(Tw.sm==0) break;
											if((Tw.N==Mn-1 && Tw.sm<B2.sm) || (Tw.N==Mn && Tw.sm>B2.sm))
												{
												setPosAndMove( &B2, Tw );
												u=-99;
												break;
												}
											if(Tw.T_pre==0) break;
											j=Tw.T_pre;
											}
									}
									if(u>-99 && u<0)
										{
										j = j_pre;
										}      // don't know which move                                    
								}
							}
						col = ((p[w]=='.' && p[w+1]=='.') ? 'b' : 'w');
						for(;w<slen && p[w]=='.';w++); 
						} 
					}

					c = p[w]; p[w]=0;
					sprintf(be,"%s", &p[i]); be+=strlen(be);
					p[w] = c;

					i+=(w-i-1);
					}
				else
					{
					if(t1>0 && ns>0 && Mn>0 && (vc==0 || (*v1).f<3))
						{
														
							// analyse move       
						k=moveTry(B2, t);
     
						if(k>=0 && (vc==0 || (*v1).f>0 || t1>2))
							{
							if(vc>0 && (*v1).f<2)
								{
								j = AddEmptySubTreeArray(j);
								}
							j = AddElementToTreeArray(j);
							B2.I2long(k,m);
							B2.g2uci( &B2.genml[k], Tr[j].uci );
							B2.getFEN( fen );
							strcpy( Pt[pc].FEN, fen );
							Tr[j].pstruI = pc;
							Tr[j].k = k;
							Tr[j].N = Mn;
							Tr[j].sm = B2.sm;
							Pt[pc].curmv = B2.mc;

							B2.mkmove(k);
							B2.moveGen();

							c1_chess::hlist *h = &B2.hist[B2.mc-1];
							if((*h).e>0) strcpy( &m[ strlen(m) ], c1_pgn_ep ); 

							if(vc==0)
							{
							 if(rf==2)
								{
								sprintf(tbbe,"%s\n\n",fen); tbbe+=strlen(tbbe);
								sprintf(tbbe,"%d%s%s\n", Mn, (B2.sm<0 ? "." : "..."), m); tbbe+=strlen(tbbe);
								B2.sPrintBoard( tbbe ); tbbe+=strlen(tbbe);
								}
							 if(rf==3)
								{
								if(ff>0)
									{
									if(strcmp(fen,c1_chess_sFEN)!=0)
										{
										sprintf(tbbe,"%s\n",fen); tbbe+=strlen(tbbe);
										B2.sPrintBoard( tbbe ); tbbe+=strlen(tbbe);
										}
									}
								if(B2.sm<0 || ff>0)
									{
									sprintf(tbbe,"%d%s", Mn, (B2.sm<0 ? "." : "..."));
									tbbe+=strlen(tbbe);
									}
								sprintf(tbbe,"%s ",m); tbbe+=strlen(tbbe);
								ff=0;
								}
							}

							if(B2.sm>0) Mn++;
							long pTp = rnDk + pc_total+pc;
							Pt[pc].idk = G[gm].idkN;

							move2unicode(ucdsymb, tu, t, (*h).m.z , (*h).m.k );

							sprintf(be,"<div class=\"_pgn1\" id=\"_pI%d\" onclick=\"_pOc(%d)\">%s</div>",
								pTp, pTp, tu ); be+=strlen(be);

							strcpy( Pt[pc].uci, Tr[j].uci );
							Pt[pc].v = vc; Pt[pc].vf = vf;
							pc++;
							if(vf>0) vf=0;
							if(vc>0 && (*v1).f<2) (*v1).f=2;
							}
						else
							{
							sprintf(be,"%s",t); be+=strlen(be);
							}       
        
						i+=(t1-1);       
						}
					else *(be++) = p[i];
					}
			}
		}

		p[slen] = c_last;

		int l2;
		if((rf&2)>0)
			{
			B2.getFEN( fen );
			if((rf&2)>0)
				{
				if(rf==2) sprintf(tbbe,"%s\n",fen); tbbe+=strlen(tbbe);
				sprintf(tbbe,"%s\n\n\n", p_result); tbbe+=strlen(tbbe);
				}
			l2 = strlen(be_0);

			fprintf(out,"%s", tbbe_0);		// simply flush txt file
			tbbe = &tbbe_0[0];
			*(tbbe)=0;

			// this removes html datas, not needed
			l2 = 0;
			be=&be_0[l2];
			*(be)=0;
			}
		else
			{
			c1_elimspaces( be_0 );
			c1_encodedata( be_0, Btbl );
			sprintf(be_0,"%s>document.write(_c1_dcd('%s'));%s", c1_scrBegin, Btbl,  c1_scrEnd);
			l2 = strlen(be_0);
			be=&be_0[l2];
			}

		bLength += l2;
		sprintf(be, "</td></tr>"); be+=strlen(be);
		if( (bLength+1024)>c1_pgn_file_size ) tooLarge = true;

	};

	// get header data by game nr
	char *getHeaderData( int gm, char *tag, int l ) {
		char *p = G[gm].H, *pt=NULL, *q, *v;
		char c, t[30];
		int f=0,j;
		t[0]='[';

		strcpy(&t[1], tag);
		int w = strlen(t);
		for(j=0;j<l;j++,p++)
			if(strncmp(p, t,w)==0) break;
		pt = p;
		for(; j<l && f<2; p++,j++)
			{
			c=(*p);
			if(c=='"' || c==0)
				{
				if(f==0) { pt=p; pt++; }
				else if(f==1) (*p)=0;
				f++;
				}
			}
		for(q=pt; f>1; q++)	/* remove ? */
			{
			c=(*q);
			if(c==0) break;
			if(c=='?')
				{
				for(v=q;;)
					{
					c=*(++v);
					if(c!='?') *(q++)=c;
					if(c==0) break;
					}
				break;
				}
			}
		return pt;
	};


	FILE *in,*out;		// file handlers

	// read and parse Pgn file (not limited in size)
	void ReadParsePgnFile( char *fname, char *to_fname ) {
		
		size_t rd,i,n;
		int l,k,f;
		char *st=NULL, *at, *to, *v, *p, c, *q;
		long count=0, c2=0;
		char *b2 = &reader_buf[ c1_pgn_filereader_buffer_size ];

		// empty output
		be = Bhtm; (*be)=0;
		ptbe = Bht2; (*ptbe)=0;
		pobe = Bht3; (*pobe)=0;
		tbbe = Btbl; (*tbbe)=0;

		tooLarge = false;
		bLength = 0;

		sprintf(be, "<!doctype html><head>"); be+=strlen(be);
		sprintf(be, "<meta http-equiv=Content-Type content=\"text/html; charset=utf-8\">"); be+=strlen(be);
		sprintf(be, "%s%s%s", c1_htmcss0, c1_domain, c1_htmcss1 ); be+=strlen(be);
		v = to_fname;		// sets title for the htm-file
		q = v;
		l = strlen(v);
		for(k=l-1,f=0; k>=0; k--)
			{
			c = v[k];
			if(c=='/' || c=='\\') { q = &v[k+1]; if(f==0) p = &v[l]; break; }
			if(c=='.') { p=&v[k]; f=1; }
			}
		if(k<0 && f==0) p = &v[l];
		c = *(p); *(p)=0;
		sprintf(be, "<title>%s</title>",q); be+=strlen(be);
		*(p) = c;

		sprintf(be, "</head><body>"); be+=strlen(be);
		sprintf(be, "%ssrc=\"%sc1_chess.js\">%s", c1_scrBegin, c1_domain, c1_scrEnd ); be+=strlen(be);
		sprintf(be, "<table>"); be+=strlen(be);

		sprintf(ptbe,"%s> _pgn_Pt=[];_c1_Pt_add(\"",c1_scrBegin ); ptbe+=strlen(ptbe);
		out = (((rf&2)>0) ? fopen( to_fname, "wb" ) : NULL);
		in = fopen( fname, "rb" );
		if(in!=NULL)
			{
			for(;!tooLarge && !feof(in);)
				{
				rd = fread(b2,1, c1_pgn_filereader_buffer_size ,in);
				b2[rd]=0;
				for(;;)
					{
					i=strlen(b2);
					if(i>=rd) break;
					b2[i]=' ';		// replace 0th char, maybe unicode or mistake
					}
				b2[rd]=0;
				if(st==NULL)
					{
					at = strstr(b2,c1_pgn_evPtr);		// first event
					if(at==NULL) break;				// no events at all
					st = (at++);
					}
				for(;!tooLarge;)
					{
					at = strstr(at,c1_pgn_evPtr);
					if((at==NULL) && (!feof(in))) break;
					if(at!=NULL) *(at)=0;
					ParsePgn_prepare(st); gc++;
					if(at!=NULL) *(at)='[';
					count++; c2++;
					if(c2==c1_pgn_buffer_games_cnt)
						{
							fsort = false;	// don't sort, long file
							for(n=0;n<c2;n++)
								{
								if((n & 0xf)==0) printf( ".");
								ParsePgn(n);
								}
							ptbeAddMousePointers();
							pc_total += pc;
							c1_0();	// clear all
							printf( "%d\n", count );
							c2 = 0;
						}
					if(at==NULL) break;
					st = (at++);
					}
				to = &reader_buf[(c1_pgn_filereader_buffer_size-rd)+(st-b2)];
				strcpy(to, st);
				at = to;
				st = (at++);
				}
			}

		SortGamesByRounds();

		for(n=0;n<c2;n++)
			{
			if((n & 0xf)==0) printf( ".");
			ParsePgn( fsort ? Sort[n] : n );
			}

		printf( "%d\n", count );

		ptbeAddMousePointers();
		sprintf(ptbe,"i\");"); ptbe+=strlen(ptbe);

		char Q[30]; Q[0]=0;
		if(rnDk>0)
			{
			sprintf(Q,"%d",rnDk);
			sprintf(ptbe,"_c1_ML(%s);",Q); ptbe+=strlen(ptbe);
			}
		sprintf(ptbe,"function OnWinLd%s(){_c1_Pt_OnLd([%s])}",Q,(++pobe)); ptbe+=strlen(ptbe);
		sprintf(ptbe,"var oOnLd%s=window.onload;if(typeof(window.onload)==\"function\") ",Q); ptbe+=strlen(ptbe);
        sprintf(ptbe,"window.onload=function(){if(oOnLd%s) oOnLd%s();OnWinLd%s()};",Q,Q,Q); ptbe+=strlen(ptbe);
		sprintf(ptbe,"else window.onload=function(){OnWinLd%s()}%s",Q, c1_scrEnd ); ptbe+=strlen(ptbe);
		sprintf(be, "</table>%s", Bht2); be+=strlen(be);
		sprintf(be, "</body></html>"); be+=strlen(be);

		if(tooLarge)
			{ sprintf(Bhtm, "<html><body>Result too large for %dMb buffer. Increase /L=Mbytes option.</body></html>",
				c1_pgn_file_size>>20) ; }

		if(in!=NULL) fclose(in);
		if(out==NULL) out = fopen( to_fname, "wb" );
		if(out!=NULL && ((rf&2)==0)) fprintf(out,"%s", Bhtm );
		if(out!=NULL) fclose(out);
	};

	void ptbeAddMousePointers() {
		int k,u=0,q;
		for(k=0;k<pc;k++)
			{
				Pstru *p = &Pt[k];
				int vf = (*p).vf;
				if(vf==0 && k>0)
					if(Pt[k-1].v<(*p).v) vf=1;

				sprintf(ptbe,"i%dv%du%sz",(*p).idk, (*p).v, (*p).uci); ptbe+=strlen(ptbe);

				long pTp = rnDk + pc_total+k;
				if(pTp == (*p).idk)
					{
					char m[20];
					sprintf(m,",%d", pTp);
					for(q=strlen(m)-1;q>=0;q--) *(--pobe) = m[q];
					vf=1;
					}
				if(vf>0 || ((*p).curmv % 10 == 0))
					{
					if( strcmp( (*p).FEN, c1_chess_sFEN )==0 ) *(ptbe++)='0';
					else
						{
							char c, *q;
							for(q=(*p).FEN;;)
								{
									c = *(q++);
									if(c==0) break;
									if(c!='/') *(ptbe++)=c;
								}
						}
					}
			u++;
			if(u==50 && k+10<pc)
				{
					u=0;
					sprintf(ptbe,"i\");_c1_Pt_add(\""); ptbe+=strlen(ptbe);
				}
		}

	};

	void c1_0() { 		// sets counters to 0
		tc = 0; gc = 0; hc = 0; hpc = 0; vc = 0; pc = 0; AddEmptySubTreeArray(0);
	};

	c1_pgn()
	{
		strcpy( c1_domain, c1_domain_c );	// main online domain
		rnDk = 0;

		/* --- remove these user option values when porting c_pgn class ---*/
		if(userGamesCnt>0) c1_pgn_buffer_games_cnt = userGamesCnt;
		if(userFileLimitSize>0) c1_pgn_file_size = userFileLimitSize;
		if(userNoDomainOption) c1_domain[0]=0;	// no domain
		if(userSafe4multiPosts) rnDk = rand()*999999;
		/* ------- */


		// memory allocation
		G = (Gstru *) malloc( sizeof(Gstru) * (c1_pgn_buffer_games_cnt+1));
		Sort = (long *) malloc( sizeof(long) * (c1_pgn_buffer_games_cnt+1));
		Tr = (Ttree *) malloc( sizeof(Ttree) * c1_pgn_buffer_games_cnt * (c1_pgn_avrg_positions_per_game+1));
		Hb = (char *) malloc( (c1_pgn_buffer_games_cnt+1) * c1_pgn_avrg_header_buffer_size );
		Pb = (char *) malloc( (c1_pgn_buffer_games_cnt+1) * c1_pgn_avrg_pgn_buffer_size );
		Vr = (Vstru *) malloc( sizeof(Vstru) * (c1_pgn_variants_cnt+1));
		reader_buf = (char *) malloc( (c1_pgn_filereader_buffer_size+1)<<1 );	// buffer 2x;
		Bhtm = (char *) malloc( c1_pgn_file_size*3 );
		Bht2 = &Bhtm[ c1_pgn_file_size ];	// some space ahead
		Bht3 = &Bht2[ c1_pgn_file_size-2 ];
		Btbl = &Bhtm[ c1_pgn_file_size<<1 ];
		Pt = (Pstru *) malloc( sizeof(Pstru) * c1_pgn_buffer_games_cnt * (c1_pgn_avrg_positions_per_game+1));
		c1_0();
		pc_total = 0;
		dist = 8000;	// canvas after each 8k characters 
		canvSize = 200;	// 200px
		tabs = false;
		
		fsort = false;
		rf = 1;			// default to html
		ucdsymb = false;
		tooLarge = false;
	};	// on object creation

	~c1_pgn()
	{
		free(Pt);
		free(Bhtm);
		free(reader_buf);
		free(Vr);
		free(Pb);
		free(Hb);
		free(Tr);
		free(Sort);
		free(G);
	};	// on object deletion

};			// ------------------end of c1_pgn class

/*
void c1_pgn_samples() {

	// 1.single game functions
	c1_chess B;
	B.setFEN( c1_chess_sFEN );
	B.uciMoves( "e2e4 a7a6 e4e5 f7f5 e5f6");
	c1_pgn P1;
	P1.fromhist(B);			// to tree
	char ucimoves[100];
	P1.uciMoves(ucimoves,0);	// from tree
	printf( "Uci moves:\n%s\n", ucimoves );

	// 2.parse files
	c1_pgn P;
	
	// make html file
	P.tabs = true;	// not inline
	P.ReadParsePgnFile("c:\\chess\\GarryKasparov\\Garry_Kasparov_1993_1994_pgn.txt", "OUTPUT.HTM");
	// and clear P object
	//P.c1_0();

	P.rf = 2;	// make a text file
	P.fsort = true;	// sort
	P.ReadParsePgnFile("c:\\chess\\GarryKasparov\\Garry_Kasparov_1993_1994_pgn.txt", "OUTPUT.TXT");

	if(P.tooLarge) printf( "Result too large.\n" );
	getchar();
};
*/

#endif  /* _INC_C1_PGN_CHESS */