------------------
-
- c1_chess tool
-
-   how o publish 2D-web-chess tournament
-
------------------

c1_chess.exe - generates a .js file from .pgn,
 		and web-page with 2D chess browser version

c1_large.exe - processes all .pgn files in folder


+--<note>---

Important: in /D case when making local browsables:
 
 The folder "d_img" contains images. Anyway, the path to images is set in c1_chess.js, the line

	var _c1_domain = 'https://chessforeva.gitlab.io/';

 Edit the .js file, remove domain manually in case if "d_img" will reside on own webserver, or locally

	var _c1_domain = '';	// or 'https://owndomain/'


 There is no big reason to that anyway, the web is always online.
+----------



c1_large_php.exe - does the same as c1_large.exe (previous version), without options


Working samples and tutorial:
https://chessforeva.gitlab.io/c1_chess.htm

2021.jan
