#include <stdio.h>
#include <stdlib.h>

/*

    Chess Magics usage sample

Compiler: gcc

*/

#define U64 unsigned long long
#define U32 unsigned int
#define U16 unsigned short
#define U8 unsigned char

#define B2G7 0x007E7E7E7E7E7E00LL

// pre-generated constants, magicgen bc16

static const U64 BishopMagics[] = {
// Magics for bishops

0x6CFFD8B9D37E06BBLL /*A1 >>48*/, 0x7C178C7BF7F57CAELL /*B1 >>48*/, 0xBD98AC81272FB5B8LL /*C1 >>48*/, 0xEDFAE51870C9F19FLL /*D1 >>48*/, 0x87F47FB1C074593ALL /*E1 >>48*/, 0xD7C16D85CA270CF5LL /*F1 >>48*/, 0x5BA782D5B7C17BB3LL /*G1 >>48*/, 0x3A4C9A5ACF80510DLL /*H1 >>48*/,
0x6D57FAB6246EF2B3LL /*A2 >>48*/, 0x0BD74D62FC617A7BLL /*B2 >>48*/, 0x9584DA4D20B959BDLL /*C2 >>48*/, 0x6F723DEBECA069F4LL /*D2 >>48*/, 0xF4ACC795CB2D5CB5LL /*E2 >>48*/, 0x43C4F193D04AD6A9LL /*F2 >>48*/, 0x612085F344704B15LL /*G2 >>48*/, 0xE1A9E4CF7060D21FLL /*H2 >>48*/,
0x596BEA7F05C0FE36LL /*A3 >>48*/, 0x6214025A7E63AD2FLL /*B3 >>48*/, 0x15912315DA5016A0LL /*C3 >>48*/, 0xBB3A96FA5092215FLL /*D3 >>48*/, 0x51C771E8BDF392C0LL /*E3 >>48*/, 0x7AEB8FC497D0BC32LL /*F3 >>48*/, 0x36FB5CF010EC579ELL /*G3 >>48*/, 0x34EB7C64AE28C243LL /*H3 >>48*/,
0x4212BD8C83A6E4C0LL /*A4 >>48*/, 0xE0412592D1C1C3FFLL /*B4 >>48*/, 0x7751B104DA2A81FBLL /*C4 >>48*/, 0xF730B59B8571381CLL /*D4 >>48*/, 0x634900EA035C3400LL /*E4 >>48*/, 0xC840D3F3B6F9B929LL /*F4 >>48*/, 0x39C239F3FC93CB76LL /*G4 >>48*/, 0x66C5D96788357B03LL /*H4 >>48*/,
0xD9824BEFC9415C11LL /*A5 >>48*/, 0xCD7DF0EFE183E6E3LL /*B5 >>48*/, 0xD79C76F086E33E64LL /*C5 >>48*/, 0x4DBA5F8501CE2400LL /*D5 >>48*/, 0xCBAF744A8043F100LL /*E5 >>48*/, 0x617DBA463E646CFFLL /*F5 >>48*/, 0x61B7B064856AD941LL /*G5 >>48*/, 0x5E682D8D0B916E5DLL /*H5 >>48*/,
0xFD9CACF9DA01803ELL /*A6 >>48*/, 0xF6779696FF3E7EE0LL /*B6 >>48*/, 0x6FE2E5338E0C1248LL /*C6 >>48*/, 0x72165A308C570B6ELL /*D6 >>48*/, 0xAF7756B732E846E2LL /*E6 >>48*/, 0x6B1A5D734E172568LL /*F6 >>48*/, 0x2197A3345514B9D9LL /*G6 >>48*/, 0x678DBC7501FB2C50LL /*H6 >>48*/,
0x78612467DABD1F04LL /*A7 >>48*/, 0x33966792F76720D1LL /*B7 >>48*/, 0x96C66374DDFB3360LL /*C7 >>48*/, 0x53860F0EE0C8CD45LL /*D7 >>48*/, 0x534290A02B766B2CLL /*E7 >>48*/, 0x5595C747518329FBLL /*F7 >>48*/, 0x4370788E0145B955LL /*G7 >>48*/, 0xFD35BBAF299013D9LL /*H7 >>48*/,
0x6E266965ADAF646CLL /*A8 >>48*/, 0xA1DEEFF0A585674FLL /*B8 >>48*/, 0xD262607972DD3A36LL /*C8 >>48*/, 0xD46BC2B72BF218A5LL /*D8 >>48*/, 0x562CDC3F02EC43F5LL /*E8 >>48*/, 0x2AA80F72248BE656LL /*F8 >>48*/, 0x91F23AC46A1EE715LL /*G8 >>48*/, 0x013FB443B15EB41ALL /*H8 >>48*/,
0
};

static const U64 RookMagics[] = {
// Magics for rooks
0xFF260527D2CAC700LL /*A1 >>48*/, 0x4CF0F24E7D5A5800LL /*B1 >>48*/, 0x829576FC130A3080LL /*C1 >>48*/, 0x5CFB475FC91E1100LL /*D1 >>48*/, 0x1C6E08D3004F2600LL /*E1 >>48*/, 0xF1B1998208AC1078LL /*F1 >>48*/, 0x06FCBDB2CB1BC4A8LL /*G1 >>48*/, 0x938C7FC1E07D6298LL /*H1 >>48*/,
0x4E2D37B8F9E42E39LL /*A2 >>48*/, 0x45719CD9ACBBEB00LL /*B2 >>48*/, 0x29FBB48EFBF98AC0LL /*C2 >>48*/, 0xBBE3DBB8542C8C80LL /*D2 >>48*/, 0x1B0CBBC889E0A010LL /*E2 >>48*/, 0x784844442DBFABB8LL /*F2 >>48*/, 0x08320F627EE206D0LL /*G2 >>48*/, 0xE651EECBDD946AE0LL /*H2 >>48*/,
0xBD8AEA197A509A03LL /*A3 >>48*/, 0x6412AE5F67893D00LL /*B3 >>48*/, 0x4CD4742FDEAA6A00LL /*C3 >>48*/, 0x324FDAC71DBAE100LL /*D3 >>48*/, 0x0D6F612384FF4DB0LL /*E3 >>48*/, 0xC0322570866933F0LL /*F3 >>48*/, 0xFE527633BA19DCCCLL /*G3 >>48*/, 0x65DD6D0E8C0A245CLL /*H3 >>48*/,
0xCCD36B245D1A34BFLL /*A4 >>48*/, 0x301B25F9D1808F00LL /*B4 >>48*/, 0xCDD5B0E696401E00LL /*C4 >>48*/, 0xAA567462A1BFE7C0LL /*D4 >>48*/, 0xCB8D9FFE2B0A4540LL /*E4 >>48*/, 0xB002D050508A8808LL /*F4 >>48*/, 0x90DBCB9AA74E5DECLL /*G4 >>48*/, 0x52BD607AD16823CELL /*H4 >>48*/,
0x6789D8D2F2B06E12LL /*A5 >>48*/, 0x84DBDAD1C0C9E180LL /*B5 >>48*/, 0xECA9A368B64154C0LL /*C5 >>48*/, 0xCB663F3098641DE0LL /*D5 >>48*/, 0x900DB76EA2B3CE30LL /*E5 >>48*/, 0x09A70669D4010E20LL /*F5 >>48*/, 0xC512D6EE770DE5A0LL /*G5 >>48*/, 0x797423BE8627BC94LL /*H5 >>48*/,
0x8A3D575077F24F63LL /*A6 >>48*/, 0x0EC9B8F218AB9D00LL /*B6 >>48*/, 0x0C8587D6C0DD33C0LL /*C6 >>48*/, 0x60AF95CADD982560LL /*D6 >>48*/, 0xACC5169EB71CA590LL /*E6 >>48*/, 0x160D1C32B7BC6398LL /*F6 >>48*/, 0x88B6F3A8886C2D2ALL /*G6 >>48*/, 0x4C5BC7A588A0AB5CLL /*H6 >>48*/,
0x0A2F0CEF3B7D0C39LL /*A7 >>48*/, 0xEF4D5C4858599E00LL /*B7 >>48*/, 0x1B041BF13B674200LL /*C7 >>48*/, 0xFB6B678E03BAAE00LL /*D7 >>48*/, 0xC277382FB0280FE0LL /*E7 >>48*/, 0x5C8971741F006128LL /*F7 >>48*/, 0x8A909A7AC999AAACLL /*G7 >>48*/, 0x5405CD9DAE7D3E08LL /*H7 >>48*/,
0x9D4D6B7CC2DAD786LL /*A8 >>48*/, 0xCE3D0AD07A35766ALL /*B8 >>48*/, 0xC0EA25A977591592LL /*C8 >>48*/, 0xC5C9BB5E7D8EF022LL /*D8 >>48*/, 0xB1552E04EC9C49E9LL /*E8 >>48*/, 0x9C16B80885B53F9ELL /*F8 >>48*/, 0xDE285EF1DAC7D27ALL /*G8 >>48*/, 0x441830E566CDF37ALL /*H8 >>48*/,
0
};

// arrays to prepare

     // pre-calculated masks to make keys list, some get same result
     // these[] are also known and can be assigned as magics numbers,
     // it is easy to calculate anyway
U64 BishopMask[64];
U64 RookMask[64];

    // The lookup table of legal moves, shifting 16 bits
U64 BishopLegalsTable[64][1<<16];
U64 RookLegalsTable[64][1<<16];

U64 getBishopMove( U8 square, U64 occupancy ) {
    return BishopLegalsTable[square]
        [(U16)(((occupancy & BishopMask[square])*BishopMagics[square] ) >> 48)];
}

U64 getRookMove( U8 square, U64 occupancy ) {
    return RookLegalsTable[square]
        [(U16)(((occupancy & RookMask[square])*RookMagics[square] ) >> 48)];
}

// ---------------------- prepares arrays
//
// variables

U64 Bo1,Bo2;    // boards
int SqI;        // square to prepare
int b_r;        // 1-bishops, 0-rooks

int legalck = 0;      // Calculate: 0- rays, 1-legalmoves

void dir(int dv, int dh) {

	int V=(SqI>>3),H=(SqI&7);
	V+=dv; H+=dh;
	while( (V>=0 && V<8) && (H>=0 && H<8) ) {
		int sq = (V<<3)|H;
		if(legalck) {
            Bo2 |= (1LL<<sq);
            if( Bo1 & (1LL<<sq) ) return;
		}
		else { Bo1|=(1LL<<sq); }
		V+=dv; H+=dh;
		}
	}

void gen2dir() {

    if(legalck) Bo2=0LL;
    else Bo1=0LL;

    if(b_r) {   //bishops
        dir(-1,-1); dir(+1,-1); dir(-1,+1); dir(+1,+1);
    }
    else {  // rooks
        dir(-1,0); dir(+1,0); dir(0,+1); dir(0,-1);
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

        U64 mult = Bo1 * (b_r ? BishopMagics[SqI] : RookMagics[SqI]);
        U16 index = (U16)(mult >> 48);

        gen2dir();      // find legal moves for square, put in Bo2

        if(b_r) BishopLegalsTable[SqI][index] = Bo2;
        else RookLegalsTable[SqI][index] = Bo2;
	}
}

void prepare_tables() {

    for(SqI=0;SqI<64;SqI++) {
        for(b_r=0; b_r<2; b_r++) {
            legalck=0;
            gen2dir();

              // masks B2-G7 rectangle squares, as keys search same results
              // but not needed here, because these magics contain all cases
            if((1LL<<SqI)&B2G7) Bo1&=B2G7;

            if(b_r) BishopMask[SqI]= Bo1;
            else RookMask[SqI]= Bo1;
            legalck=1;
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

        // put any pieces on A8 B7 E1
    U64 Occupancy =(1LL<<56)|(1LL<<49)|(1LL<<4);

    U64 bishop_E4 = getBishopMove(28,Occupancy);
    displayBitboard(bishop_E4);

    U64 rook_H1 = getRookMove(7,Occupancy);
    displayBitboard(rook_H1);

    printf("\nPress a key to close...\n");
    getch();

    return(0);
}

/*
Displays:

oooooooo
oXoooooX
ooXoooXo
oooXoXoo
oooooooo
oooXoXoo
ooXoooXo
oXoooooX

oooooooX
oooooooX
oooooooX
oooooooX
oooooooX
oooooooX
oooooooX
ooooXXXo

*/
