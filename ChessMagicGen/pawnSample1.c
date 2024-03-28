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
0x6CFFD8BCD3813025LL, 0xD124186042012412LL, 0x47D1CB98DC98C626LL, 0x9960A3319A0370A7LL, 0xF57AA11C65FA8AECLL, 0x666F4312B4C037E1LL, 0x2A68DDD475354CE9LL, 0xA2E0E09B5C0EE01DLL,
0xE2791197D71749EFLL, 0x8041E0F89AAFD8F1LL, 0xC4E86860077C1E6BLL, 0xF21CA852246F6593LL, 0x2494C41F7D900347LL, 0x6674B60D9A900A65LL, 0xBF709C4103B32D43LL, 0x7E7C73602722894BLL,
0x753A0BE05B7409DELL, 0xFAD801465CEBC598LL, 0x75491B383EB986CELL, 0x160836D6E157ADEFLL, 0x4293770388AFED51LL, 0xFC00457C53A7A8CCLL, 0x933A416F7FA1BEFBLL, 0x3A6DD28A944A89A4LL,
0x2B32C9EDE5D36625LL, 0x598807C071807290LL, 0x51F67A2B4FCD94BELL, 0x30468DEB19182555LL, 0x52502DB0AE2A8854LL, 0xC766DFDBFA0EE2B9LL, 0x138F57656ADC76B5LL, 0x096D4FAF27875385LL,
0xF6C08BB990BB080DLL, 0xEA57DFA529A2C4FELL, 0x5159A8BE8414092BLL, 0x2F7650DCB9C84A47LL, 0xCB9D650D37ED81BDLL, 0xC98D4D7C35F95288LL, 0x9AFC4746431B8281LL, 0xAC7E6366E9AEC902LL,
0xE50204A876F2910CLL, 0xDCDA62861C5E9DF2LL, 0xB3E7DCF35BC01861LL, 0xDF5BC8E4CB5AC47DLL, 0x985E5D3573EE5DA0LL, 0x3F622C3D95F8D13FLL, 0x9CE96D4D9B47C5E4LL, 0x47C859E4A93F3B86LL,
0x8B9EE7CFD60C44A9LL, 0x48CD5724C7FD3EF1LL, 0xEBE321BE5396A4C7LL, 0x781EF3D4A0CBADCFLL, 0x74B6985859DD0E89LL, 0xAB099F864255121ALL, 0x4B32732AFF896361LL, 0x3C4755EB59E83C65LL,
0x0D08957E08521372LL, 0xFDF19B39AD6BC6B2LL, 0xF4D721133D240DA9LL, 0x8C3481A7C4E3D2BFLL, 0x06CFF88D337EFF50LL, 0x1D7AC438742547ACLL, 0xCDDBF6FB9AA9832DLL, 0x6B9E69C6883FA50BLL,
};

static const U64 PawnMagicsBlack[] = {
// Pawn Magics for black
0x3BE7FC46927CF82FLL, 0x4AAC763FA470CE98LL, 0x0FF62BEE8FBD5F76LL, 0xB2589E054CC0E43CLL, 0x0D88BB0FD4C3F231LL, 0x20F213C013DD9FEALL, 0xD0B2B77848F0A66DLL, 0xEB14C34F54F2D57ALL,
0x19D7C8BAC44A9A6FLL, 0x9E090BB094DEE86DLL, 0xBA473B4EB44158E8LL, 0xA844702091EB8067LL, 0x23EC2A26EF0BE169LL, 0x4EF88CEE76CE3FC9LL, 0x0F17C5CCB32A834DLL, 0xC4D1814E4C75D21BLL,
0x03976178DFD226F3LL, 0xB117C3E4260AEE44LL, 0xD9A5F62FFCC2D7EELL, 0x4391D38271B166B8LL, 0xE843C4FF9888ABA1LL, 0x121B595647031BBDLL, 0x2D28D420366E2A36LL, 0xC932750F8F70B654LL,
0x021A35782B70B577LL, 0x22484DC1D25713C4LL, 0x0E921CE58FA7D771LL, 0xA4454E470A24AC31LL, 0xA1A0D233233A1672LL, 0xECBB9B320FD66DD3LL, 0xFF52D3BAFC80874BLL, 0x016A9AD5C6274070LL,
0x6761EA34FECAFF77LL, 0xCB99473867611CA4LL, 0x93E2A89D1DAF3327LL, 0xB0C4D93293A295C1LL, 0xC80CF6EC257F7312LL, 0x5AFA753D0ACD0130LL, 0x96AF80A64DEF717DLL, 0xF8ACCB6A2FE5196FLL,
0xDD0A4D5B5C1D215ELL, 0xC34300BB46EB9347LL, 0x5411B3BE16DA5F92LL, 0x684585F768A7CE1BLL, 0x941465C6672E1F57LL, 0x2D7C51046B0EAA08LL, 0x648D13FAED26DA3ELL, 0xA4EC588D84466058LL,
0x49130F7B82174554LL, 0xAAA5518D3961B481LL, 0xB27780EAA746F79FLL, 0xD1D092A030E42126LL, 0x7C0C9FF0C8720D80LL, 0x38B93FAD85A2150DLL, 0x1E7F6AF95FC31136LL, 0x1CFB3E0C75045016LL,
0x05A7788C6DC71739LL, 0x0C32324921486AA6LL, 0xFDB67D5B6271DEF2LL, 0x76711792B83C3F48LL, 0xA57A37C961F4B050LL, 0x24AB64AD89927642LL, 0x8140B8DA4374A11FLL, 0x01D5067165EACDA4LL,
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
