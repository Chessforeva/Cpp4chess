/*
 * Gemini generated - 2026-01-26
 * Chess Diagram Generator (dgr), Chessforeva jan.2026
 * uses lodepng (lodepng.h, lodepng.cpp)
 *
 *  Not the best version of diagrams, but anyway... It works.
 *  The web version at https://chessforeva.gitlab.io/dg.htm
 *  15 picture files t_...png should be in current folder. 
 */

#include "lodepng.h"
#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <cmath>

using namespace std;

// Data structure to store image buffers in memory
struct ImageBuffer {
    vector<unsigned char> pixels;
    unsigned w = 0, h = 0;
};

// Global parameters from command line
unsigned int Size = 1024;
float Sz2 = 1.3f;
bool draw_tm = true;

void drawToBoard(ImageBuffer& board, const ImageBuffer& piece, int x, int y, int xL, int yL) {
    if (xL <= 0 || yL <= 0) return;

    float pW = (float)piece.w;
    float pH = (float)piece.h;

    for (int i = 0; i < yL; ++i) {
        for (int j = 0; j < xL; ++j) {
            // Calculate source coordinates as floats
            float src_xf = ((float)j + 0.5f) * (pW / (float)xL);
            float src_yf = ((float)i + 0.5f) * (pH / (float)yL);

            int dst_x = x + j;
            int dst_y = y + i;

            // Bounds checks using floats before casting
            if (src_xf < 0.0f || src_xf >= pW || src_yf < 0.0f || src_yf >= pH) continue;
            if (dst_x < 0 || dst_x >= 1024 || dst_y < 0 || dst_y >= 1024) continue;

            // Maintain float precision for the index calculation, cast to size_t only at the end
            size_t p_idx = (size_t)((floor(src_yf) * pW + floor(src_xf)) * 4.0f);
            size_t b_idx = (size_t)(((float)dst_y * 1024.0f + (float)dst_x) * 4.0f);

            float alpha = piece.pixels[p_idx + 3] / 255.0f;
            if (alpha <= 0.01f) continue;

            for (int c = 0; c < 3; ++c) {
                board.pixels[b_idx + c] = (unsigned char)(piece.pixels[p_idx + c] * alpha +
                    board.pixels[b_idx + c] * (1.0f - alpha));
            }
            board.pixels[b_idx + 3] = 255;
        }
    }
}


/**
 * Draws the "To Move" indicator triangle
 */
void drawSign(ImageBuffer& board, const ImageBuffer& signImg, int n) {
    float K = Size / 1024.0f;
    int Wi = 140;
    int Hi = 126;

    float x = (1024.0f - 60.0f) * K;
    float y = (n == 0 ? 1024.0f - 60.0f : 10.0f) * K;
    float xL = Wi * 0.4f * K;
    float yL = Hi * 0.4f * K;

    drawToBoard(board, signImg, (int)(x / K), (int)(y / K), (int)(xL / K), (int)(yL / K));
}

/**
 * Ported piece coordinate and offset logic
 */
void drawPiece(ImageBuffer& board, const ImageBuffer& pieceImg, int h, int v, string id) {
    int X0 = 53, Y0 = 62, dY = 112, dX = 115;
    float K = Size / 1024.0f;
    float x0 = 0, y0 = 0, kx = 1, ky = 1;

    if (id == "wp") { x0 = 26; y0 = 18; kx = 260 * 0.23f; ky = 439 * 0.19f; }
    else if (id == "wb") { x0 = 23; y0 = 8; kx = 323 * 0.20f; ky = 739 * 0.135f; }
    else if (id == "wn") { x0 = 16; y0 = 8; kx = 407 * 0.20f; ky = 571 * 0.17f; }
    else if (id == "wr") { x0 = 20; y0 = 18; kx = 407 * 0.17f; ky = 528 * 0.165f; }
    else if (id == "wq") { x0 = 15; y0 = 6; kx = 390 * 0.20f; ky = 842 * 0.12f; }
    else if (id == "wk") { x0 = 6; y0 = 0; kx = 250 * 0.40f; ky = 506 * 0.22f; }
    else if (id == "bp") { x0 = 18; y0 = 18; kx = 300 * 0.25f; ky = 449 * 0.186f; }
    else if (id == "bb") { x0 = 20; y0 = 8; kx = 386 * 0.19f; ky = 690 * 0.146f; }
    else if (id == "bn") { x0 = 20; y0 = 10; kx = 361 * 0.20f; ky = 566 * 0.17f; }
    else if (id == "br") { x0 = 17; y0 = 12; kx = 361 * 0.22f; ky = 504 * 0.19f; }
    else if (id == "bq") { x0 = 8; y0 = 8; kx = 499 * 0.20f; ky = 820 * 0.124f; }
    else if (id == "bk") { x0 = 14; y0 = 4; kx = 391 * 0.22f; ky = 930 * 0.116f; }

    float x = ((h * dX) * K) + ((X0 + x0) * K);
    float y = (((7 - v) * dY) * K) + ((Y0 + y0) * K);
    float xL = kx * K * Sz2;
    float yL = ky * K * Sz2;

    if (Sz2 > 1.0f) {
        x -= (kx * ((Sz2 - 1.0f) * 0.5f) * K);
        y -= (ky * ((Sz2 - 1.0f) * 0.5f) * K);
    }

    drawToBoard(board, pieceImg, (int)(x / K), (int)(y / K), (int)(xL / K), (int)(yL / K));
}

int main(int argc, char* argv[]) {

    if (argc < 2 || string(argv[1]) == "/h") {
        cout << "Chess diagram generator tool." << endl;
        cout << " syntax: dgr fen=<FEN> pgn=<output.png> [size=300-1024] [scale=1.0-1.3]" << endl;
    }

    string fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w";
    string out_file = "output.png";

    for (int i = 1; i < argc; ++i) {
        string arg = argv[i];
        if (arg.find("fen=") == 0) fen = arg.substr(4);
        else if (arg.find("pgn=") == 0) out_file = arg.substr(4);
        else if (arg.find("size=") == 0) Size = stoi(arg.substr(5));
        else if (arg.find("scale=") == 0) Sz2 = stof(arg.substr(6));
    }

    ImageBuffer board;
    lodepng::decode(board.pixels, board.w, board.h, "t_board.png");

    map<char, string> piece_ids;
    piece_ids['P'] = "wp"; piece_ids['B'] = "wb"; piece_ids['N'] = "wn"; piece_ids['R'] = "wr";
    piece_ids['Q'] = "wq"; piece_ids['K'] = "wk"; piece_ids['p'] = "bp"; piece_ids['b'] = "bb";
    piece_ids['n'] = "bn"; piece_ids['r'] = "br"; piece_ids['q'] = "bq"; piece_ids['k'] = "bk";


    // Load piece assets from folder using standard C++11 iterators
    map<string, ImageBuffer> piece_cache;
    for (map<char, string>::iterator it = piece_ids.begin(); it != piece_ids.end(); ++it) {
        string piece_name = it->second;
        string filename = "t_" + piece_name + ".png";

        unsigned error = lodepng::decode(piece_cache[piece_name].pixels,
            piece_cache[piece_name].w,
            piece_cache[piece_name].h,
            filename);

        if (error) {
            cout << "Warning: Could not load " << filename << " - " << lodepng_error_text(error) << endl;
        }
    }

    int rank = 7, file = 0;
    size_t space_pos = fen.find(' ');
    string board_part = (space_pos == string::npos) ? fen : fen.substr(0, space_pos);

    for (char c : board_part) {
        if (c == '/') { rank--; file = 0; }
        else if (isdigit(c)) { file += (c - '0'); }
        else if (piece_ids.count(c)) {
            drawPiece(board, piece_cache[piece_ids[c]], file, rank, piece_ids[c]);
            file++;
        }
    }

    if (space_pos != string::npos && space_pos + 1 < fen.length()) {
        char tm = fen[space_pos + 1];
        if (tm == 'w' || tm == 'b') {
            int n = (tm == 'w') ? 0 : 1;
            ImageBuffer sign;
            string sign_file = (n == 0 ? "t_w3.png" : "t_b3.png");
            lodepng::decode(sign.pixels, sign.w, sign.h, sign_file);
            drawSign(board, sign, n);
        }
    }

    if (Size != 1024) {
        vector<unsigned char> res(Size * Size * 4);
        for (unsigned y = 0; y < Size; ++y) {
            for (unsigned x = 0; x < Size; ++x) {
                int sx = (x * 1024) / Size;
                int sy = (y * 1024) / Size;
                for (int c = 0; c < 4; ++c)
                    res[(y * Size + x) * 4 + c] = board.pixels[(sy * 1024 + sx) * 4 + c];
            }
        }
        lodepng::encode(out_file, res, Size, Size);
    }
    else {
        lodepng::encode(out_file, board.pixels, 1024, 1024);
    }

    return 0;
}