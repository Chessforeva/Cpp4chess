
magicgen.exe	(Win32, gcc build)

MagicGen.c - A simple chess magic numbers generator.

	Supports chess engines with fast alhorithms
		finding
			bishop / rook legal chess moves on chess board
				by occupancy of pieces as a lookup key
	
	Known.
	Lookup to Table [square][key]
			
		key =   ( Occupancy&Mask * MagicNumber ) >> const

Samples:
			
Sample1.c	- fixed bit-shift
Sample2.c	- masks as constants
Sample3.c	- various bit-shifts


Other:
pawnmagicgen.exe	- some magics for pawns too (bugfix v.03.2024)
pawnMagicGen.c		- with source
pawnSample1.c		- and how to use it


sample_js.htm	- Javascript uint64 chess magic in action samples (emulated HTML5, slow)
https://chessforeva.codeberg.page/ChessMagicGen/sample_js.htm


tables.htm	- An online generator of bishop, rook chess moves masks for all 64 squares.
Can download bitboards table to analyse.
https://chessforeva.codeberg.page/ChessMagicGen/tables.htm

Also sources at:

https://github.com/Chessforeva/Cpp4chess

u64_chess.h uses these magic bitboards


Chessforeva, feb.2022