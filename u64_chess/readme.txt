A fast 64-bit bitbards chess moves processing C source.
It is not an engine. It is used for checkmates searching.
Also it is 64-bit, gcc, clang, MSVC tested on chess games,
ready for Webassembly programming.
u64_chess is a base for next C projects.

Chessforeva, feb.2022

----------------
Added 03.2024

 pgn2uci64 for windows x64,  -this is fast and good for large pgn processing

 Bugfix version for pawns - other magics. Rare cases of blocked pawns.

Added 03.2022:
  pgn2uci tool prepares uci moves for chess engines
	stockfish position startpos moves e2e4 e7e5 ....
Usage:
 
 pgn2uci <pgn-file> <output-uci-moves-textfile>

 pgn2uci64 <pgn-file> <output-uci-moves-textfile>
 
Also there is c1_chess tool too, but not so fast.

Added 06.2022:
  WebAssembly usage sample
  https://chessforeva.codeberg.page/wasm/u64wasmSample.htm

Added 06.2022
  Webassembly ChessPuzzles tool (pz.c, pz.wasm)
  http://chessforeva.blogspot.com/2022/07/chess-puzzles-tool.html
