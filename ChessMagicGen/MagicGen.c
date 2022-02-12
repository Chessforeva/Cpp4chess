#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

/*

	Chess Magic Numbers generator

	Finds uint64 constants for bishops and rooks,
	to calculate legal chess moves,
	depeending on occupancy of pieces on board.

	Prepares C-code ready constants too.

	The simple version, finds anyways.
	v1.0 by Chessforeva, feb.2020

Compiler: gcc

*/

#define U64 unsigned long long
#define U32 unsigned int

#define MegaByte (1LL<<20)

// if user assigned
int usr_square;
int usr_b_r;
int usr_bits;
int usr_bits_const;
int usr_searchtime;
int usr_B2G7;

char outpFile[100]; // write findings log to

FILE *outp;

char buffer[100], bufSq[4];
char bitcntBuf[1024];

// Prepare table of chess moves

U64 Bo2, Board;
int square;
int b_r=0;        // 1-bishops, 0-rooks

/*
 U64 masks [64][2][16384][2];

        Sadly, we should use allocate instead.

 U64 numbers as masks:
 [] for 64 squares...
 [] rook or bishop...
 [] on e4 can move 14 squares (1<<14=16384)...
 [] for each occupancy mask[0] -> move to mask[1]

 size = 32Mb    (in reality need 22Mb)

*/
void *Masks_memory; // allocated memory pointer

U64 *Masks( int square, int b_r, int n, int m ) {
    void *p = Masks_memory;
    p+=((b_r<<21)<<3);
    p+=((square<<15)<<3);
    p+=((n<<1)<<3);
    p+=(m<<3);
    return p;
}

// Contains length of occupancy table for all squares, for 1-bishops/0-rooks
int TbLen[64][2];

U64 *TB2;   // table to project >> results

int legalck = 0;      // Calculate: 0- rays, 1-legalmoves

void dir(int dv, int dh) {
	int V=(square>>3),H=(square&7);
	V+=dv; H+=dh;
	while( (V>=0 && V<8) && (H>=0 && H<8) ) {
		int sq = (V<<3)|H;
		if(legalck) {
            Bo2 |= (1LL<<sq);
            if( Board & (1LL<<sq) ) return;
		}
		else { Board|=(1LL<<sq); }
		V+=dv; H+=dh;
		}
	}

void gen2dir() {

    if(legalck) Bo2=0LL;
    else Board=0LL;

    if(b_r) {   //bishops
        dir(-1,-1); dir(+1,-1); dir(-1,+1); dir(+1,+1);
    }
    else {  // rooks
        dir(-1,0); dir(+1,0); dir(0,+1); dir(0,-1);
    }
}


// Does all the permutations for the current square and Board
void Permutate() {

	int inner = ( (1LL<<square) & 0x007E7E7E7E7E7E00LL );

	int bits[64];		    	// This will contain square numbers
	int n=0,sq=0;
	for(;sq<64;sq++) {
		if( Board & (1LL<<sq) ) bits[n++]=sq;
		}

	int LEN = (1<<n);			// length of permutations table
	int T=0;
	U64 *p = Masks(square, b_r, T, 0);

	for(int i=0;i<LEN;i++) {	// go through all the cases

        Board = 0LL;

        for(int j=0;j<n;j++)	// scan as bits
		{
			sq = bits[j];
                // set bit on Board
            if( i & (1<<j) ) Board |= (1LL<<sq);
		}

        // now Board contains occupancy

        gen2dir();      // find legal moves for square, put in Bo2

        // if to ignore occupancies on A1-H1,A8-H1,A1-A8,H1-H8
        if(usr_B2G7 && inner) Board&=0x007E7E7E7E7E7E00LL;

        *(p++) = Board;
        *(p++) = Bo2;       // legalmoves
        T++;
	}

	TbLen[square][b_r]=T;

}


void prepare_tables() {

    for(square=0; square<64; square++) {
        for(b_r=0; b_r<2; b_r++) {
            legalck=0;
            gen2dir();
            legalck=1;
            Permutate();
        }
    }
}

void U64toString(char *buf, U64 a) {
  U32 HiByte = (a>>32)&0xFFFFFFFF;
  U32 LoByte = (a&0xFFFFFFFF);
  char buf_Hi[20], buf_Lo[20], l_hi, l_lo;
  sprintf(buf_Hi,"%X", HiByte); l_hi = strlen(buf_Hi);
  sprintf(buf_Lo,"%X", LoByte); l_lo = strlen(buf_Lo);
  sprintf(buf,"0x0000000000000000");
  memcpy(buf+10-l_hi, buf_Hi, l_hi);
  memcpy(buf+18-l_lo, buf_Lo, l_lo);
}

int RANDOMIZER = 1;
U32 r_x = 30903, r_y = 30903, r_z = 30903, r_w = 30903, r_carry = 0;
unsigned int rand32() {
    U32 t;
    r_x = r_x * 69069 + RANDOMIZER;   r_y ^= r_y << 13;
    r_y ^= r_y >> 17;   r_y ^= r_y << 5;   t = (r_w << 1) + r_z + r_carry;
    r_carry = ((r_z >> 2) + (r_w >> 3) + (r_carry >> 2)) >> 30;
    r_z = r_w;   r_w = t;   return r_x + r_y + r_w;
}

U64 rand64() {
    U64 rr=rand32(); rr<<=32; rr|=rand32();
    return rr;
}

void square2at(char *s, int sq) {
    int V=sq>>3, H=sq&7;
    s[0]=('A'+H);
    s[1]=('1'+V);
    s[2]=0;
}


// find magic number for square, bishop or rook
void find_SqBR() {

    square2at(bufSq, square);

    U64 *table = Masks(square, b_r, 0, 0);

    int LEN=TbLen[square][b_r];

    int found = 0;
    int bitCnt = (usr_bits_const ? usr_bits : 3); // will increase

    printf("sq# %d of 63\n",square);

    for(;!found;) {

     if(bitCnt<16) bitCnt++;    // do again, but limit below (1<<16)

     if(usr_bits_const || (usr_bits>=0 && bitCnt>usr_bits)) bitCnt=usr_bits;

     printf("searching: square=%s %s bits=%d\n", bufSq,
                (b_r ? "bishops" : "rooks"), bitCnt);

     U64 toN = (1LL<<(15+usr_searchtime));
     for(U64 N=0LL; N!=toN; N++) {

        U64 *z=TB2;
        int k=(1<<bitCnt);      // clear previous search
        while(k--) *(z++)=0;


        // find the magic number!

        U64 Magic = rand64();

        U64 *p=table;

        int good=1;
        for(int w=0; w<LEN; w++) {
            Board = *(p++);
            Bo2 = *(p++);

            // Funky fast working sample for strange chess engines :)
            //U64 mult = ((~Board)*(Magic^Board));

            U64 mult = (Board * Magic);

            U64 shft = mult>>(64-bitCnt);
            U32 index =  (U32)shft;

            U64 *q=&TB2[index];

            if( !(*q) ) *q = Bo2;
            else if( *q != Bo2 ) {
                good=0;
                break;
            }
        }

        if(good){
             found=1;
             U64toString( buffer, Magic );
             printf("found magic: %s\n",buffer);
             char *s=bitcntBuf;
             s+=strlen(bitcntBuf);
             sprintf(s,"%d,",bitCnt);
             fprintf(outp,"%sLL /*%s >>%d*/,%c",
                     buffer,bufSq,64-bitCnt, (((square&7)==7)?13:32));

             break;
        }
     }
    }

    if(!found) printf("Error!\n");

}

void find_Magics() {

    for(b_r=1; b_r>=0; b_r--)
        if(usr_b_r<0 || usr_b_r==b_r) {

        fprintf( outp, "// Magics for %s\n", (b_r ? "bishops" : "rooks") );

        for(square=0; square<64; square++) {

            if(usr_square<0 || usr_square==square) {
                find_SqBR();
            }
        }
        fprintf( outp, "\n// Bits\n%s\n",bitcntBuf);

    }
}

// cmd settings
void usr_args(char *s) {

    if(*s=='r') RANDOMIZER = atoi(++s);
    else if(*s=='s') usr_square = atoi(++s);
    else if(*s=='B') usr_b_r = 1;
    else if(*s=='R') usr_b_r = 0;
    else if(*s=='b') {
            if(*(s+1)=='c') { usr_bits_const = 1; s++; }
            usr_bits = atoi(++s);
        }
    else if(*s=='T') usr_searchtime = atoi(++s);
    else if(*s=='f') usr_B2G7 = 1;
}

void welcome() {

    printf("A simple Chess Magics generator to lookup bishop, rook legal moves\n");
    printf(" (SquareOccupancy&RaysMask  * Magic) >> (64-bits) = index to Table\n\n");
    printf("usage: magicgen [options-list]\n options:\n");
    printf("  rN  - randomize number N\n");
    printf("  B   - bishops only\n");
    printf("  R   - rooks only\n");
    printf("  sN  - square N only (0-63)\n");
    printf("  bN  - bits (1<<N),or below 16 only\n");
    printf("  bcN - constant bits (1<<N) for all\n");
    printf("  TN  - searchtime increase N=2,3,..\n");
    printf("  f   - B2-G7 filter, otherwise all\n");
    printf("\nsamples:\nmagicgen r9\nmagicgen s36 B\nmagicgen R s63 b15\n");
    printf("\nv1.0, by Chessforeva, feb.2022\n");
}

int main(int argc, char *argv[])
{
    if(argc<2) {
        welcome();
        printf("\nPress Enter to start anyway, or Esc key to close...\n");

        int c = getch();
        if(c==27) return(0);
    }

    usr_square = -1;
    usr_b_r = -1;
    usr_bits = -1;
    usr_bits_const = 0;
    usr_searchtime = 0;
    usr_B2G7 = 0;

    for(int i=1; i<argc; i++) usr_args(argv[i]);

    Masks_memory = malloc(60*MegaByte);
    TB2 = Masks_memory +(40*MegaByte);

    prepare_tables();

    sprintf(outpFile,"magicgen%d.txt",RANDOMIZER);
    printf("Writing output to file: %s\n", outpFile);
    outp = fopen(outpFile,"w");

    find_Magics();      // search our magic numbers

    fclose(outp);

    free(Masks_memory);

    printf("\nPress a key to close...\n");
    getch();

    return(0);
}
