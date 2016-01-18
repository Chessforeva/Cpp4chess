/*
================================================================================
=   CHESS class - The basic chess logic: complete board in variables,
=   move generation, position setup, make & unmake move, history of all moves,
=   printing of board and moves done to string pointer,
=
=   for copy+paste on MS Studio C++, or GCC projects
=   look for c1_samples
=   no dynamic memory allocation
=   no libraries to include at all
================================================================================
*/

#ifndef _INC_C1_CHESS
#define _INC_C1_CHESS

// Some data strings
static char *c1_chess_sFEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1\0",
	*c1_chess_S = " KQRBNPkqrbnp\0", *c1_chess_Sw = "KQRBNP\0", *c1_chess_Sb = "kqrbnp\0",
	*c1_chess_m = "O0KQRBN\0", *c1_chess_cs = "KQkq\0",
	*c1_chess_OO = "0-0\0", *c1_chess_OOO = "0-0-0\0";

class c1_chess {				// ------------------[start of c1_chess class]

	static const int Pe=0, Pwk=1, Pwq=2, Pwr=3, Pwb=4, Pwn=5, Pwp=6,
						Pbk=7, Pbq=8, Pbr=9, Pbb=10, Pbn=11, Pbp=12;
	static const int MAX_moves_hist = 300*2;		// buffer size will be for 300 moves history
	static const int MAX_possible_moves = 100;		// buffer size for possible next moves

public:

	// independent code, could use string.h
	static bool isUpper( char c ) { return (c>='A' && c<='Z'); }
	static bool isLower( char c ) { return (c>='a' && c<='z'); }
	static bool isAlpha( char c ) { return isUpper(c) || isLower(c); }
	static bool isDigit( char c ) { return (c>='0' && c<='9'); }
	static char toUpper( char c ) { return ( isLower(c) ? c-'a'+'A' : c ); }
	static char toLower( char c ) { return ( isUpper(c) ? c-'A'+'a' : c ); }
	static int iAbs( int n ) { return ( n<0 ? -n : n ); }

	static int indexOfchar( char *s, char c )	// searches char in string
	{
		for(int i=0;s[i]!=0;i++)
			if(s[i]==c) return i;
		return -1;
	}
	static int toUInt( char *s )	// converts positive integer to string
	{
		int r=0;
		for(int i=0;;i++)
		{
			char c = s[i];
			if(isDigit(c)) r=(r*10)+(c-'0');
			else break;
		}
		return r;
	}
	static void toString( int i, char *s )		// converts positive integer to string
	{
		int r=i,d=10,k=0;
		for(;r>=d;d*=10);
		for(;d>1;)
		{
			d/=10;
			int a = r/d;
			r-=(d*a);
			s[k++]=('0'+a);
		}
		s[k]=0;
	}

	static int lenOfString( char *s )		// returns length of string
	{
		int i=0;
		for(;i<1 || s[i-1]!=0;i++);
		return i-1;
	}

	static void setString( char *s, char *from )		// sets string, copies
	{
		for(int i=0;i<1 || s[i-1]!=0;i++) s[i]=from[i];
	}

	char B[64];		// chess board 64 squares [ 0pnbrqkPNBRQK ]
	int sm;			// side to move now (1-white,-1 black)
	char cs[4];     // KQkq castlings
	int ep;			// en-passant square or -1

	struct glist {		/* structure for list of possible moves generation */
		int f;		// from square [0..63]
		int t;		// to square [0..63]
		char p;		// piece to promote 0QRBN
		char z;		// piece to move [ pnbrqkPNBRQK ]
		char c;		// check, checkmate sign
		int k;		// castling left or rigth
	} genml[MAX_possible_moves];
	int gc;			// count of possible moves now

	struct hlist {		/* structure for list of history of all moves */
		glist m;	// move information
		char u;		// piece was on square to
		int e;		// this is enpassant capture and here is square
		char cs[4];	// castling-statuses before the move
		int ep;		// en-passant pre-information for unmake
	} hist[MAX_moves_hist];
	int mc;			// move counter

	bool isWhite( char p ) { return ( isAlpha(p) ? indexOfchar(c1_chess_Sw,p)>=0 : (p>0 && p<Pbk)); };
	bool isBlack( char p ) { return ( isAlpha(p) ? indexOfchar(c1_chess_Sb,p)>=0 : (p>Pwp)); };
	int pcI( char p ) { return indexOfchar(c1_chess_S,p); };
	int sq2i( char *sq ) { return ((sq[1]-'1')<<3) + (sq[0]-'a'); };
	void i2sq( int i, char *sq ) { sq[0]=('a'+(i&7)); sq[1]=('1'+(i>>3)); };

	int X,Y;	// for move generation
	int iX(int i) { return (i&7); };
    int iY(int i) { return (i>>3); };


	bool setFEN( char *pos ) {            /* sets position by FEN, returns false on error */

    int i, j, y=7, x=0;
	char c;
	for(i=0;i<64;i++) B[i]=0;

	for(i=0;;i++)
	{
	c = pos[i];
	if(c==' ') break;
	if(c==0) return false;
	if(c!='/')
		{
		if(isDigit(c)) x+=(c-'1');
		else if(isAlpha(c)) B[(y<<3)+x]=c;
		if(x>6) { x=0; y--; } else x++;
		}
	}
	c = pos[++i];
	sm = (c=='w'?1:-1);
    cs[0]=cs[1]=cs[2]=cs[3]=0;
	for(i+=2;;i++)
	{
	c = pos[i];
	if(c==' ') break;
	if(c==0) return false;
	j=indexOfchar(c1_chess_cs,c);
	if(j>=0) cs[j]=1;
	}
	c = pos[++i];
	ep = ((c!=0 && c!='-' ) ? sq2i(&c) : -1 );
	mc = 0;
	for(int q=0;;i++)
	{
	c = pos[i];
	if(c==0) return true;
	if(c==' ') q++;
	else if(q>1) break;
	}
	mc = ((toUInt(&pos[i])-1)<<1)+(sm<0?1:0);	// simply ignores garbage in hist
    moveGen();
	return true;
    };

	void getFEN( char *s ) {                 /* gets current FEN of position */

	int i=0;
    for(int y=7; y>=0; y-- )
		{
		int a=0;
		for(int x=0; x<8; x++ )
		{
		char c=B[(y<<3)+x];
		if(c==0) a++; else { if(a>0) s[i++]=('0'+a); a=0; s[i++]=c; }
		}
		if(a>0) s[i++]=('0'+a);
		if(y>0) s[i++]='/';
		}
    s[i++]=' ';
	s[i++]=(sm>0?'w':'b');
	s[i++]=' ';
    if(cs[0]+cs[1]+cs[2]+cs[3]==0) s[i++]='-';
    else
		{
		for(int j=0;j<4;j++)
			if(cs[j]>0) s[i++]=c1_chess_cs[j];
		}
    s[i++]=' ';
	if(ep<0) s[i++]='-';
	else
		{
		i2sq(ep,&s[i]); i+=2;
		}
	s[i++]=' ';
	s[i++]='0';
	s[i++]=' ';
	toString((mc>>1)+1,&s[i]);
    };


	void sPrintBoard(char *outp_str) {            /* prepares string for printing board position (simple text) */
    int i=0;
	char *p=outp_str;
	for(int y=7; y>=0; y-- )
		{
		for(int x=0; x<8; x++ )
			{
			char c=B[(y<<3)+x];
			p[i++] = ((c==0) ? '.' : c);
			}
		p[i++] = 13; p[i++] = 10;
		}
	p[i] = 0;
    };

	//------------------------- [ move generation ]------------------------

	void moveGen () { moveg1(0); };           /* generates list of moves */

	void push_genml(int f, int t, char p, char c, char z, int k)
	{
		glist *o = &genml[gc++];
		(*o).f = f; (*o).t = t; (*o).p = p;
		(*o).c = c; (*o).z = z; (*o).k = k;
	};

	bool addmvvalid() {          /* pre-validation of move */

	bool r = false;
	mkmove(gc-1);
	sm=-sm;
	r=isCheck();   // is king still under check
	sm=-sm;
	unmkmove();
    return r;
	};

	void addmove(int f, int t, int r, int k) {
								/* move from square to square, promoted piece,
                                 check-mate sign, castling sign */
    char z = B[f];
    if(r>0)
		{
		push_genml( f,t,'Q',0,z,k);
		if(addmvvalid()) gc--;
		else
			{
			push_genml(f,t,'R',0,z,false);
			push_genml(f,t,'B',0,z,false);
			push_genml(f,t,'N',0,z,false);
			}
		}
	else
		{
		push_genml(f,t,0,0,z,k);
		if(addmvvalid()) gc--;
		}
	};

	void pawngen( int i ) {            /* generates all pawn moves */

	int r=0;
	char p=0;
    if(sm>0)
	{
	if(Y==6) r=1;
	if(X>0 && Y<7)
		{
		p=B[i+7];
		if(isBlack(p) || (ep==i+7) ) addmove(i,i+7,r,0);
		}
	if(X<7 && Y<7)
		{
		p=B[i+9];
		if(isBlack(p) || (ep==i+9) ) addmove(i,i+9,r,0);
		}
	if(Y<7 && B[i+8]==0)
		{
		addmove(i,i+8,r,0);
		if(Y==1 && B[i+16]==0) addmove(i,i+16,0,0);
		}
	}
	else
	{
	if(Y==1) r=1;
	if(X<7 && Y>0)
		{
		p=B[i-7];
		if(isWhite(p) || (ep==i-7) ) addmove(i,i-7,r,0);
		}
	if(X>0 && Y>0)
		{
		p=B[i-9];
		if(isWhite(p) || (ep==i-9) ) addmove(i,i-9,r,0);
		}
	if(Y>0 && B[i-8]==0)
		{
		addmove(i,i-8,r,0);
		if(Y==6 && B[i-16]==0) addmove(i,i-16,0,0);
		}
	}

    };

	void knightgen( int i ) {            /* generates all knight moves */

	for(int w=-2;w<3;w++)
	 for(int z=-2;z<3;z++)
		if( (iAbs(w)+iAbs(z))==3 )
		{
		int m=X+w, n=Y+z;
		if(m>=0 && m<8 && n>=0 && n<8)
			{
			int j=(n<<3)+m;
			char c=B[j];
			if(c==0 || isBlack(c)==(sm>0) ) addmove(i,j,0,0);
			}
		}
    };

	void bishopgen( int i ) {            /* generates all bishop moves */

    for(int w=-1;w<2;w+=2)
	 for(int z=-1;z<2;z+=2)
	  for(int q=1;q<8;q++)
		{
		int m=X+(q*w); int n=Y+(q*z);
		if(m>=0 && m<8 && n>=0 && n<8)
			{
			int j=(n<<3)+m;
			char c=B[j];
			if(c==0 || isBlack(c)==(sm>0) ) addmove(i,j,0,0);
			if(c!=0) q=8;
			}
		else q=8;
		}
    };

	void rookgen( int i ) {            /* generates all rook moves */

    for(int w=-1;w<2;w++)
	 for(int z=-1;z<2;z++)
		if(w==0 || z==0)
			for(int q=1;q<8;q++)
			{
			int m=X+(q*w), n=Y+(q*z);
			if(m>=0 && m<8 && n>=0 && n<8)
				{
				int j=(n<<3)+m;
				char c=B[j];
				if(c==0 || isBlack(c)==(sm>0) ) addmove(i,j,0,0);
				if(c!=0) q=8;
				}
			else q=8;
			}
    };

	void queengen( int i ) {            /* generates all queen moves */

    rookgen(i);
    bishopgen(i);
    };

	void kinggen( int i ) {            /* generates all king moves */

    for(int w=-1;w<2;w++)
	 for(int z=-1;z<2;z++)
		{
        int m=X+w, n=Y+z;
		if(m>=0 && m<8 && n>=0 && n<8)
			{
			int j=(n<<3)+m;
			char c=B[j];
			if(c==0 || isBlack(c)==(sm>0) ) addmove(i,j,0,0);
			}
		}
    int q=(sm>0?0:2);
	for(int x=0;x<2;x++)
	 if(cs[x+q]>0)
		{
		int e1=(sm>0?4:60), d=(x==0?1:-1), cc=1, y;
		for(y=1;y<3+x;y++)
			{
			if(B[ e1+(d*y) ]!=0) cc=0;
			}
		if(cc>0)
			{
			for(y=1;y<3;y++)
				B[e1+(d*y)]=(sm>0?'K':'k');
			cc=( isCheck() ? 0 : 1 );
			for(y=1;y<3;y++)
				B[e1+(d*y)]=0;
			if(cc>0) addmove(e1,e1+(d<<1),0,x+1);
			}
		}
	};

	void copy_pos_only ( c1_chess *f )	// to see one possible move after current
	{
		int i=0;
		for(;i<64;i++) B[i]=(*f).B[i];
		sm=(*f).sm;
		for(i=0;i<4;i++) cs[i]=(*f).cs[i];
		ep=(*f).ep;
		gc = 0; mc = 0;
	};

	void moveg1(int ch) {	/* ch=0 first generate, ch=1 for faster checkmate detection only */

	gc = 0;
	for(int i=0;i<64;i++)
		{
		char c=B[i];
		if(c!=0)
			{
			if((sm>0) == isWhite(c))
				{
				c=toLower(c);
				X=iX(i); Y=iY(i);
				if(c=='p') pawngen(i);
				else if(c=='n') knightgen(i);
				else if(c=='b') bishopgen(i);
				else if(c=='r') rookgen(i);
				else if(c=='q') queengen(i);
				else if(c=='k') kinggen(i);
				}
			}
		}

    if(ch==0 && gc>0)
		{
		for(int k=0;k<gc;k++)
			{
			mkmove(k);
			if(isCheck())
				{
				c1_chess B2;
				B2.copy_pos_only(this);
				B2.moveg1(1);
				genml[k].c = ( B2.gc==0 ? '#' : '+' );
				}
			unmkmove();
			}
		}

	};

	void I2long( int k, char *s ) {       /* long notated move Ng1-f3 */

	int i=0;
	glist *o = &genml[k];
    if( (*o).k>0) setString( s, ((*o).k>1 ? c1_chess_OOO : c1_chess_OO ) );
	else
		{
		char c=toUpper( B[(*o).f] );
		if(c!='P') s[i++]=c;
		i2sq( (*o).f, &s[i] ); i+=2;
		s[i++] = ( (B[(*o).t]!=0 || ep==(*o).t) ? 'x':'-' );
		i2sq( (*o).t, &s[i] ); i+=2;
		if((*o).p!=0) { s[i++]='='; s[i++]=(*o).p; }
		if((*o).c!=0) s[i++]=(*o).c;
		s[i]=0;
		}
    };

	void I2short( int k, char *s ) {      /* short notated move Nf3 */

    char m[12];
	I2long(k,m);
	glist *o = &genml[k];
    if((*o).k>0) setString( s, m );
	else
		{
		char z=(*o).z;
		int ps=((z=='p' || z=='P')? 0 : 1);
		int q,w;
		for(q=0;q<2;q++)
		{
		int t=0;
		for(int i=0;i<gc;i++)
			{
			glist *u = &genml[i];
			int fo=(*o).f, to=(*o).t, fu=(*u).f, tu=(*u).t;

			if(B[fo]==B[fu])
			{
			if(q==0 && iY(fo)==iY(fu) && to==tu) t++;
			if(q==1 && iX(fo)==iX(fu) && (to==tu || (ps==0 && iX(fo)==iX(fo))) ) t++;
			}
			}
		if(t<2)
			{
			if(q==0 && ps>0) m[ps+0]=' ';
			if(q==1) m[ps+1]=' ';
			}
		}
		if(ps==0)
			{
			m[ps+1]=' ';
			if(iX((*o).f)==iX((*o).t)) m[ps+3]=' ';
			else m[ps+4]=' ';
			}
		m[ps+2]=' ';
		for(q=0,w=0; q<1 || s[q-1]!=0; w++) if( m[w]!=' ' ) s[q++]=m[w];
		}
    };

	void sPrintGenMoves(char *outp_str) {          /* prepares generated moves for printing*/
    int i=0;
	char *p=outp_str;
    char m[12];
    for(int k=0;k<gc;k++)
		{
		I2short /*I2long*/ (k,m);
		for(int q=0;q<1 || m[q-1]!=0;) p[i++] = m[q++];
		p[i-1] = ',';
		}
	p[i-1] = 0;
    };

	//------------------------- [ end of move generation ]------------------------

	//------------------------- [ check? checkmate? stalemate? ]------------------

	bool isCheck() {             /* detects check right now to king to move */

    int t=0,w,z,q,m,n,j;
	char p,c;

    for(int i=0;t==0 && i<64;i++)
	{
	p=B[i];
	if((p=='K' && sm>0) || (p=='k' && sm<0))
		{
         /* directional moves */
		int X=iX(i), Y=iY(i);

		for(w=-1;t==0 && w<2;w++)
		 for(z=-1;t==0 && z<2;z++)
		  for(q=1;t==0 && q<8;q++)
			{
			m=X+(q*w); n=Y+(q*z);
			if(m>=0 && m<8 && n>=0 && n<8)
				{
				j=(n<<3)+m;
				c=B[j];
				if( c!=0 && isBlack(c)==(sm>0) )
					{
					if((w==0 || z==0) && ((c=='r' && sm>0) || (c=='R' && sm<0))) t=1;
					else if((w!=0 && z!=0) && ((c=='b' && sm>0) || (c=='B' && sm<0))) t=1;
					else if((c=='q' && sm>0) || (c=='Q' && sm<0)) t=1;
					else if((q==1) && ((c=='k' && sm>0) || (c=='K' && sm<0))) t=1;
					else if((q==1) && (w!=0) && (z!=0) &&
						((z>0)==(sm>0)) && ((c=='p' && sm>0) || (c=='P' && sm<0))) t=1;
					}
				if(c!=0) q=8;
				}
			else q=8;
			}

		/*knight moves */
		for(w=-2;t==0 && w<3;w++)
		 for(z=-2;t==0 && z<3;z++)
		  if( iAbs(w)+iAbs(z)==3 )
			{
			m=X+w; n=Y+z;
			if(m>=0 && m<8 && n>=0 && n<8)
				{
				j=(n<<3)+m;
				c=B[j];
				if((c=='n' && sm>0) || (c=='N' && sm<0)) t=1;
				}
			}
		}
	}
	return (t>0);
    };

	bool isStaleMate() {          /* detects stalemate */

    bool r=false;
    if( !isCheck() )
		{
		moveGen();
		r=(gc==0);
		}
	return r;
    };

    bool isCheckMate() {          /* detects checkmate */

    bool r=false;
    if( isCheck() )
		{
		moveGen();
		r=(gc==0);
		}
    return r;
    };

	//------------------------- [ move and undo ]------------------

	void mkmove(int k) {            /* process move k from genlist created by movegen */

    glist *o = &genml[k];

	hlist *h = &hist[mc++];
	(*h).m.f = (*o).f; (*h).m.t = (*o).t; (*h).m.p = (*o).p;
	(*h).m.z = (*o).z; (*h).m.c = (*o).c;  (*h).m.k = (*o).k;

	char p=B[(*o).f];

    (*h).u = B[(*o).t];
    (*h).e = (ep==(*o).t && (p=='p'|| p=='P') ? (*o).t+(sm>0?-8:8) : -1);
    for(int i=0;i<4;i++)
		(*h).cs[i] = cs[i];

    (*h).ep = ep;

	B[(*o).t]=((*o).p!=0? (sm<0 ? toLower( (*o).p ) : (*o).p ) :p);
	B[(*o).f]=0;
    if((*h).e>=0) B[(*h).e]=0;

    int x=(sm>0?0:2), e1=(sm>0?4:60), h1=(sm>0?7:63), a1=(sm>0?0:56);
    if((*o).f==e1) { cs[0+x]=0; cs[1+x]=0; }
    if((*o).f==h1 || (*o).t==h1) cs[0+x]=0;
    if((*o).f==a1 || (*o).t==a1) cs[1+x]=0;
    if((*o).k>0)
		{
		if((*o).k==1) { B[h1-2]=B[h1]; B[h1]=0; cs[0+x]=0; }
		else { B[a1+3]=B[a1]; B[a1]=0; cs[1+x]=0; }
		}

	ep=-1;
    if(p=='p'|| p=='P')
		{
		int d=((*o).t) - ((*o).f);
		if(iAbs(d)==16) ep=((*o).f) + (d>>1);
		}

	sm=-sm;
    };


	void unmkmove() {          /* process unmake last move */

    hlist *h = &hist[--mc];
	glist *o = &( (*h).m );

	B[(*o).f]=(*o).z;
	B[(*o).t]=(*h).u;
    if((*h).e>=0) B[(*h).e]=(sm>0?'P':'p');
    if((*o).k>0)
		{
		if((*o).k==1) { B[(*o).f+3]=B[(*o).f+1]; B[(*o).f+1]=0; }
		else { B[(*o).f-4]=B[(*o).f-1]; B[(*o).f-1]=0; }
		}
	for(int i=0;i<4;i++)
		cs[i] = (*h).cs[i];
	ep=(*h).ep;
	sm=-sm;
    };

    int uci2I( char *u ) {         /* locates k in genlist by given uci move e2e4 e1g1 a7a8q */

    int f=sq2i( &u[0] ), t=sq2i( &u[2] ), r=-1;
    char p = ( isAlpha(u[4]) ? toUpper(u[4]) : 0);
    for(int i=0;r<0 && i<gc;i++)
		{
		glist *o = &genml[i];
		if((*o).f==f && (*o).t==t && (p==0 || (*o).p==p)) r=i;
		}
    return r;
    };

	void m2uci( hlist *h, char *s ) {
		i2sq( (*h).m.f, &s[0] );
		i2sq( (*h).m.t, &s[2] );
		s[4] = toLower( (*h).m.p );
		s[5] = 0;
    };

	void g2uci( glist *o, char *s ) {
		i2sq( (*o).f, &s[0] );
		i2sq( (*o).t, &s[2] );
		s[4] = toLower( (*o).p );
		s[5] = 0;
    };

	void uciMoves( char *us ) {     /* processes list of uci moves, fxmpl. "e2e4 e7e5 g1f3\0" */

	int i=0, j=0;
	char p = 0;
	for(;i<1 || p!=0;i++)
		{
		p = us[i];
		if( ((p==' ') || (p==0)) && (i>j) )
			{
			moveGen();
			int k=uci2I( &us[j] );
			if(k>=0) mkmove(k);
			j = i+1;
			}
		}
	moveGen();
	};

	void hist2uci( char *s ) {       /* history to uci move list */

    int i=0;
    for(int k=0;k<mc;k++)
		{
		if(k>0) s[i++]=' ';
		m2uci( &hist[k], &s[i] );
		for(;s[i]!=0;i++);
		}
    };

	void hist2pgn( char *s ) {       /* history to pgn of short notated moves */

	int i=0,r=0;
	c1_chess B2;
	B2.setFEN( c1_chess_sFEN );
    for(int n=0; r>=0 && n<mc; n++)
		{
		hlist *h = &hist[n];
		B2.moveGen();
		r=-1;
		for(int k=0;r<0 && k<B2.gc;k++)
			{
			glist *o = &B2.genml[k];
			if((*o).f==(*h).m.f && (*o).t==(*h).m.t && ((*h).m.p==0 || (*o).p==(*h).m.p)) r=k;
			}
		if(r>=0)
			{
			if(B2.sm>0)
				{
				if(n>0) s[i++]=' ';
				toString( (B2.mc>>1)+1, &s[i] );
				for(;s[i]!=0;i++);
				s[i++]='.';
				}
			else s[i++]=' ';
			B2.I2short(r,&s[i]);
			for(;s[i]!=0;i++);
			B2.mkmove(r);
			}
		}
    };

	int movestr2I( char *s ) {      /* finds index of chess move from given string, a dumb way */

    int k=-1;
	int fx=-1, fy=-1, tx=-1, ty=-1;
	char p=0,c;
	for(int j=0;s[j]!=0;j++)
		{
		c=s[j];
		if(j==0) p=( indexOfchar(c1_chess_m,c)<0 ? 'P': c );
		if(c>='a' && c<='h')
			{
			if(fx<0) fx=c-'a';
			else if(tx<0) tx=c-'a';
			}
		if(c>='0' && c<='8')
			{
			if(fy<0) fy=c-'1';
			else if(ty<0) ty=c-'1';
			}
		}
	if(tx<0 && fx>=0) { tx = fx; fx=-1; }
	if(ty<0 && fy>=0) { ty = fy; fy=-1; }

    for(int i=0;k<0 && i<gc;i++)
		{
		glist *o = &genml[i];

		if( ((*o).k==1 && ((s[2]=='O'||s[2]=='0')&&(s[3]!='-'))) ||
			((*o).k==2 && ((s[4]=='O'||s[4]=='0')&&(s[3]=='-'))) ||
			((p == toUpper( B[(*o).f] )) &&
			(fx<0 || fx==iX((*o).f)) &&
			(fy<0 || fy==iY((*o).f)) &&
			(tx<0 || tx==iX((*o).t)) &&
			(ty<0 || ty==iY((*o).t))) ) k=i;
		}
    return k;
	};

	c1_chess() { /*setFEN( c1_chess_sFEN );*/ };	// on object creation

	~c1_chess() {};	// on object deletion

};				// ------------------[end of c1_chess class]

/*
#include <stdio.h>

void c1_samples() {

	//buffers for printing, include stdio.h
	char board[100], possible[256], fen[100], movesuci[2000], movespgn[2000];
	c1_chess B;
	B.setFEN( c1_chess_sFEN );

	//B.uciMoves( "e2e4 c7c5 g2g3 b7b6 g1f3 c8b7 f1g2 b8c6 e1g1 d7d6 g1h1 d8d7 a2a3 e8c8");
	//B.uciMoves( "e2e4 a7a6 e4e5 f7f5 e5f6");
	//B.uciMoves( "b2b3 a7a5 b3b4 a5b4 a2a3 b4a3 c1b2 a3b2 h2h3 b2a1n h3h4 a1b3");
	//B.uciMoves( "g2g4 e7e5 f2f4 d8h4" );

	//B.uciMoves( "e2e4 c7c5 g2g3 b7b6 g1f3 c8b7 f1g2 b8c6");
	B.uciMoves( "e2e4" );

	B.hist2uci( movesuci );
	printf("Moves made in UCI standard:\n%s\n", movesuci );
	B.hist2pgn( movespgn );
	printf("Moves made in pgn-notation:\n%s\n", movespgn );

	if(B.isCheckMate()) printf("Checkmate!\n");
	if(B.isStaleMate()) printf("Stalemate!\n");
	//B.unmkmove();
	//B.uciMoves( "h7h6");

	B.moveGen();

	B.sPrintBoard( board );
	printf("Chess board now:\n%s\n", board );

	B.getFEN( fen );
	printf( "FEN of position now:\n%s\n", fen );

	B.sPrintGenMoves( possible );
	printf("Moves possible now:\n%s\n", possible );

	int k = B.movestr2I("O-O");
	printf( "Move 0-0 element is %d\n", k );
	getchar();
};
*/

#endif  /* _INC_C1_CHESS */
