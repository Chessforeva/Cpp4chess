#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

/*

	Chess Magic Numbers generator for PAWNS


	Finds uint64 constants for pawns,
	to calculate legal chess moves,
	according to the occupancy of pieces on board.

	Prepares C-code ready constants too.

Compiler: gcc

*/

#define U64 unsigned long long
#define U32 unsigned int
#define U16 unsigned short
#define U8 unsigned char

#define MegaByte (1LL<<20)

// if user assigned
int usr_b_r;
int usr_bits;

int bitCnt = 4;     // fixed, or can be 8,...

char outpFile[100]; // write findings log to

FILE *outp;

char buffer[100], bufSq[4];

// Prepare table of chess moves

U64 Bo2, Board;
int square;
int b_w=0;        // 0 white/ 1-black

U64 masks [64][2][1<<16][2];
U64 result[1<<16];

// Contains length of occupancy table
int TbLen[64][2];

int legalck = 0;      // Calculate: 0- allmoves, 1-legalmoves

U8 BoSet(U8 sq, U8 capt) {
    U8 b = 0;
    U64 B = (1LL<<sq);
    if(legalck) {
        if(Board & B) b = (U8)1;
        if(capt==b) Bo2 |= B;
        }
    else { Board|=B; }
    return b;
}

void gen_pawnmoves() {

    int V=(square>>3),H=(square&7);
    if(V>0 && V<7) {
        if(b_w) V--; else V++;

        U8 sq = (V<<3)|H;
        U8 f = BoSet(sq,0);
        if(H>0) BoSet(sq-1,1);
        if(H<7) BoSet(sq+1,1);
        if(!f) {
            if(b_w) {
                if(V==5) BoSet(sq-8,0);
                }
            else {
                if(V==2) BoSet(sq+8,0);
                }
            }
    }
}

// Does all the permutations for the current square and Board
void Permutate() {

	int bits[64];		    	// This will contain square numbers
	int n=0,sq=0;
	for(;sq<64;sq++) {
		if( Board & (1LL<<sq) ) bits[n++]=sq;
		}

	int LEN = (1<<n);			// length of permutations table
	int T=0;

	for(int i=0;i<LEN;i++) {	// go through all the cases

        Board = 0LL;

        for(int j=0;j<n;j++)	// scan as bits
		{
			sq = bits[j];
                // set bit on Board
            if( i & (1<<j) ) Board |= (1LL<<sq);
		}

        // now Board contains occupancy

        Bo2 = 0LL;

        gen_pawnmoves();      // find legal moves for square, put in Bo2

        masks[square][b_w][T][0] = Board;
        masks[square][b_w][T][1] = Bo2;       // legalmoves
        T++;
	}

	TbLen[square][b_w]=T;

}


void prepare_tables() {

    for(square=0; square<64; square++) {
        for(b_w=0;b_w<2;b_w++) {
            legalck = 0;
            Board = 0LL;
            gen_pawnmoves();
            legalck = 1;
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

int RANDOMIZER = 11;
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


// find magic number
void find_Magics_pawns() {

    square2at(bufSq, square);

    int LEN=TbLen[square][b_w];

    int found = 0;

    printf("sq# %d of 63\n",square);

    for(;!found;) {

     printf("searching pawns: sq=%s %s bits=%d\n",
            bufSq, (b_w ? "black" : "white"), bitCnt);

     U64 toN = (1LL<<22);
     for(U64 N=0LL; N!=toN; N++) {

        memset( result, 0, sizeof(U64)<<bitCnt);

        // find the magic number!

        U64 Magic = rand64();

        int good=1;
        for(int w=0; w<LEN; w++) {
            Board = masks[square][b_w][w][0];
            Bo2 = masks[square][b_w][w][1];

            U64 mult = (Board * Magic);

            U64 shft = mult>>(64-bitCnt);
            U32 index =  (U32)shft;

            U64 *q=&result[index];

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
             fprintf(outp,"%sLL,%c", buffer, (((square&7)==7)?13:32));

             break;
        }
    }
    }

    if(!found) printf("Error!\n");

}

void find_Magics() {

    for(b_w=0; b_w<2; b_w++) {

        fprintf( outp, "// Pawn Magics for %s\n", (b_w ? "black" : "white") );

        for(square=0; square<64; square++) {

            find_Magics_pawns();
        }
    }
    fprintf(outp,"\n// BitCount=%d  >>%d\n", bitCnt, 64-bitCnt);

}

// cmd settings
void usr_args(char *s) {

    if(*s=='r') RANDOMIZER = atoi(++s);
    else if(*s=='b') {
            usr_bits = atoi(++s);
            if(usr_bits>=4) {
                bitCnt = usr_bits;
            }
        }
}

void welcome() {

    printf("Chess Magics for pawns generator to lookup legal moves\n");
    printf(" (SquareOccupancy&SqMovesMask  * Magic) >> (64-bits) = index to Table\n\n");
    printf("usage: magicgen [options-list]\n options:\n");
    printf("  rN  - randomize number N\n");
    printf("  bN  - bits count N=4,...\n");
    printf("\n\nThere is no big reason in pawn magics, but it works anyway.\n");
}

int main(int argc, char *argv[])
{
    if(argc<2) {
        welcome();
        printf("\nPress Enter to start anyway, or Esc key to close...\n");

        int c = getch();
        if(c==27) return(0);
    }

    usr_b_r = -1;
    usr_bits = -1;

    for(int i=1; i<argc; i++) usr_args(argv[i]);

    prepare_tables();

    sprintf(outpFile,"pawnmagicgen%d.txt",RANDOMIZER);
    outp = fopen(outpFile,"w");

    find_Magics();      // search our magic numbers

    fclose(outp);
    printf("\nOutput file ready: %s\n", outpFile);

    printf("\nPress a key to close...\n");
    getch();

    return(0);
}
