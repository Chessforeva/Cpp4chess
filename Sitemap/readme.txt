
 A very simple sitemap* creator to let user to browse for files on web sites.

This MS Windows batch tool makes a simple 
 Index of... file browser for a web server, as Apache's

*It is not the sitemap.xml for Googlebot

Steps:
1. cmd> dir /s >output.txt         of a local folder, or a variety of .txt outputs of folders;
2. write a config.ini file by copying and modifying sample.ini;
3. cmd> sitemap.exe config.ini     to make a browsable web document.

Notes on filenames in the config file. The algorithm searches for parts in fullpath.
So, provide exact fullpath, or be sure there is no other file or folder with that part in the name.
Also uppercases & lowercases are different filenames.

Directory outputs can be copied to a single file:
cmd> copy *.txt output_all.txt
Or list multiple files, then it will try to read format (date,size,dots and commas) for each of them.

That is all.

Chessforeva apr.2021

Thanks to TinyC compiler (http://hwww.tinycc.org)

--------------------
*upd.dec.2024
files to be listed in files.lst

