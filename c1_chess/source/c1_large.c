/*

 c1_large.exe  -  extender for c1_chess.exe, that should be in current folder!

 Splits large PGN files.
 Generates HTML index and browsable chess pages

 Place PGN files in current folder and start this tool to generate browsables.
 Chessforeva (2014.oct.)

 Compiled on LCC-WIN32

*/

#include <direct.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <process.h>
#include <io.h>

int GAMES_PER_BROWSABLE = 100;			// default

int SingleEvent = 0;
int dont_set_CANVAS_SIZE = 0;
int LOCAL_OR_NODOMAIN = 0;

char c1_args[14][1024];
int c1_argc = 0;

// MAIN PROGRAM
int main(int argc,char *argv[])
{

	c1_argc = 1; // first reserved for canvas size
	for(int o=0; o<14; o++) sprintf( c1_args[o], "" );

	if(argc<2)
		{
		printf("Extender for c1_chess.exe\n");
		printf("  searches for chess PGN files in the current folder\n");
		printf("  and generates browsable web versions.\n\n");
		printf("Usage: c1_large [options]\n");
		printf("  options list:\n");
		printf("  /D - for local or other domain\n");
		printf("  /G=100 - count of games per browsable\n");
		printf("  /E - single event file, no sorting\n");
		printf("  +other c1_chess options\n\n\n");
		printf("Press key to proceed or ESC to cancel\n\n");
		char c = getch();
		if(c==27 || c=='n' || c=='N') return 0;
		}
	else
		{
		for(int argi = 1; argi<argc; argi++)
			{

				char *aS = argv[ argi ];
				char *y = aS;


				if( *(y++)=='/' && ((*y)=='D' || (*y)=='d' ) )
				   	{
					LOCAL_OR_NODOMAIN = 1;
					}
				y = aS;
				if( *(y++)=='/' && ((*y)=='C' || (*y)=='c' ) )
				   	{
					dont_set_CANVAS_SIZE = 1;
					sprintf( c1_args[0], "%s", aS );
					}
				else
					{
					y = aS;
					if( *(y++)=='/' && ((*y)=='G' || (*y)=='g' ) && *(++y)=='=' )
					   	{
						GAMES_PER_BROWSABLE = atoi( ++y );
						}
					else
					   	{
						y = aS;
						if( *(y++)=='/' && ((*y)=='E' || (*y)=='e' )  ) SingleEvent = 1;
						else
							{
							sprintf( c1_args[ c1_argc++ ], "%s", aS );
							}
						}
					}

			}
		}

	printf("Working...\n");

	// Create folder
    char web[60];		// will create new filder for browsables
    time_t ltime; time(&ltime);
	sprintf(web,"web_%d",ltime);	// make a folder for datas

	char cur[1024];		// current path
	getcwd( cur, 1024 );

	char path[1024];	// working path
	sprintf(path,"%s\\%s",cur,web);
	mkdir (path);
	printf("New folder %s\n",web);

	char indxfile[1024];	// index web page file
	sprintf(indxfile,"%s\\index.htm",path);
	FILE *I = fopen (indxfile, "w");
	fprintf(I,"<!doctype html>\n");
	fprintf(I,"<meta http-equiv=Content-Type content=\"text/html; charset=utf-8\">\n");
	fprintf(I,"<title>Browsable chess games</title>\n");
    fprintf(I,"<body>\n");
	fprintf(I,"<table style=\"display:inline;color:#993300\">\n");
	fprintf(I,"<tr>\n");

	char filemask[1024];	// index web page file
	sprintf(filemask,"%s\\*.*",cur);
    char **flist = findfiles(filemask, FINDFILES_NODIRS);
    char **foflist = flist;

	char Fname[2048], F[2048], FS[2048], fnameR[2048];
	unsigned long pos = 0, p_pre, p_max;

	int c1_exist = 0;			// is there c1_chess.exe or not

    while (*foflist) {
		sprintf(Fname,*foflist);
		sprintf(F,(char*)(Fname+strlen(cur)+1) );

		if(strcmp(F,"c1_chess.exe")==0) c1_exist = 1;

		int ax = charRAt(F,'.');

		char ext[100];	// file extension
		sprintf(ext,"%s", (ax>0 && ax == strlen(F)-4 ? (char*) (F+ax+1) : "") );
		sprintf(FS,"%s", F);
	    if(ax>0 && ax == strlen(F)-4)	// make file name shorter without extension
		   	{
			int lS = strlen(FS);
			char *pS = FS+lS-4;
			(*pS) = 0;
			}

		if( strlen(ext)==0 || strstr("{exe}{dll}{com}", ext)==NULL )
    	{

		// so this file may contain PGN datas, lets process it
		FILE *f = fopen (Fname, "rb");
		fseek(f, 0, SEEK_END);
		p_max = ftell(f);

		fseek(f, 0, SEEK_SET);

		int MAX_buffer = 6 * 1024;	// 6KB
		char buf[ MAX_buffer ];		//  of string we read and process
		int is_pgn = 0;
		int MAX_events = 500;
		char eventlist[MAX_events][1024];
		int evcnt = 0;
		char event[1024], *ev;
		char d[1024];

		for( ; !feof(f) ; )
			{
			readDatas(f, buf, MAX_buffer);
			if( memcmp("[Event ",buf,7)==0 && buf[strlen(buf)-1]==']' )
				{
				if(!is_pgn) printf("File: %s\n",F);
				is_pgn = 1;

				getHDdata(buf, &event);
				int isev = 0;
				for(int j = 0; !isev && (j<evcnt); j++)
					if( strcmp( eventlist[j], event )== 0 ) isev=1;
				if(!isev && evcnt<MAX_events) sprintf( eventlist[ evcnt++ ], "%s", event );
				}
			}

		if(is_pgn)
		   	{
			if(SingleEvent) evcnt=1;	// simply one event

			printf("Reading %d events\n",evcnt);

			for(int Ej = 0; Ej<evcnt; Ej++)
				{
					sprintf(event, "%s", eventlist[Ej]);	// current event only
					printf("  event %s\n",event);

					int nw = 1;			// new comes flag
					int gm = 0;			// game flag
					int sk = 0;			// skip flag
					int cx = 0;
					pos = 0;
					int mnRound = 99;
					int mxRound = 0;

					int MAX_GAMES = 5000;		// maximum games per event
					int S[ MAX_GAMES ];			// sorting order
					int Sf[ MAX_GAMES ];		// sorted flag
					long R[ MAX_GAMES ];		// round+table value for sorting
					unsigned long P[ MAX_GAMES ];		// pointer in file
					int Ro[ MAX_GAMES ];		// round
					int Tb[ MAX_GAMES ];		// table

					fseek( f, 0, SEEK_SET );

					for( ; !feof(f) ; )
					{

					p_pre = pos;
					pos += readDatas(f, buf, MAX_buffer);

					if( buf[0]=='[' && buf[1]!='%' && buf[strlen(buf)-1]==']' )
						{
						if(gm)
							{
							if(!sk) cx++;
							sk = 0;
						    gm = 0;
							nw = 1;
							if(cx>=MAX_GAMES)
								{
									printf("Error: More than %d games per event!\n", MAX_GAMES);
									break;
								}
							}
						if(nw)
							{
							memcpy( &P[cx], &p_pre, sizeof( unsigned long ) );
							nw = 0;
							Sf[cx]=0;
							}
						if( memcmp("[Event ",buf,7)==0 )
							{
							getHDdata(buf, &d);
							if( strcmp( d, event )!=0 && (!SingleEvent) ) sk = 1;
							gm = 0;
							}
						if( memcmp("[Round ",buf,7)==0 )
							{
							getHDdata(buf, &d);
							int Rd = atoi(d);
							int Tl = 0;

							long Rou = Rd;
							if(Rou>0 && Rou<30) Rou *= 10000;

							int qx = charRAt(d,'.');
							if(qx<0) qx = charRAt(d,',');
							if(qx>=0)
								{
								Tl = atoi( &d[qx+1] );
								Rou += Tl;
								}
							memcpy( &Ro[cx], &Rd, sizeof( int ) );
							memcpy( &Tb[cx], &Tl, sizeof( int ) );
							memcpy( &R[cx], &Rou, sizeof( long ) );
							if (Rd<mnRound) mnRound = Rd;
							if (Rd>mxRound) mxRound = Rd;
							}
						}
					else if( strlen(buf)>0 ) gm = 1;
					}

					if(gm) if(!sk) cx++;

					int t, w, m, r, gk, i=0, k=0, si=0;
					if(!SingleEvent)
						{
						for(k=0; k<cx; k++ ) S[k]=k;	// no sorting
						}
					else
						{
						for(k=0; k<cx; k++ )		// bubble sorting
							{
							si=-1;
							for(i=0; i<cx; i++)
								{
								if( (!Sf[i]) && (si<0 || R[i]<R[si]) ) si = i;
								}
							Sf[si] = 1;
							S[k] = si;
							}
						}

					FILE *fo;
					fprintf(I,"<td><b>%s</b> (%d chess games)</td></tr><tr>", event, cx);

					sprintf(d,"");	// shorted id_of_event
					if(evcnt>1) sprintf(d,"_%d",Ej);

					if(cx<=GAMES_PER_BROWSABLE)		// single resulting file
						{
						fprintf(I,"<td><a href=\"%s%s.htm\">Rounds %d-%d</a></td></tr><tr>",
							FS, d, mnRound, mxRound);
						sprintf(fnameR, "%s\\%s%s.txt", path, FS, d);
						fo = fopen (fnameR, "wb");
						for(k=0; k<cx; k++)
							{
							m = S[k];
							pos = P[m];
							fseek(f, pos, SEEK_SET);

							gm = 0;
							for( ; !feof(f) ; )
								{
								readDatas(f, buf, MAX_buffer);
								w = strlen(buf);
								if( buf[0]=='[' && buf[1]!='%' && buf[w-1]==']' )
									{
									if(gm) break;
									}
								else if( w>0 ) gm = 1;
								if(w>0) fprintf(fo,"%s\n",buf);
								}
							if(gm) fprintf(fo,"\n\n");
							}
						fclose(fo);
						}
					else
					if((cx/mxRound)<GAMES_PER_BROWSABLE)	// split by rounds
						{
						fprintf(I,"<td>");
						r = 0;
						for(k=0; k<cx; k++)
							{
							m = S[k];
							pos = P[m];
							fseek(f, pos, SEEK_SET);

							if( Ro[m]>r )
								{
								if(r>0) fclose(fo);
								r = Ro[m];
								fprintf(I,"<a href=\"%s%s_r%d.htm\">R%d</a>%c ",
							   	 FS, d, r, r, (r<mxRound ? ',' : ' ') );
								sprintf(fnameR, "%s\\%s%s_r%d.txt", path, FS, d, r);
								fo = fopen (fnameR, "wb");
								}

							gm = 0;
							for( ; !feof(f) ; )
								{
								readDatas(f, buf, MAX_buffer);
								w = strlen(buf);
								if( buf[0]=='[' && buf[1]!='%' && buf[w-1]==']' )
									{
									if(gm) break;
									}
								else if( w>0 ) gm = 1;
								if(w>0) fprintf(fo,"%s\n",buf);
								}
							if(gm) fprintf(fo,"\n\n");
							}
						if(r>0) fclose(fo);

						fprintf(I,"</td></tr><tr>");

						}
					else				// split by portions
						{
						fprintf(I,"<td>");
						r = 0;
						gk = 0;
						int Rf=0, Tf=0;
						int ag = cx / GAMES_PER_BROWSABLE;
						int AVRG = cx / (++ag);
						char *smf = "<font size=\"1\">";
						char *sm_ = "</font>";

						for(k=0; k<cx; k++)
							{
							m = S[k];
							pos = P[m];
							fseek(f, pos, SEEK_SET);

							r = Ro[m];
							t = Tb[m];

							if(gk==0 || gk>=AVRG)
								{
								if(Rf>0)
								   	{
									fclose(fo);

				fprintf(I,"<a href=\"%s%s_r%d.%d.htm\">R%d%s.%d%s-R%d%s.%d%s</a>%c ",
							   	 FS, d, Rf, Tf, Rf, smf, Tf, sm_, r, smf, t, sm_,
								 (r<mxRound ? ',' : ' ') );

									}
								sprintf(fnameR, "%s\\%s%s_r%d.%d.txt", path, FS, d, r, t);
								fo = fopen (fnameR, "wb");
								gk=0;
								Rf = r;
								Tf = t;
								}
							gk++;

							gm = 0;
							for( ; !feof(f) ; )
								{
								readDatas(f, buf, MAX_buffer);
								w = strlen(buf);
								if( buf[0]=='[' && buf[1]!='%' && buf[w-1]==']' )
									{
									if(gm) break;
									}
								else if( w>0 ) gm = 1;
								if(w>0) fprintf(fo,"%s\n",buf);
								}
							if(gm) fprintf(fo,"\n\n");

							}
						if(r>0)
						   	{
							fclose(fo);

					fprintf(I,"<a href=\"%s%s_r%d.%d.htm\">R%d%s.%d%s-R%d%s.%d%s</a>",
							   	 FS, d, Rf, Tf, Rf, smf, Tf, sm_, r, smf, t, sm_ );

							}

						fprintf(I,"</td></tr><tr>");

						}


				 }

			}
		fclose(f);
		}

        foflist++;
    }


	fprintf(I,"</tr>\n");
	fprintf(I,"</table>\n");
	fprintf(I,"</body>\n");
	fprintf(I,"</html>\n");
	fclose(I);

	int spw;

	if(!c1_exist) printf("Error: c1_chess.exe does not exist\n");
	else
		{
		char cmd[1024], fmask[1024];
		sprintf( cmd, "%s\\c1_chess.exe", cur );
		sprintf( fmask, "%s\\*.txt", web );

		char *argz[17];
		argz[0] = (const char *)fmask;
		argz[1] = (const char *)fmask;
		argz[2] = (const char *)c1_args[0];
		argz[3] = (const char *)c1_args[1];
		argz[4] = (const char *)c1_args[2];
		argz[5] = (const char *)c1_args[3];
		argz[6] = (const char *)c1_args[4];
		argz[7] = (const char *)c1_args[5];
		argz[8] = (const char *)c1_args[6];
		argz[9] = (const char *)c1_args[7];
		argz[10] = (const char *)c1_args[8];
		argz[11] = (const char *)c1_args[9];
		argz[12] = (const char *)c1_args[10];
		argz[13] = (const char *)c1_args[11];
		argz[14] = (const char *)c1_args[12];
		argz[15] = (const char *)c1_args[13];
		argz[16] = NULL;

		if(!dont_set_CANVAS_SIZE)	// just set canvas size 400px
			{
			sprintf( c1_args[0], "/C=400" );
			}

		spw = _spawnv( _P_WAIT, cmd, argz );
		if( spw!=0 ) printf("Error: Can not execute c1_chess.exe with parameters\n");

		}

    free(flist);

	// and remove tmp-files
	sprintf(filemask,"%s\\*.txt",path);
	flist = findfiles(filemask, FINDFILES_NODIRS);
	foflist = flist;
	while (*foflist) {
		remove( *foflist );
        foflist++;
    }
	free(flist);

	if(LOCAL_OR_NODOMAIN)		// let's do copying too
		{
		printf("Copying other files.\n" );

		// copy pictures
		char path_img[1024];
		int fld_crea = 0;
		sprintf(path_img,"%s\\d_img",path);

		sprintf(filemask,"%s\\d_img\\*.*",cur);
		flist = findfiles(filemask, FINDFILES_NODIRS);
		foflist = flist;
		while (*foflist) {
			sprintf(Fname,*foflist);
			sprintf(F,(char*)(Fname+strlen(cur)+1) );

			if(!fld_crea)
				{
				mkdir (path_img);
				fld_crea = 1;
				}
			sprintf(fnameR, "%s\\%s", path, F);
			copyFile(Fname, fnameR);
        	foflist++;
    	}
		free(flist);

		// .css
		sprintf(filemask,"%s\\*.css",cur);
		flist = findfiles(filemask, FINDFILES_NODIRS);
		foflist = flist;
		while (*foflist) {
			sprintf(Fname,*foflist);
			sprintf(F,(char*)(Fname+strlen(cur)+1) );
			sprintf(fnameR, "%s\\%s", path, F);
			copyFile(Fname, fnameR);
        	foflist++;
    	}
		free(flist);

		// .css
		sprintf(filemask,"%s\\*.js",cur);
		flist = findfiles(filemask, FINDFILES_NODIRS);
		foflist = flist;
		while (*foflist) {
			sprintf(Fname,*foflist);
			sprintf(F,(char*)(Fname+strlen(cur)+1) );
			sprintf(fnameR, "%s\\%s", path, F);
			copyFile(Fname, fnameR);
        	foflist++;
    	}
		free(flist);

		}

	int isBrowser = 0;
	char *BROWpath[6];
	BROWpath[0]= "C:\\Program Files (x86)\\Google\\Chrome\\Application\\chrome.exe";
	BROWpath[1]= "C:\\Program Files\\Google\\Chrome\\Application\\chrome.exe";
	BROWpath[2]= "C:\\Program Files (x86)\\Mozilla Firefox\\firefox.exe";
	BROWpath[3]= "C:\\Program Files\\Mozilla Firefox\\firefox.exe";
	BROWpath[4]= "C:\\Program Files (x86)\\Internet Explorer\\iexplore.exe";
	BROWpath[5]= "C:\\Program Files\\Internet Explorer\\iexplore.exe";

	char *argb[3];
	argb[0] = (const char *)indxfile;
	argb[1] = (const char *)indxfile;
	argb[2] = NULL;

	for(int z=0;z<6;z++)
		{
		flist = findfiles(BROWpath[z], FINDFILES_NODIRS);
		foflist = flist;
		while (*foflist) {

			char spwnFile[1024];
			sprintf(spwnFile,"%s",BROWpath[z]);

			spw = execv( spwnFile, argb );
			if( spw == 0 ) isBrowser = 1;
			break;
			foflist++;
    	}
		free(flist);
		}

	printf("Done.\n" );

	if(!isBrowser) printf("See %s\\index.htm page!\n", web);

	// wait keypress
	printf("Press key to exit.");
	getch();

	return 0;
}


// some helping functions
int charRAt( char *s, char c )
{
	char *p = strrchr(s,c);
	return (p==NULL ? -1 : p-s);
}

void spcAlltrim( char *s )
{
	char *p = s, *q, *b;
	if( (*p)<=32 )		// remove leading spaces and special chars
		{
		q=p; for( q++;(*q)!=0 && (*q)<=32 ; q++);
		for(b=p; ;q++,b++)
			{
			(*b) = (*q);
			if((*q)==0) break;
			}
		}

	int l = strlen(s);
	if(l>0)
		{
		p = s + (l-1);
		for( ;l>0 && (*p)<=32 ; p--, l-- ) (*p)=0;
		}
}

void getHDdata( char *sou, char *data )
{
	char *p = strchr(sou,'"');
	sprintf(data,"%s", (p==NULL ? "" : p+1));
	p = strrchr(data,'"');
	if(p!=NULL) (*p) = 0;
}

int readDatas( FILE *fi, char *buf, int maxLen )
{
	char *p = buf;
	int l = 0;
	for(;!feof(fi); )
		{
		fread(p,1,1,fi); l++;
		if( (*p)==13 || (*p)==10 || (l+2)>maxLen )
		   	{
			 *(++p) = 0;
			 break;
			}
		p++;
		}
	spcAlltrim( buf );
	return l;
}

void copyFile( char *from, char *to )	// copies files
{
	FILE *in = fopen( from, "rb" );
	FILE *ou = fopen( to, "wb" );
	for( ; !feof(in) ; )
		{
		char c;
		int n = fread(&c, 1, 1, in);
		if(n>0) fprintf(ou, "%c",c);
		}
	fclose(in);
	fclose(ou);
}
