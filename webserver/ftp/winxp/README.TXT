This is an FTP tool to access old Windows PC.
TinyC compiled, old style C, not up to date.

ftp_xp.exe server does it in a single connection.
No userlist, no ssl. No threading, minimal functionality.
Intended to send, receive and close the application.

It can serve Windows cmd.exe > ftp.exe
Or other ftp client app.
Features are:
Passive or port, active or binary data sending.
Can set UTF8 if filenames encoded.
Simple, small, does nothing but requires nothing too.

Place it in a folder and run.
The current folder of the application becomes the root "/"
Put and get, and rename, and delete too.
Limited to this root.

In fact, it works well in local network.
The router and firewall also blocking ftp from outside.
 
Nowadays just use web-browser and download the needed things.
The www_xp.exe, for example, can do the same.

Anyway, better get zipper on that PC, zip the content and
 send or download from elsewhere. No need look each file.

FTP is very old thing. It becomes obsolete. Use curl or wget.

Issues seen:

Empty folders sometimes give errors on Windows Explorer.
I suspect Microsoft did not like ftp and made a garbage just to be.
After disabled everything at all.

Important: always set binary file transfer, otherwise will get
truncated short files. ASCII was used on mainframes.
For windows ftp.exe it is "binary" command.

For encoded folders and filenames in ftp.exe use ""
cd "filename%(my)+-folder"

File sizes may be small because it isn't important in sending.

-----------------
Chessforeva 30%, ChatGPT 60%, Gemini 10%
2025
