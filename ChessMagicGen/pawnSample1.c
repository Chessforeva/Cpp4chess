#include <stdio.h>
#include <stdlib.h>

/*

    Chess Pawn Magics usage sample

Compiler: gcc

*/

#define U64 unsigned long long
#define U32 unsigned int
#define U16 unsigned short
#define U8 unsigned char


// pre-generated constants, pawnMagicGen created

static const U64 PawnMagicsWhite[] = {
// Pawn Magics for white
0x6CFFD877D338779FLL, 0x2D0386CD9AF51A16LL, 0xDCB0008790284C44LL, 0x333C8EF2E5D906EFLL, 0x1E6DA07F86EC13EELL, 0x94D11367A0FD5CADLL, 0x9709B0EB6FCD810FLL, 0x3D8E90C4BB4005ADLL,
0xA6A7BE94DA1F1137LL, 0x1B8630C2852BB09FLL, 0x9340708591EE1E37LL, 0xE91CBC5191A3A197LL, 0xAC4EE4310657296BLL, 0xDEE4E205AA98C729LL, 0xA8ABD5055AF4DB4ELL, 0x46BA9C7AE7242EB6LL,
0x6252531829102CD7LL, 0xB176F36A437EFFBCLL, 0xE238975A85BBD30ELL, 0x0F4879719F5BBD48LL, 0x391BABD74587079BLL, 0x9AA959C5C38475F9LL, 0x47EEF42CFD4B9CBELL, 0xDD2625A6DEB18CF0LL,
0x64D008AB74486795LL, 0xA549FB872086711CLL, 0xF78E16E50D1B5A89LL, 0x62652CC3E7B2F26CLL, 0xE6FF22805DA4DE30LL, 0x7461A0774C4AA2EBLL, 0xA313AE58FE92BF56LL, 0xBAB572F8AD1F9EB5LL,
0x94294BB147BB4DD1LL, 0x8FE9F782F3628071LL, 0x502A4DFC65BB2CD7LL, 0x0B17D7D020C71381LL, 0xDE1BC1108396147CLL, 0x0A30E5A50BD76BF5LL, 0xCFB908AD73376570LL, 0xFA7695CF29EC6255LL,
0x52098B6D5758118BLL, 0xBDEF4CCC69BEE6EDLL, 0xCCAA2F891E29F084LL, 0x82E975DC643571B7LL, 0xB3AE62F29104E480LL, 0xC0D3E6819B2CD37BLL, 0xCAEA117F7672BB6DLL, 0x429332B81C18CA21LL,
0x9CB017BC75E21E8BLL, 0x0EF099078FCFACA1LL, 0x4F53FDFEEA51FFD8LL, 0x72786340CF25CF09LL, 0xE42C34931F0857D6LL, 0x5F7333B802F85437LL, 0x857BC8DD1CCE839DLL, 0x2F78D25830845451LL,
0xFAB2742802D61FD6LL, 0xF209C3C525FC4C59LL, 0xBA341C4C9C969AFCLL, 0x463FC1E7ADD33298LL, 0x8CF30B7B3FCD7B03LL, 0xEDB6E8803AB36382LL, 0xA7FC85869CE3A9BFLL, 0xB3874247C2457A49LL,
};

static const U64 PawnMagicsBlack[] = {
// Pawn Magics for black
0x34B5385467B0144ELL, 0x681F851A088CC9B4LL, 0x7B89F324FE8CF26ALL, 0x5BF0C6B8DE51A4A0LL, 0x01AA7F90EA79B9BELL, 0x56E5D76FD603F447LL, 0x753FD6C0F78E9120LL, 0x07BD569FEE94CB02LL,
0xBE258FF73E57AC8FLL, 0x5E4E68981294BC7ALL, 0x5A062E1BF8D08922LL, 0xC80D9383740DEA61LL, 0x23695C2D0F721BDBLL, 0x0977D01E826FC653LL, 0x90E511DF9A108B39LL, 0x41A803CDB5471786LL,
0x335AE3B15F5240DELL, 0x561F57E2309575B9LL, 0x3B11DC8FEB1ACF22LL, 0xAEAF00C7EF4D0E39LL, 0x81D6D3B4AEF63ABFLL, 0xBF115E265089C862LL, 0x4FC3107939FE3E66LL, 0x1798603F58B091DFLL,
0x6E27B57B81A9D2C3LL, 0x37AB512383E9DB9FLL, 0xBBF8B5BD094F8341LL, 0xC207384103D41905LL, 0xF995FA59DC45BA79LL, 0x50295899DBB069C6LL, 0x04C2C62280E54351LL, 0xC7940130BD5768EBLL,
0x337C373FBEE06777LL, 0xFB43A615E3F737A6LL, 0xA7B268D08CDFF0C2LL, 0xD71F68F86D96D418LL, 0x77EBB0A6233650BELL, 0x75B14A4A00012E94LL, 0x5AA5ECD91FD515C3LL, 0x4A931E83788D4547LL,
0xFF25A7A3640D3D1DLL, 0xCEE6E5AF3A655B1ELL, 0x4FA319769DB79349LL, 0x433A9EBBCEE081AALL, 0x97417A5A444776A4LL, 0x577170E23E38E6D8LL, 0x1FC0E04A1D272ECFLL, 0xAF7433E56B8CAE83LL,
0xE1B6AFBA7C1824A7LL, 0xA453ACBE41B15D47LL, 0xB4618E85E118C676LL, 0x0F100C3916985A57LL, 0xA0E42AA06F344462LL, 0x5DC8C067518E1929LL, 0xE8BB6960D5C30AE2LL, 0x53D67A580A7EDC08LL,
0x5FBDF0D194E122DALL, 0x592BEB15A5348A40LL, 0xF7E6A95D44B5559ELL, 0x198D8621B1AE3F9ALL, 0x17C09FEFDFEDC9A9LL, 0xCF90010F623D1DAELL, 0x5AD8FDAA5A792633LL, 0x05C71892C521DFCBLL,
};

// BitCount=4  >>60

// arrays to prepare

U64 PawnMaskWhite[64];
U64 PawnMaskBlack[64];

    // The lookup table of legal moves
U64 PawnWhiteLegalsTable[64][1<<4];
U64 PawnBlackLegalsTable[64][1<<4];

#define getWhitePawnMove(square,occupancy) PawnWhiteLegalsTable[square][((occupancy&PawnMaskWhite[square])*PawnMagicsWhite[square])>>60]

#define getBlackPawnMove(square,occupancy) PawnBlackLegalsTable[square][((occupancy&PawnMaskBlack[square])*PawnMagicsBlack[square])>>60]

// ---------------------- prepares arrays
//
// variables

U64 Bo1,Bo2;    // boards
int SqI;        // square to prepare
int b_w;        // 0-white/1-black pawns

int legalck = 0;      // Calculate: 0- all, 1-legalmoves

U8 BoSet(U8 sq, U8 capt) {
    U8 b = 0;
    U64 B = (1LL<<sq);
    if(legalck) {
        if(Bo1 & B) b = (U8)1;
        if(capt==b) Bo2 |= B;
        }
    else { Bo1|=B; }
    return b;
}

void gen_pawnmoves() {

    int V=(SqI>>3),H=(SqI&7);
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

// Does all the permutations for the current square and Bo1 board
void Permutate() {

	int bits[64];		    	// This will contain square numbers
	int n=0,sq=0;
	for(;sq<64;sq++) {
		if( Bo1 & (1LL<<sq) ) bits[n++]=sq;
		}

	int LEN = (1<<n);			// length of permutations table
	for(int i=0;i<LEN;i++) {	// go through all the cases

        Bo1 = 0LL;
        for(int j=0;j<n;j++)	// scan as bits
		{
            if(i&(1<<j)) Bo1|=(1LL<<bits[j]);
		}
        // now Bo1 contains occupancy

        U64 mult = Bo1 * (b_w ? PawnMagicsBlack[SqI] : PawnMagicsWhite[SqI]);
        U8 index = (U8)(mult >> 60);

        Bo2 = 0LL;
        gen_pawnmoves();      // find legal moves for square, put in Bo2

        if(b_w) PawnBlackLegalsTable[SqI][index] = Bo2;
        else PawnWhiteLegalsTable[SqI][index] = Bo2;
	}
}

void prepare_tables() {

    for(SqI=0; SqI<64; SqI++) {
        for(b_w=0;b_w<2;b_w++) {
            legalck = 0;
            Bo1 = 0LL;
            gen_pawnmoves();
            if(b_w) PawnMaskBlack[SqI] = Bo1;
            else PawnMaskWhite[SqI] = Bo1;
            legalck = 1;
            Permutate();
        }
    }
}


// ----------------------

void displayBitboard(U64 N) {
    for(int V=7;V>=0;V--) {
        for(int H=0;H<=7;H++) {
            int i=(V<<3)|H;
            printf("%c", ((N&(1LL<<i))?'X':'o'));
        }
        printf("\n");
    }
    printf("\n");
}


int main(int argc, char *argv[])
{
    prepare_tables();

        // Occupancy, put any pieces on A3 A4 E8
    U64 Occ =(1LL<<16)|(1LL<<24)|(1LL<<60);

    U64 wpawn_A2 = getWhitePawnMove(8,Occ);
    displayBitboard(wpawn_A2);
    U64 wpawn_B3 = getWhitePawnMove(17,Occ);
    displayBitboard(wpawn_B3);
    U64 wpawn_C2 = getWhitePawnMove(10,Occ);
    displayBitboard(wpawn_C2);
    U64 wpawn_D7 = getWhitePawnMove(51,Occ);
    displayBitboard(wpawn_D7);
    U64 bpawn_H7 = getBlackPawnMove(55,Occ);
    displayBitboard(bpawn_H7);

    U64 EnPassant_G3 = (1LL<<22);
    Occ |=EnPassant_G3;
    U64 bpawn_F4 = getBlackPawnMove(29,Occ);
    displayBitboard(bpawn_F4);

    printf("\nPress a key to close...\n");
    getch();

    return(0);
}

/*
Displays:

oooooooo
oooooooo
oooooooo
oooooooo
oooooooo
oooooooo
oooooooo

oooooooo
oooooooo
oooooooo
oooooooo
XXoooooo
oooooooo
oooooooo
oooooooo

oooooooo
oooooooo
oooooooo
oooooooo
ooXooooo
ooXooooo
oooooooo
oooooooo

oooXXooo
oooooooo
oooooooo
oooooooo
oooooooo
oooooooo
oooooooo
oooooooo

oooooooo
oooooooo
oooooooX
oooooooX
oooooooo
oooooooo
oooooooo
oooooooo

oooooooo
oooooooo
oooooooo
oooooooo
oooooooo
oooooXXo
oooooooo
oooooooo

*/
