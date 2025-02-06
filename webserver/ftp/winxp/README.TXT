This is an FTP tool to access old Windows PCs.
To get out photos and content from once abandoned stuff.

Made for TinyC, the best compiler for old Windows.
Read down about Security! This thing goes online.

ftp_xp.exe server can serve one single connection.
No userlist, no ssl. No threading, minimal functionality.
Intended to send, receive and close the application.

It can serve Windows cmd.exe > ftp.exe
Or other ftp client apps.

Features are:
Passive or port, active or binary data sending.
Can set UTF8 if filenames are encoded.
Encoded paths and files can be accessed by using "?" instead
of non-ascii letters. ftp>cd "J??AA"
Also can try OPTS UTF8 OFF, or ON. Sometimes helps.
It is specially for opening old folders made by non-IT members :)
Also can modify access attributes properties of files and folders,
if it blocks something.
Actually this tool reads PC as a local disk.

Place it in a folder and run.
The current folder of the application becomes the root "/"
Put and get, and rename, and delete too.
Limited to the root. Normally, but admin can access everything.

In fact, it works well on the local network.
The router and firewall also block ftp from outside.
 
Nowadays just use a web-browser and download the needed things.
The www_xp.exe, for example, can do the same.

Anyway, better get zipper on that PC, zip the content and
send or download from elsewhere. No need to look at each file.
Depends on disk space capabilities. 

FTP is a very old thing. It becomes obsolete. Use curl or wget.
Or ssl based servers with multithreading and proper user management.
They are known to security systems, not such a personal PC hacking tool.

Security:

 This tool, in result, is too powerful, so have to make some restrictions.
Command line options define user access and permissions.
Help is on /? or /h.
Otherwise can get a responding something. Read the help and everything is here.
It is for people having good intentions. Tool can be evil we don't wanna be.

Issues seen:

Empty folders sometimes give errors on Windows Explorer.
I suspect Microsoft did not like ftp and made garbage just to be.
After disabled everything to not confuse users, it seems.

Important: always set binary file transfer, otherwise will get
truncated short files. ASCII was used on mainframes.
For windows ftp.exe it is "binary" command.

For encoded folders and filenames in ftp.exe use ""
cd "filename%(my)+-folder"

File sizes may be small because it isn't important in sending.

Modern Windows suspect TinyC executables as malwares and deletes. 
It is made for XP anyway.

Prior to XP there were times of Trumpet over telephone line and WS_ftp tool,
as I remember, from the tucows site once. Or others.
Nobody knew about ssl and harmful bots all day checking network sockets, by the way.

-----------------
Chessforeva 30%, ChatGPT 60%, Gemini 10%
2025
