# Stockfish 5 webassembly build

It is a .wasm chess engine without loads of web-server requirements.
Online sample at
https://chessforeva.github.io/stockfish5wasm/main.html

## How to compile

Just used a good Stockfish 5 well prepared source for Emscripten
https://github.com/exoticorn/stockfish-js

Made small adjustments, look for tag [Emscript] everywhere

Compiled on Windows 10
```sh
  emcc S\main.cpp -O2 -s WASM=1 -o main.html -s INITIAL_MEMORY=134217728

  ```

This generated 3 files: main.html, main.js, main.wasm

Made some adjustments in main.html, main.js
Recompile main.wasm and overwrite as needed.

That's it.

Chessforeva
https://github.com/Chessforeva/Cpp4chess/Stockfish5_wasm
jan.2025