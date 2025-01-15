# Stockfish 5 webassembly build

It is a .wasm chess engine without extensive web-server requirements.<br />
 A minimal sandboxed for webpages bestmove calculator for chess positions.<br />
 Not a strongest Stockfish build ever.<br />

Online sample at<br />
 https://chessforeva.github.io/stockfish5wasm/main.html

## How to compile

Just used a good Stockfish 5 well prepared source for Emscripten<br />
 https://github.com/exoticorn/stockfish-js

Made small adjustments, look for tag [Emscript] everywhere

Compiled on Windows 10
```sh
  emcc main.cpp -O2 -s WASM=1 -o main.html -s INITIAL_MEMORY=134217728

  ```

This generated 3 files:<br /> main.html, main.js, main.wasm

Made some adjustments in<br />
 main.html, main.js<br />
Recompile main.wasm and overwrite as needed.

That's it.

Chessforeva,jan.2025<br />
 https://github.com/Chessforeva/Cpp4chess/  Stockfish5_wasm
