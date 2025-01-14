/*

	Webassembly sample version of u64chess
	
	It's how to make it advanceable and working...ok
	https://github.com/Chessforeva/Cpp4chess     u64_chess
	
*/


// C program to compile with emscripten

#include <emscripten.h>
#include "u64_chess.h"

char data[10000];
U8 mv[4<<8];

/*
1.Emscripten compile that works good (Chrome, Windows 10 compiler emcc, jun.2022)


emcc u64wasmSample.c -O2 -s WASM=1 -o u64wasmSample.html -s INITIAL_MEMORY=134217728

*/

// Init chess
extern "C" void EMSCRIPTEN_KEEPALIVE _init() {
  Init_u64_chess();
}

// Do sample moves
extern "C" void EMSCRIPTEN_KEEPALIVE _makeSomeMoves() {

    uciMove("f2f3");
    uciMove("e7e5");
    uciMove("g2g4");

    MoveGen(mv);
    sLegalMoves( data,mv);	// prepare current moves to Javascript
}

// To return string from data to Javascript
// (loop till 0)
extern "C" int EMSCRIPTEN_KEEPALIVE _get( int i ) {
  return (int)data[i];
}

