/*

 Minifies javascript code
 This part is improvable.

 */

#define UPPCHAR(x) (((x>='a') && (x<='z')) ? x-32 : x)

#define CRLF(c) (c==13 || c==10)
// tabs (code=9), spaces, or other control chars (c==9 || c==32)
#define TABSPC(c) 	(c!=0 && c<=32 && (!CRLF(c)))
#define DIGIT(c) (c>='0' && c<='9')
#define LALPHA(c) (c>='a' && c<='z')
#define UALPHA(c) (c>='A' && c<='Z')
#define ALPHAS(c) (LALPHA(c)||UALPHA(c))
#define KEYWORDCHR(c) ( ALPHAS(c)||DIGIT(c)||(c=='$')||(c=='_'))

CHAR *mSp,*mTp;
BOOL m1st;    // first...
BOOL mJS;   // javascript code flag
BOOL mS1;   //  ' flag
BOOL mS2;   //  " flag
BOOL mS3;   //  / flag
CHAR *mPre;	// pre char position

// is this string here?
BOOL startswith_all_case( CHAR *a )
{
 CHAR d, c=*a, *p;
 for( p=mSp;  c!=0; c=*(++a))
  {
   d=*(p++); if( UPPCHAR(d)!=c ) return FALSE;
  }
 return TRUE;
}

// faster compare
BOOL startswith( CHAR *a )
{
 return ((*mSp==*a) && memcmp(mSp,a,strlen(a))==0);
}

// is this string after?
BOOL comesafter( CHAR *a )
{
 CHAR c, *pre;
 BOOL r=FALSE;
 UINT l=strlen(a);
 if(a[l-1]==*(mSp-1))
	{
	pre=mSp; mSp-=l;
	r=startswith(a);
	if(r) { c = *(--mSp); r = !KEYWORDCHR(c); }
	mSp=pre;
	}
 return r;
}

// forward till character
void SkipTo( CHAR c )
{
 for( CHAR d=*mSp; d!=0; d=*(++mSp) )
  {
    if(mJS && mTp!=NULL) *(mTp++)=d;
    if(d==c) break;
  }
}

//look for previous character
BOOL pre4regexp()
{
 CHAR *p=mSp, c=*(--p);
 for(; TABSPC(c) || CRLF(c); c=*(--p));
 return (c==0 || strchr("(:=,;",c)!=NULL );
}

// is "in " case here
BOOL testINcase()
{
 CHAR *p=mSp, c0=*(++p),c1=*(++p),c2=*(++p);
 return (UPPCHAR(c0)=='I' && UPPCHAR(c1)=='N' && !KEYWORDCHR(c2));
}

void CkJS()
{
 CHAR c;
 m1st = FALSE; mJS =FALSE;
 for( c=*mSp; !m1st && c!=0; c=*(++mSp) )
 {
  if(c<122 && c>32) { m1st = TRUE; mJS = (c!='<'); }   // check for html,xml
 }
}

/* code minifier

 Params: strings dst,src are javascript texts
 +user option flags:
  to remove spaces and tabs
  to reduce line breaks
  to remove comments
  to remove ";;" things 

 todo:
  This is a very simple program, nothing smart.
  May cause javascript errors, so, should be debugged in browser's developer tools.
 */

void Minfy( CHAR* dst, CHAR* src, BOOL R0932, BOOL R1310, BOOL Rcmnt, BOOL Rsmrt )
{
 CHAR c,*u, q;
 BOOL b,z;
 UINT n13=0;
 mS1 = FALSE; mS2 = FALSE; mS3 = FALSE;
 mSp=src; mTp=dst; mPre = mSp;

 CkJS(); // is javascript
 
 for(mSp=src,c=*mSp; c!=0; c=*(++mSp) )
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
   	*(mTp++)=c;
    continue;
   }

  if(c==39 && !mS2) mS1 = !mS1;   // '
  if(c==34 && !mS1) mS2 = !mS2;   // "

  if(!(mS1 || mS2 || mS3))
    {
    if(Rsmrt)
			{
			q = *mPre;
			if( q==';' )
			 {
			 	if( c==';' ) continue;
			 	if( c=='}' && *(mTp-1)==q ) { *(mTp-1)=c; continue; }  
			 }
			}

			// remove comments
    if(startswith("//"))
			{
			for(;c!=0 && !CRLF(c); c=*(++mSp)) if(!Rcmnt) *(mTp++)=c;
			if(!Rcmnt)		
			 {
			 	if(c==13) { *(mTp++)=c; c=*(++mSp); }	// skip LFs
			 	if(c==10) { *(mTp++)=c; c=*(++mSp); }
			 }		
			mSp--;
			continue;
			}

    if(startswith("/*"))
			{
			for(;c!=0; c=*(++mSp))
			 {
			 if(!Rcmnt) *(mTp++)=c;
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
	

    // end of line
    if(R1310 && CRLF(c))
     {
		u = mSp; mSp = mPre+1;
		if(comesafter("else")) *(mTp++)=32;
		mSp = u; c=*mSp;
		z = (c==13);
     	b = (z && *(++mSp)==10);
     	if((++n13)<12)
     	 {
				if(z && !b) mSp--;
				continue;
     	 }    	
     	n13=0;
     }


	if(!(CRLF(c) || TABSPC(c))) mPre = mSp;

    if(TABSPC(c) && (!testINcase()))
     {
     	if(R0932) c=32;		// no tabs in result
     	
     	u = mSp; mSp = mPre+1;
			b =
        comesafter("function") ||
        comesafter("var") ||
        comesafter("typeof") ||
        comesafter("instanceof") ||
        comesafter("return") ||
        comesafter("case") ||
        comesafter("new") ||
        comesafter("else") ||
        comesafter("throw") ||
        comesafter("break") ||
        comesafter("continue");
      if(!b)
         {
         	if((mSp==u) && comesafter("in"))
         	 {
         	 	*(mTp++)=c;
         	 	continue;
         	 }
         }
      mSp = u; c=*mSp;
      if(!b)
        	{
        	for(; TABSPC(c) && (!testINcase()) ; c=*(++mSp))
         	 if(!R0932) *(mTp++)=c;
        	mSp--;
        	continue;
        	}
     }

    }

  *(mTp++)=c;
 }
*mTp=0;
}
