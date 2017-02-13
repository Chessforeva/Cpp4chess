
/*
 Obfuscator
 Place after minfy.c
 */

CHAR keyword[100];
CHAR kbuf[512],kbf[512];	// small buffers
CHAR *GKp;			// pointer to read from list
CHAR GKc;			// contains delimiter 1 or 0-end
CHAR G0,G1;			// chars around keyword in the text
CHAR *GList;		// pointer to klist
CHAR *Gsrc;			// pointer to src
UINT Glen;			// length by user
long Gsrclen;		// source len
UINT Gcnt;			// count of keyword in source
BOOL Gfunc;			// is function 
BOOL Gvary;			// is variable
BOOL Gobjc;			// is object
BOOL Gprop;			// is object property
BOOL Ghtml;			// is html internal
CHAR Gdesc[8];
UINT GN;			// counts inserted
CHAR gencd[15], gcdp[15];		// codes to generate
CHAR *kw_new,*kw_old;	// pointers to keywords and new
UINT l_new,l_old;		// length
int j,v;				// some loop variables

// looks for keyword before this position in text
void SeeKeywordPre()
{
 UINT l;
 CHAR *p=mPre, c=*p, *q=keyword;
 for(l=0; KEYWORDCHR(c) && l<99; c=*(--p),l++);
 c=*(++p);
 if(l>0 && DIGIT(c)) l=0;
 for( ;l>0; c=*(++p), l--) *(q++) = c;
 *q=0;
}

void GK()		// get in kbuf the current data from list
{
	CHAR *p=GKp;
	GKc=*p;
	for(;GKc!=0 && GKc!=1; GKc=*(++GKp));
	if(GKc==1) *GKp=0;
	strcpy(kbuf,p);
	*(GKp++)=GKc;
}

// is keyword already in the list?
BOOL wasKeyWord()
{
 for(GKp=GList, GKc=*(GKp); GKc; )
	{
	GK();GK();		// read newname,keyword
	if(strcmp(keyword,kbuf)==0) return TRUE;
	GK();GK();		// read count, descr
	}
 return FALSE;
}

// finds a record count to insert before
BOOL sortInsert()
{
 UINT N;
 CHAR *p;
 for(GKp=GList, GKc=*(GKp); GKc; )
	{
	p= GKp;
	GK();GK();		// read newname,keyword
	GK(); N = atoi(kbuf);	// read count
	GK();		// descr
	if(Gcnt>N) { GKp = p; break; }
	}
 return (*GKp!=0);
}

void GenNwCode()
{
	CHAR *p = &gencd[12], c;
	for(;;)
	{
		c=*p;
		switch(c)
		{
		case ' ': { *p='a'; return; }
		case 'z': { *p='A'; break; }
		case 'Z': { *p=(*(p-1)==' ' ? 'a' : '0'); break; }
		case '9': { *p='a'; break; }
		default: { *p=(++c); break; }
		}
		if(*p=='a') { if(*(--p)==' ') { *p='a'; if(*(p-1)==' ') break; } }
		else break;
	}
}
// gen. and validate for known 2 byte keywords
void GenValidNwCode()
{
 CHAR *p;
 for(;;)
  {
  GenNwCode();
  p = gencd;
  while(*p==' ') p++;
  if(strcmp(p,"in")!=0 && strcmp(p,"of")!=0 && strcmp(p,"if")!=0) break;
  } 
}

// generates all new keywords
void genKwNwCodes()
{
 CHAR *p,*q,c,*w,*u;
 BOOL b;
 UINT L;
 for(GKp=GList, GKc=*(GKp); GKc; GKc=*(GKp))
	{
	p= GKp;
	GK();GK();		// read newname,keyword
	L = strlen(kbuf);
	b = (L>=Glen);
	strcpy(kbf,kbuf);		// save
	GK();GK();		// count,descr
	b = b & (kbuf[0]!='I');		// without internal html keywords
	if(b)
		{
		strcpy(gcdp,gencd);				// save
		for(u=GKp;;)				// loop till code is not in list
			{
			GenValidNwCode();
			for(q=gencd,w=keyword,c=*q;;c=*(++q))		// copy to keyword without spaces
				{
				if(c!=' ') *(w++)=c;
				if(!c) break;
				}
			if(!wasKeyWord()) break;
			}
		if(strlen(keyword)<L) memcpy(p,gencd,13);		// good, save
		 else strcpy(gencd,gcdp);		// restore
		GKp=u;
		}
	}
}

void isProtokbuf()
{
	Gfunc = Gfunc | (strstr(kbuf,"prototype")!=NULL);
}

// looks in kbuf for html known texts (some known)
void isHtmlkbuf()
{	
	Ghtml = Ghtml | ( strstr(kbuf,"document")!=NULL || strstr(kbuf,"window")!=NULL || strstr(kbuf,"style")!=NULL ||
				strstr(kbuf,"navigator")!=NULL  || strstr(kbuf,"innerHTML")!=NULL );
}


void kwHTML()
{
	isHtmlkbuf();
	strcpy(kbuf,keyword);
	isHtmlkbuf();
}

// is this function?
void gKWfunct( CHAR *p )
{
 CHAR *q=mSp, c;
 mSp=p; c=*p;
	// see "function " before
 for(c=*(--mSp); CRLF(c) || TABSPC(c); c=*(--mSp));		// skip tabs
 mSp++;
 if( comesafter("function") ) Gfunc = TRUE;
 mSp=p; c=*p;
 
	// see ":function()" after
 for(; KEYWORDCHR(c); c=*(++mSp));	// look for first tabs
 for(; CRLF(c) || TABSPC(c); c=*(++mSp));		// skip tabs
 
 if(c==':' || c=='=')
		{
		for(c=*(++mSp); CRLF(c) || TABSPC(c); c=*(++mSp));
		if( startswith("function") ) Gfunc = TRUE;
		}
 mSp=q;
}

// counts keywords
// Actually, it is incorrect, because it scans all the text including javascript comments and html tags
//
void gKWcount()
{
 CHAR *p=Gsrc, c=*p, *q,*w, c1=keyword[0];
 UINT l = strlen(keyword), k;
 for(Gcnt=0;c!=0; c=*(++p))			// this is slow for larger javascripts, maybe can index somehow
  {
	if(c==c1 && memcmp(p,keyword,l)==0)	// if keyword here
		{
		G0=*(p-1); G1=*(p+l);
		if(!KEYWORDCHR(G0) && !KEYWORDCHR(G1))
		 {
			gKWfunct(p);
			Gcnt++;			// if part of other keyword
			if(G0=='.')
			 {
			 Gprop = TRUE;
			 q=p-1; for(c=*q, k=0; c=='.' || KEYWORDCHR(c); c=*(--q),k++ );
			 memcpy(kbuf,++q,k); kbuf[k]=0;
			 isProtokbuf();
			 kwHTML();
			 }
			if(G1=='.')
			 {
			 Gobjc = TRUE;
			 q=p+l; w=q; for(c=*q, k=0; c=='.' || KEYWORDCHR(c); c=*(++q),k++ ) ;
			 memcpy(kbuf,w,k); kbuf[k]=0;
			 kwHTML();
			 }
		 }
		}
  }
}


/* prepares keywords

 Params: string src is javascript text,
  klist is a pointer to place keywords as strings (toname,name,description,count)
 +user option flags:
  to look function name keywords
  to look variable name  keywords
  to look all kinds of keywords
  min.len. of keywords
  window handle for %xx disp.
*/


void PrepKeywords( CHAR* src, CHAR *klist, BOOL Rfunc, BOOL Rvars, BOOL Rall, UINT kwlen, HWND inforwin )
{

 CHAR c,*u, q;
 BOOL b;
 GList = klist; Gsrc = src; GN = 0; Glen = kwlen; Gsrclen = strlen(src);
 
 mSp=src;
 mTp = NULL;
 mS1 = FALSE; mS2 = FALSE; mS3 = FALSE;

 sprintf(gencd, "%13s","");
	 
 CkJS(); // is javascript
 
 for(mSp=src, mPre = mSp,c=*mSp; c!=0; c=*(++mSp) )
 {
				
  if(!mJS && startswith_all_case("<SCRIPT"))
   {
    mJS = TRUE; SkipTo('>'); mJS=(*(mSp-1)!='/'); continue;     // here the script starts
   }
  if(mJS && startswith_all_case("</SCRIPT"))
   {
    SkipTo('>'); mJS = FALSE; continue;               // script ends
   }

  if(!mJS)	// if html then do nothing
   {
    continue;
   }

  if(c==39 && !mS2) mS1 = !mS1;   // '
  if(c==34 && !mS1) mS2 = !mS2;   // "

  if(!(mS1 || mS2 || mS3))
    {

			// remove comments
    if(startswith("//"))
			{
			for(;c!=0 && !CRLF(c); c=*(++mSp));
			if(c==13) c=*(++mSp);	// skip LFs
			if(c==10) c=*(++mSp);
		
			mSp--;
			continue;
			}
			}

    if(startswith("/*"))
			{
			for(;c!=0; c=*(++mSp))
			 {
			 if(c=='/' && *(mSp-1)=='*') break;
			 }
			continue;
			}
		
    if(c=='/')    // try skip regexp
			{
			if(pre4regexp())
				{
				SkipTo('/'); continue;
				}
			}

	// new keyword possible, loofor functions only
	if( (Rall && !(CRLF(c) || TABSPC(c)) && !KEYWORDCHR(c))  || (c==':' || c=='=' || c=='('))
	 {
 
		SeeKeywordPre();			// see, what a keyword is here
		if( keyword[0]!=0 && !wasKeyWord() )
		{
			Gfunc = FALSE; Gvary = FALSE; Gobjc = FALSE; Gprop = FALSE; Ghtml = FALSE;

			gKWcount();		// counts keyword appearance in javascript text
			if(c!='(' || Gfunc)
				{
				if(!Gfunc && c!='(') Gvary = TRUE;

				sprintf(Gdesc, "%c%c%c%c%c", (Ghtml?'I':' '), (Gfunc?'F':' '), (Gvary?'V':' '),
					(Gobjc?'O':' '), (Gprop?'P':' ') );
				sprintf(kbf, "%s%c%s%c%8d%c%s%c",  gencd ,1, keyword,1, Gcnt, 1, Gdesc,1 );
				if( sortInsert() )
					{
					j = strlen(kbf);
					v = strlen(GKp);
					memmove( GKp+j, GKp, v+1);
					memcpy( GKp, kbf, j );
					}
				else strcat(klist, kbf); 
				GN++;

				sprintf(kbuf, "%d%c found %d", 100*(mSp-src)/Gsrclen, '%', GN);
				SendMessage(inforwin, WM_SETTEXT, 0, (LPARAM)kbuf);
				}
		}
	 }

	if(!(CRLF(c) || TABSPC(c))) mPre = mSp;

 }

 //generate codes
 genKwNwCodes();
 
}

/* looks for keyword */
void tryReplaceKeywordHere()
{
 CHAR *p=mSp;
 if(memcmp(p,kw_old,l_old)==0)	// if keyword here
	{
	G0=*(p-1); p+=l_old; G1=*p;
	if(!KEYWORDCHR(G0) && !KEYWORDCHR(G1))		// todo: should verify variables and functions aa() or bb=...
		{
		if(l_new!=l_old)
			{
			p = mSp+l_old;
			memmove( mSp + l_new, p, strlen(p)+1 );
			}
		memcpy( mSp, kw_new, l_new );			// replace with new
		mSp+=l_new;
		}
	}
}

/*
 performs replacement keyword->new keyword
 
 todo:   isfunc is not used, anyway can verify for "(" after function name
 */
void Obfuscate( CHAR* src, CHAR *kwnew, CHAR *kwrd, BOOL RinHtml, BOOL isfunc )
{
 CHAR c,c2=0;

 mSp=src;
 mTp = NULL;
 mS1 = FALSE; mS2 = FALSE; mS3 = FALSE;
 kw_new = kwnew; kw_old = kwrd;
 l_new = strlen(kw_new);
 l_old = strlen(kw_old);
	 
 CkJS(); // is javascript
 
 for(mSp=src, mPre = mSp,c=*mSp; c!=0; c=*(++mSp) )
 {
				
  if(!mJS && startswith_all_case("<SCRIPT"))
   {
    mJS = TRUE; SkipTo('>'); mJS=(*(mSp-1)!='/'); continue;     // here the script starts
   }
  if(mJS && startswith_all_case("</SCRIPT"))
   {
    SkipTo('>'); mJS = FALSE; continue;               // script ends
   }

  if(RinHtml)	// see for on...="
	{
	 if(c==c2) { c2 = 0; mJS = FALSE; }
	 else
	 {
	  if(!mJS && (c==39 || c==34) && (*mPre)=='=')
		{
		c2 = c; mJS = TRUE;
		continue;
		}
	 }
	}
	
  if(!mJS)	// if html then do nothing
   {
	if(!(CRLF(c) || TABSPC(c))) mPre = mSp;
    continue;
   }

  if(c==39 && !mS2) mS1 = !mS1;   // '
  if(c==34 && !mS1) mS2 = !mS2;   // "

  if(!c2 && !(mS1 || mS2 || mS3))
    {

			// remove comments
    if(startswith("//"))
			{
			for(;c!=0 && !CRLF(c); c=*(++mSp));
			if(c==13) c=*(++mSp);	// skip LFs
			if(c==10) c=*(++mSp);
		
			mSp--;
			continue;
			}


    if(startswith("/*"))
			{
			for(;c!=0; c=*(++mSp))
			 {
			 if(c=='/' && *(mSp-1)=='*') break;
			 }
			continue;
			}
		
    if(c=='/')    // try skip regexp
			{
			if(pre4regexp())
				{
				SkipTo('/'); continue;
				}
			}
	}
	
	tryReplaceKeywordHere();			// look for keyword		
			 
	if(!(CRLF(c) || TABSPC(c))) mPre = mSp;

 }
 
}