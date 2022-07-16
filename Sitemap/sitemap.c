/*
 A simple Index Of ... sitemap as filebrowser
  html creator tool v.1.0.
  
  Chessforeva.blogspot.com , 04.2021.
*/

#include <stdio.h>

/*
 A bunch of working variables 
*/

char *config, *dir, *htmS; 
FILE *f, *f2, *fo;
char *p, *q, *g, *v, *w, *m, *u;
int n, i, j, cnt, c, c2, e, D, F, r, T, Ds, U, R;
char *folder;

/* buffers for directory file processing */
char *work_mem, *buf0, *buf1, *buf2, *buf3, *buf4, *buf5, *bufL, *bufR, *subdirs;
char mbuffer[2048];

/* pointers to config data */
char *ABOUT, *DIR_OUTPUTS, *HTML_FILENAME, *EXCLUDE, *ROOT_PAGE, *ALIAS;
char *WWW, *REPOS, *REDIRECT, *READ_TITLES, *DESCRIPT_EXT, *DESCRIPTION, *EPILOG;

/* positions in a row */
int d_date, d_time, d_size, d_name;
/* width in a row */
int w_date, w_time, w_size;
/* position of  ":" in file time */
int p_time, p_date;

char *SzStr = " KMGTP   ";	// bytes, Kbytes, Mbytes, ...

/* helping functions */

/* skips tag row */
char *skip_row() {
	char *g = p;
	while(*g!=0 && *g!=13 && *g!=10) g++;
	while(*g==13 || *g==10) g++;
	return g;
}

/* trim */
void alltrim(char *str) {
	char *v = str, *g = v;
	while( *v==32 || *v==8 ) v++;
	while( *v!=0 ) (*g++) = (*v++); *g=0;
	while( (*--g)==32 || *g==8 ) *g=0;
}

/* reads string into buffer, uses global pointer p */
int get_str_row( char *str, int trim ) {
	char *g = str;
	*g=0;
	if(*p=='[' || *p==0) return 0;
	while(*p!=0 && *p!=13 && *p!=10) (*g++) = (*p++);
	*g=0;
	while(*p==13 || *p==10) p++;
	if(trim) alltrim( str );
	return 1;
}

/* reads string into bufL, bufR, "="-split, uses global pointer p */
int get_equ_row() {
	char *g;
	int i;
	
	for(i=0; i==0; ) {
	
	g = bufL;
	i=0;
	if(*p=='[' || *p==0) break;
	while(*p!=0 && *p!=13 && *p!=10) {
		*g = *p;
		if(*p=='=') {
			*g=0; g=bufR-1; i++;
			}
		g++; p++;
		}
	*g=0;
	}
	while(*p==13 || *p==10) p++;
	alltrim( bufL );
	return i;
}

/* folder to path */
void rep_slash( char *str ) {
	char *m = str, *v=m+1;
	if(*v==':') *m = toupper(*m);
	while(*m!=0) {
		if(*m=='\\') *m='/';
		m++;
	}
}

/* add folder slash, if need */
void rep_add_slash( char *str ) {
	char *m = str;
	if(*m!=0) {
		while(*m!=0) m++;
		if(*(--m)!='/') {
			*(++m)='/';
			*(++m)=0;
			}
		}
}

/* copies part of string (n chars) */
char *nstrcpy( char *buf_t, char *buf_f, int n ) {
	char *f = buf_f, *t = buf_t;
	while((n--)>0) (*t++) = (*f++);
	*t=0;
	return buf_t;
}

/* calculates file size text */
void calc_size ( char *buf ) {
	char *b = buf, *f = buf, *s = SzStr;
	int i=0, n=0;
	while(*f!=0) {
		if(*f==-1 || *f==255 || *f==',' ) { i++; s++; }
		f++;
		}
	f = buf;
	while(*f!=0) {
		if(*f=='<' || *f==-1 || *f==255 || *f==',' ) break;
		if(isdigit(*f)) {
			(*b++) = *f; n++;
			}
		f++;
	}
	*b=0;
	if(n>0) {
		(*b++) = ' ';
		(*b++) = *s;
		*b=0;
	}
	
}

/* converts local path to visible Alias name */
char *toAlias( char *buf_t, char *buf_f ) {
	char *f = buf_f, *t = buf_t;
	char *p2, *g2, *m, *l, *h, *w, *z;
	int b;
	
	strcpy(t,f);
	
	p2 = p; g2 = g;
	bufL+=1*1024*1024;		// buffers are large, so skips some space for free bytes
	bufR+=1*1024*1024;
	
	p = ALIAS;
	while (p!=NULL && get_equ_row()>0) {
		m = bufL;
		rep_slash(m);
		b = strlen(m);
		if(strstr(t,m)!=NULL) {
				l = f; z = t;
				while( strncmp(l,m,b)!=0 ) {
					l++; z++;
					}
				w = l+b;
				h = bufR;
				alltrim(h);
				strcpy(z,h);
				strcat(t,w);
				break;
		}
	}
	
	p = p2; g = g2;
	bufL-=1*1024*1024;
	bufR-=1*1024*1024;
	
	alltrim(t);
	return t;
}

/*
	Try to read title from htm files.
*/

void getTitle( char *buf, char *htmfile ) {
	
	char *z, *v = buf;
	
	*v = 0;
	
	if(T==0 || strstr(htmfile,".htm")==NULL) return;
	
	if( (f2 = fopen(htmfile,"r"))==NULL ) return;
	
	z = htmS;
	while( !feof(f2) ) *(z++) = fgetc(f2);
	fclose(f2);
	*z = 0;
	
	for(z = htmS; *z!=0 ; z++) {
		if(strncmp(z,"<TITLE>",7)==0 || strncmp(z,"<title>",7)==0 ) {
			z+=7;
			while(*z!='<' && *z!=0) {
				if(*z!=13 && *z!=10) *(v++)=*z;
				z++;
				}
			break;
			}
		}
	*v=0;
}

/*
 Config file into memory, sets pointers.
*/

int read_config( char *cfg_filename ) {
	
	if( (f = fopen(cfg_filename,"r"))==NULL ) return 0;
	p = q = config;
	while( !feof(f) ) *(p++) = fgetc(f);
	fclose(f);
	*p = 0;
	if((*config)==0) return 0;

	ABOUT = DIR_OUTPUTS = HTML_FILENAME = EXCLUDE = ROOT_PAGE = ALIAS = NULL;
	WWW = REPOS = REDIRECT = READ_TITLES = DESCRIPT_EXT = DESCRIPTION = EPILOG = NULL;
	for(p = q; *p!=0 ; p++) {
		if(strncmp(p,"[ABOUT]",7)==0) ABOUT = skip_row();
		if(strncmp(p,"[DIR_OUTPUTS]",13)==0) DIR_OUTPUTS = skip_row();
		if(strncmp(p,"[HTML_FILENAME]",15)==0) HTML_FILENAME = skip_row();
		if(strncmp(p,"[EXCLUDE]",9)==0) EXCLUDE = skip_row();
		if(strncmp(p,"[ROOT_PAGE]",11)==0) ROOT_PAGE = skip_row();
		if(strncmp(p,"[ALIAS]",7)==0) ALIAS = skip_row();
		if(strncmp(p,"[WWW]",5)==0) WWW = skip_row();
		if(strncmp(p,"[REPOS]",7)==0) REPOS = skip_row();
		if(strncmp(p,"[REDIRECT]",10)==0) REDIRECT = skip_row();	
		if(strncmp(p,"[READ_TITLES]",13)==0) READ_TITLES = skip_row();
		if(strncmp(p,"[DESCRIPT_EXT]",14)==0) DESCRIPT_EXT = skip_row();		
		if(strncmp(p,"[DESCRIPTION]",13)==0) DESCRIPTION = skip_row();
		if(strncmp(p,"[EPILOG]",8)==0) EPILOG = skip_row();
	}

	return 1;
}

/* finds folder in the list */
int folderN( char *name ) {
	
	char *q = subdirs;
	int i;
	
	for(i=0; i<cnt; i++) {
		if(strcmp(q, name)==0) { folder = q; break; }
		while(*q!=0) q++;
		q++;
		}
	return i;
}

/* finds parent folder back in the list */
int folderBack() {
	
	char *q = subdirs, *f = folder;
	int a,b, i, n=0;
	a = strlen(f);
	
	for(i=0; i<cnt; i++) {
		b = strlen(q);
		if(b<a && strncmp(f,q,b)==0) n = i;
		while(*q!=0) q++;
		q++;
		}
	return n;
}

/* finds 1st folder starting with pth */
int folder1st( char *pth ) {
	
	char *q = subdirs, *f = buf4, *t = buf5;
	int a, i, n=0;
	
	strcpy(f,pth);
	rep_add_slash(f);
	a = strlen(f);

	for(i=0; i<cnt; i++) {
		strcpy(t,q);
		rep_add_slash(t);
		if(strcmp(t,f)==0) { n = i; break; }
		while(*q!=0) q++;
		q++;
		}
	if(n==0) {
	 q = subdirs;
	 for(i=0; i<cnt; i++) {
		strcpy(t,q);
		rep_add_slash(t);
		if(strncmp(t,f,a)==0) { n = i; break; }
		while(*q!=0) q++;
		q++;
		}
	}
	return n;
}

/* found new folder in the directory output */
void new_folder( char *name ) {
	
	char *q = subdirs;
	int i = folderN( name );
	if(i!=cnt ) return;	// already
	
	for(i=0; i<cnt; i++) {
		while(*q!=0) q++;
		q++;
		}
	cnt++;

	strcpy(q,name);
	folder = q;
}


/*
 dir /s > output.txt generates known structured text file
*/
int read_dirfile( char *filename ) {

	char *w,*d, *ps, *fs, *p2, *g2, *g, *P;
	int i, j;

	if( (f = fopen(filename,"r"))==NULL ) return 0;

	ps = p;
	
	p = dir;
	while( !feof(f) ) *(p++) = fgetc(f);
	fclose(f);
	*p = 0;
	
	d_date = d_time = d_size = d_name = p_time = p_date = -1;
	w_date = w_time = w_size = 0;
	
	p = dir;
	while ( get_str_row( buf1, 0 ) ) {
		i=0; P = g = buf1;
		while(*g!=0) {
			/* is it date? */
			if(d_date==-1 && (*g=='.' || *g=='/' || *g=='-')) {
				p_date = i;
				v = g+3;
				if(*v==*g) {
					while(i>0 && *g!=32 && *g!=8 && *g!=13 && *g!=10) {
						i--; g--;
						}
					d_date = i; v = g;
					while(*v!=32 && *v!=8 && *v!=13 && *v!=10) {
						v++; w_date++;
					}
					
					printf("pos-date: left:%d, width:%d\n",d_date,w_date);
					i=0; g=P;
				}
			}
			
			/* is it time? */
			if(d_date!=-1 && d_time==-1 && *g==':') {
				v = g-1; w = g+1; m = g+3;
				if( isdigit(*v) && isdigit(*w) && (*m==32 || *m==':') ) {		/* 0-9 */
					p_time = i;
					while(i>0 && *g!=32 && *g!=8 && *g!=13 && *g!=10) {
						i--; g--;
						}
					d_time = ++i; v = ++g;
					while(*v!=32 && *v!=8 && *v!=13 && *v!=10) {
						v++; w_time++;
					}
					if( strncmp(v," AM",3)==0 || strncmp(v," PM",3)==0 ) w_time+=3; 			
					
					printf("pos-time: left:%d, width:%d\n",d_time,w_time);
					i=0; g=P;
				}
			}
			
			/* is it size, same as <DIR>, < other >? */
			if(d_size==-1 && *g=='<') {
				v = g; r = 0;
				while(*v!=13 && *v!=10 && *v!=0) {
					if(*(v++)=='>') {
						d_size = i; w_size = r;
						while(*v==32 || *v==8) {
							v++; w_size++;
							}
						printf("pos-size: left:%d, width:%d\n",d_size,w_size);
							/* The filename is here */
						if(d_name==-1) {
							d_name = (++i)+w_size;
							printf("pos-name: left:%d\n",d_name);
						}
						i=0; g=P;
						break;
					}
					r++;
				}
			}
			
			g++; i++;
		}

	}
	
	/* if values not set, then use known */
	if(d_date==-1 || d_time==-1 || d_size==-1 || d_name==-1) {
		printf("Can not split by structure the dir /s file, sorry.\n");
		}
	else
	{
	
	/* Make a list of folders at first */
	p = dir;
	while ( get_str_row( buf1, 1 ) ) {
	  g = buf1;
	  if(*g!=0) {
		w = g;
		while(*w!=0) {
			if(strncmp(++w,"irectory of ",12)==0) {
				w+=12;
				rep_slash(w);
				new_folder(w);
				}
			}
		}
	}

	/* Process all folders and files */
	p = dir;
	while ( get_str_row( buf1, 1 ) ) {
	  g = buf1;
	  if(*g!=0) {
		w = g;
		while(*w!=0) {
			if(strncmp(++w,"irectory of ",12)==0) {
				w+=12;
				rep_slash(w);
				c = folderN(w);
				
					/* if should ignore this folder */
				p2 = p; g2 = g;
				p = EXCLUDE;
				e = 0;
				while (p!=NULL && get_str_row( buf3, 1 ) ) {
					g = buf3;
					rep_slash(g);
					if(*g=='[') break;
					if(*g!=0) {
						if(strstr(w,g)!=NULL) e = 1;
					}
				 }
				p = p2; g = g2;
				D = 0;
				
				if(e==1) {		/*skip this folder*/
					for(;;p++) {
						if(*p==0 || strncmp(p,"ile(s)",6)==0) break;
						}
					while(*p!=0 && *p!=13 && *p!=10) p++;
					while(*p!=0 && *p==13 && *p==10) p++;
					break;
				}
				else
				{
				
fprintf(fo, "<div id=\"fo_%d\" class=\"FD\">\n", c);
fprintf(fo, "<h1>Index of %s</h1>\n", toAlias(buf3,w) );

				U = 0;
				p2 = p; g2 = g;
				p = REPOS;
				while (p!=NULL && get_equ_row()>0) {
					g = bufL;
					rep_slash(g);
					if(strstr(w,g)!=NULL) {
						u = bufR; alltrim(u);
if(U==0) fprintf(fo, "<h3>");
else fprintf(fo, " ,  ");
						U = 1;
						rep_slash(u);
fprintf(fo, "<a href=\"%s\" target=\"_blank\">%s</a>", u, toAlias(buf3,u) );
						}
				}
				p = p2; g = g2;
if(U) fprintf(fo, "</h3>\n");

fprintf(fo, "<table><tr><td></td><td><font color=\"blue\"><b>Name</b></font></td>");
fprintf(fo, "<td align=\"right\"><font color=\"blue\"><b>Last modified</b></font></td>" );
fprintf(fo, "<td></td><td width=20></td><td><font color=\"blue\"><b>Size</b></font></td>" );
fprintf(fo, "<td width=20></td><td><font color=\"blue\"><b>Description</b></font></td>" );
fprintf(fo, "</tr>\n");
fprintf(fo, "<tr><th colspan=\"8\"><hr></th></tr>\n");
				D = 1;
				}
				}
			if(D==1 && strncmp(w,"ile(s)",6)==0) {
fprintf(fo, "</table>\n");
fprintf(fo, "</div>\n", c, w);
				D = 0;
				}
			}
		w = g+p_time; d = g+p_date;
		if(*w==':' && (*d=='.' || *d=='/' || *d=='-') )	{
			/* this is a file or folder */
			w = g+d_size;
			F = ( *w=='<' ? 1 : 0);
			
			w = g+d_name;
			strcpy(buf0,w);
			c2 = 0;
			
			/* checking file path */
			
				strcpy(buf2, folder);
				strcat(buf2, "/");
				strcat(buf2, g+d_name);
				rep_slash(buf2);
				p2 = p; g2 = g;
				p = EXCLUDE;
				e = 0;
				while (p!=NULL && get_str_row( buf3, 1 ) ) {
					g = buf3;
					rep_slash(g);
					if(*g=='[') break;
					if(*g!=0) {
						if(strstr(buf2,g)!=NULL) e = 1;
					}
				 }
				p = p2; g = g2;
				
				if( e==0 && strcmp(".",w)==0 ) e=1;
				if( e==0 && strcmp("..",w)==0 )	{
					e=1;
					c2 = folderBack();
					if(c2<c) e=2;		/* back folder */ 
					}
				
				if(e==0 || e==2) {
					/* good, can be */
					
fprintf(fo, "<tr><td>");
if(e==2) {

fprintf(fo,"<img src=\"b.gif\"></td><td><div class=\"FN\" onclick=\"SW(%d,%d)\"> .. </div></td>", c, c2 );
}
else if(F) {

printf("%s\n",buf2);
	
fs = folder; c2 = folderN(buf2); folder = fs;

fprintf(fo,"<img src=\"f.gif\">");
fprintf(fo, "</td><td><div class=\"FN\" onclick=\"SW(%d,%d)\"> %s </div></td>", c, c2, toAlias(buf3, buf0) );

}
else {

fprintf(fo,"<img src=\"%s.gif\">" , ( strstr(buf0,".htm")==NULL ? "u" : "w") );

				p2 = p; g2 = g;
				
				*buf4=0;
				p = REDIRECT;
				while (p!=NULL && get_equ_row()>0) {
					g = bufL;
					rep_slash(g);
					if(strcmp(buf2,g)==0) {
						u = bufR;
						alltrim(u);
						rep_slash(u);
						strcpy(buf4,u);
						}
				}

				if( *buf4==0 ) {
				
				p = WWW;
				while (p!=NULL && get_equ_row()>0) {
					g = bufL;
					rep_slash(g);
					if(strstr(buf2,g)!=NULL) {
						u = bufR;
						alltrim(u);
						rep_slash(u);
						strcpy(buf4,u);
						strcat(buf4,buf2+strlen(bufL));
						}
				}
				}
				
				p = p2; g = g2;

fprintf(fo, "</td><td><a href=\"%s\" target=\"_blank\">%s</a></td>", buf4, toAlias(buf3, buf0) );
}

fprintf(fo, "<td align=\"right\">%s</td>", nstrcpy( buf3, g+d_date,w_date ) );
fprintf(fo, "<td><font size=2>%s</font></td>", nstrcpy( buf3, g+d_time,w_time ) );

calc_size( nstrcpy( buf3, g+d_size,w_size ) );
fprintf(fo, "<td></td><td>%s</td>", buf3);

getTitle( buf3, buf2 );

				p2 = p; g2 = g;
				p = DESCRIPT_EXT;
				Ds = 0;
				while (p!=NULL && get_equ_row()>0) {
					g = bufL;
					rep_slash(g);
					u = buf0;
					while(*u!=0) {
					 if(strcmp(u,g)==0) {
						Ds = 1;
						strcat(buf3,bufR);
						break;
						}
					 u++;
					}
				}
				p = DESCRIPTION;
				while (p!=NULL && get_equ_row()>0) {
					g = bufL;
					rep_slash(g);
					if(strstr(buf2,g)!=NULL) {
						Ds = 1;
						strcat(buf3,bufR);
						}
				}
				p = p2; g = g2;

fprintf(fo, "<td></td><td>%s</td></tr>\n", buf3 );	

				}
			}
		}
	}
	
	}
	
	p = ps;

	return 1;
}

int exitprog() {

	printf("\nPress a key...\n");
	getch();
	return 0;
}

/*
 Sitemap creator
*/

int main(int argc, char **argv) 
{
	
	if (argc < 2) {
		printf("A simplest web sitemap filebrowser creator.\n\nusage: sitemap <config_file.ini>\n");
		return exitprog();
	}

		/* do it in a lots of memory */
	work_mem = (char *)malloc(80*1024*1024);
	if( work_mem == NULL ) {
		printf("Can not locate free memory 80Mb?!\n");
		return exitprog();
		}

	/* memory buffers */
	p = work_mem;
	config = p; *p=0; p+=5*1024*1024;
	dir = p; *p=0; p+=20*1024*1024;
	folder = p; *p=0; p++;			/* empty on start */
	buf0 = p; *p=0; p+=1*1024*1024;
	buf1 = p; *p=0; p+=1*1024*1024;	/* some buffers */
	buf2 = p; *p=0; p+=1*1024*1024;
	buf3 = p; *p=0; p+=1*1024*1024;
	buf4 = p; *p=0; p+=1*1024*1024;
	buf5 = p; *p=0; p+=1*1024*1024;
	bufL = p; *p=0; p+=2*1024*1024;		/* ... till "=" */
	bufR = p; *p=0; p+=2*1024*1024;		/* "=" after ... */
	subdirs = p;  *p=0; p+=10*1024*1024;	/* the list of all folders */
	htmS = p;
	
	cnt = D = T = R = 0;
	
	if( read_config( argv[1] )==0 ) {
		printf("No config.\n");
		return exitprog();
		}
		
	/* processes directories outputs */
	p = HTML_FILENAME;
	e = 0;
	if(p!=NULL)
	 while ( get_str_row( mbuffer, 1 ) ) {
		m = mbuffer;
		if(*m!=0) {
		printf("writing: %s\n",m);
		e = 1;
		break;
		}
	}
	if(e==0 || strstr(m,".htm")==NULL ) {
		printf("No html-file name.\n");
		return exitprog();	
		}
	
	if((fo = fopen(m,"w"))==NULL ) {
		printf("Can not create a file.\n");
		return exitprog();	
	}

fprintf(fo, "%c%c%c",0xEF,0xBB,0xBF);		/*UTF-8*/
fprintf(fo, "<html>\n<head>\n<title>Index Of</title>\n");
fprintf(fo, "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\">\n");
fprintf(fo, "<style type=\"text/css\">\n");
fprintf(fo, "DIV.FN{ cursor:pointer; text-decoration: bold; color: black;}\n");
fprintf(fo, "DIV.FD{ visibility:hidden;position:absolute;left:12px;top:12px }\n");
fprintf(fo, "</style>\n");
fprintf(fo, "</head>\n<body onload=\"loaded()\">\n");
fprintf(fo, "<!-- sitemap tool, %s -->\n", strdate(buf0));
fprintf(fo, "<script type=\"text/javascript\">\n");
fprintf(fo, "function GE(i) { return document.getElementById(i); }\n");
fprintf(fo, "function loaded() { SW(0,0); }\n");
fprintf(fo, "function SW(f,t) { var o=GE(\"fo_\"+f),w=GE(\"fo_\"+t); o.style.visibility=\"hidden\";");
fprintf(fo, "document.body.scrollTop=document.documentElement.scrollTop=w.scrollTop=w.scrollLeft=0;");
fprintf(fo, "w.style.visibility=\"visible\"; }\n");
fprintf(fo, "</script>\n");
	
	/* title read or not? */
	p = READ_TITLES;
	if(p!=NULL)
	 while ( get_str_row( buf1, 1 ) ) {
	  g = buf1;
	  if(*g!=0 && (*g=='1' || *g=='y' || *g=='Y' )) T=1;
	}
	
	/* root page or not? */
	p = ROOT_PAGE;
	if(p!=NULL)
	 while ( get_str_row( buf1, 1 ) ) {
	  g = buf1;
	  if(*g!=0 && (*g=='1' || *g=='y' || *g=='Y' )) R=1;
	}

	if(R) new_folder("<root_page>");
			
	/* processes directories outputs */
	p = DIR_OUTPUTS;
	if(p!=NULL)
	 while ( get_str_row( buf1, 1 ) ) {
	  g = buf1;
	  if(*g!=0) {
		printf("dir /s >%s\n",g);
		if( read_dirfile(g)==0 ) printf("No file?\n");
		}
	}
	
	/* add the root page at the end */
	if(R) {
			printf("Creating the root page.\n");
			
fprintf(fo, "<div id=\"fo_0\" class=\"FD\">\n");
fprintf(fo, "<h1>Sitemap root /</h1>\n");
fprintf(fo, "<table><tbody>\n");

				p = ALIAS;
				while (p!=NULL && get_equ_row()>0) {
					g = bufL;
					rep_slash(g);
					printf("%s\n",g);
					c = folder1st(g);
fprintf(fo, "<tr><td>");
fprintf(fo,"<img src=\"f.gif\">");
fprintf(fo, "</td><td><div class=\"FN\" onclick=\"SW(%d,%d)\"> %s </div></td>", 0, c, toAlias(buf3, bufR) );
fprintf(fo, "</tr>\n");
				}

fprintf(fo, "\n</tbody></table>\n");
fprintf(fo, "</div>\n");
		
		}
	
fprintf(fo, "</body>\n<html>\n");
	
	//free(&work_mem);
	fclose(fo);
	
	return exitprog();
}
