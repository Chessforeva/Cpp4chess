These sources were used to make a chess polyglot book from
  https://www.computerchess.org.uk/ccrl/402.archive/games.html

As result a .bin-file:
  http://chessforeva.surge.sh/chessbook/ccrl402_pack.htm


Toolchain:

1.  pgn_filter_elo.exe from="CCRL.40-2.Archive.[2165313].pgn" to="elo2400.pgn" elo=2400

2.  pgn2uci.exe elo2400.pgn ucis.txt

3.  ucitxt_polyglot.exe ucifile=ucis.txt out=ccrl402_pack min_occ=3 


Someday may be needed...



The poly2csv tool is for .bin polyglot books to extract them to usable text file.


jan.2026