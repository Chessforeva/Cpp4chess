
Chess Diagram Generator tool (dgr)
Requires these 15 png-files in the current folder:
 t_b3.png, t_w3.png, t_board.png,
 t_bb.png, t_bk.png, t_bn.png, t_bp.png, t_bq.png, t_br.png
 t_wb.png, t_wk.png, t_wn.png, t_wp.png, t_wq.png, t_wr.png


The web version at https://chessforeva.gitlab.io/dg.htm
Chessforeva jan.2026

Designed for Colab mostly. It loses quality much when reducing size.

Usage:
    dgr fen="8/2K4R/3P2k1/3N4/4p3/3R4/8/8 w" file="dg1.png"
    dgr fen="8/2K4R/3P2k1/3N4/4p3/3R4/8/8 w" size=800 scale=1.0 file="dg1.png"

Size is for board picture. Can set less than 1024 (by default) and lose quality.
Scale is for pieces from 1.0 to 1.3 (by default).
File is "output.png" by default.


Other notes:
Uses lodepng sources.

Works on MSStudio 2022

Linux compile:
g++ main.cpp lodepng.cpp -o dgr -O3 -Wall -Wextra -static-libstdc++