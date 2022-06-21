A fast 64-bit bitbards chess moves processing C source.
It is not an engine. It is used for checkmates searching.
Also it is 64-bit, gcc, clang, MSVC tested on chess games,
ready for Webassembly programming.
u64_chess is a base for next C projects.

Chessforeva, feb.2022

----------------
Added 03.2022:
  pgn2uci tool prepares uci moves for chess engines
	stockfish position startpos moves e2e4 e7e5 ....
Usage: pgn2uci <pgn-file> <output-uci-moves-textfile>

Also there is c1_chess tool too, but not so fast.

Added 06.2022:
  WebAssembly usage sample
  https://chessforeva.codeberg.page/wasm/u64wasmSample.htm

