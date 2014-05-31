/*
	c1_chess - Utility for creation of web-pages from chess pgn-notations

	Designed on MS Visual Studio 2010 C++

*/

#include "stdafx.h"	// nothing

#include <tchar.h>
#include <stdio.h>
#include <windows.h>
#include <string.h>

// user set options
bool userSortOption = false;
long userGamesCnt = 0;
bool userToTxtOption = false;
bool userTabsOption = false;
long userFileLimitSize = 0;
long userCanvasSize = 0;
long userIntDist = 0;
bool userNoDomainOption = false;

char filename[2048], resname[2048];

#include "c1_chess.h"
#include "c1_chess_pgn.h"

int _tmain(int argc, _TCHAR* argv[])
{
	//c1_samples();
	//c1_pgn_samples();

	if(argc<2)
		{
		printf("Chess logic processing utility v1.0, 2014\n\n");
		printf("Creates a browsable web-page from pgn-file (optional sorting by rounds)\n");
		printf("or creates a txt-file of all boards and moves for chess games from pgn-file\n\n");
		printf("usage: c1_chess <pgn file(s) to parse> [options]\n");
		printf("  options list:\n");
		printf("   /S=2k - big event of 2000 games, sorted by rounds\n");
		printf("            (default is unsorted, for fast processing on low memory usage)\n");
		printf("   /2 - output positions and moves to txt-files (default is html)\n");
		printf("   /T - use tabs in annotations (default is inline)\n");
		printf("   /L=4 - set limit of htm-file to 4Mb (default is 2Mb)\n");
		printf("   /C=300 - set board canvas size to 300px (default is 200px)\n");
		printf("   /I=2k - more board canvases 2000 chars inbetween (default is 8000)\n");
		printf("   /D - remove domain to use locally or on other server\n");
		printf("\nSample to process all pgn-files of current folder:\n");
		printf("   c1_chess  *.pgn /T /L=1 /C=400\n\n");
		printf("Free source at: http://github.com/Chessforeva/Cpp4chess\n");
		printf("Blog at: http://chessforeva.appspot.com/c1_chess.htm \n\n");
		printf("Click Enter.\n"); getchar();
		}
	else
		{
		char *p, *w, c, *b, *sp;
		char a[1024];			// to convert TCHAR* to char*

		int i,k,l,fcount=0;
		filename[0] = 0;	// no files

		for(i=1;i<argc;i++)
			{
			for( c=1, b=a, w=(char *)argv[i]; c!=0;)		//convert
				{
				c = *(w++); w++;	// TCHAR is 2 bytes wide, ignore second byte
				*(b++)=c;
				}

			if(a[0]=='/')
				{
				c = a[1];
				if(c=='S' || c=='s')
					{
					userSortOption = true;
					if(a[2]=='=') userGamesCnt = atoi(&a[3]) * 1000;			// ts.
					}
				else if(c=='2') userToTxtOption = true;
				else if(c=='T' || c=='t') userTabsOption = true;
				else if(c=='L' || c=='l')
					{
					if(a[2]=='=') userFileLimitSize = atoi(&a[3]) * 1024 * 1024;	// Mb
					}
				else if(c=='C' || c=='c')
					{
					if(a[2]=='=') userCanvasSize = atoi(&a[3]);
					}
				else if(c=='I' || c=='i')
					{
					if(a[2]=='=') userIntDist = atoi(&a[3]) * 1000;			// ts.
					}
				else if(c=='D' || c=='d') userNoDomainOption = true;
				}
			}

		for(i=1;i<argc;i++)
			{
			if(argv[i][0]!='/')
				{
				for(p=filename, w=(char *)argv[i], c=1; c!=0; )
					{
					c=*(w++); w++;	// TCHAR is 2 bytes wide, ignore second byte
					*(p++)=c;
					}
				b = filename;
				l = strlen(b);
				for(k=l-1; k>=0; k--)
					{
					c = b[k];
					if(c=='/' || c=='\\') { p = &b[k+1]; break; }
					}
				if(k>=0) *(p)=0;
				else p= filename;
				sp = p;

				WIN32_FIND_DATA fd;
				HANDLE h = FindFirstFile(argv[i],&fd);
				for(;h != INVALID_HANDLE_VALUE;)
					{
					for(p=sp, w=(char *)fd.cFileName, c=1; c!=0; )
							{
							c=*(w++); w++;	// TCHAR is 2 bytes wide, ignore second byte
							*(p++)=c;
							}
					if(sp[0] != '.')
					{
					printf("%s\n",filename);
					strcpy(resname,filename);

					c1_pgn P;
					P.c1_0();	// clear counters

					if(userSortOption) P.fsort = true;
					if(userToTxtOption) P.rf = 2;
					if(userTabsOption) P.tabs = true;
					if(userCanvasSize>0) P.canvSize = userCanvasSize;
					if(userIntDist>0) P.dist = userIntDist;
					// for other options look in pgn class constructor

					b = resname;
					l = strlen(b);
					for(k=l-1; k>=0; k--)
						{
						c = b[k];
						if(c=='/' || c=='\\') { p = &b[l]; break; }
						if(c=='.') { p=&b[k]; break; }
						}
					if(k<0) p = &b[l];
					for(;;)
						{
						sprintf(p, ".%s", ( userToTxtOption ? "txt" : "htm" ));
						if( strcmp( resname, filename )==0 ) p = &b[l];
						else break;
						}
					P.ReadParsePgnFile(filename, resname);
					if(P.tooLarge)
						{
						printf( "Result too large.\n" );
						P.tooLarge = false;
						}
					fcount++;
					}

					if(FindNextFile(h, &fd) == FALSE) break;
					}
				}

			}
		if(fcount>0) printf("%d file%s processed.\n", fcount, (fcount>1 ? "s" :""));
		}

	return 0;
}

