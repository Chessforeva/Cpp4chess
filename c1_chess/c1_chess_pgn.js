/*=====================================

CHESS JAVASCRIPT OBJECT (_pgn)
use right after c1_chess.js

PGN object (chess variants, annotations)
p=_pgn.clone()            - clones empty pgn object
p.PGN(<pgn string>)       - the main functionality that generates tree from pgn-string

p.fromhist(o)             - generates pgn subtree for one main variant from given chess object
s=p.uciMoves()            - obtains uci string of moves from tree of pgn object (main variant only)

=====================================*/

/* PGN object */
/* for PGN() function usage */


_pgn = {
        
    t: [],  /*tree*/
    startFEN: _c1.sFEN, /* starting position */
    shtm: '', /* resulting htm as string after PGN function */
    ns: 0,  /*starting move number*/
    fast: 0,    /* set 1 to skip comments */
    compr: 1,    /* set 1 to compress shtm by using javascript */
    tabs: 0,     /* for comments, variants
    on_click: '_pOc',   /* onclick function */
    idk: '',        /* id number to know dhtml objects */
    canv: { Cnt: 0,
            Dist:8000,     /* distance const. between diagrams */
            Size:"200" },  /* default size of canvas */
    hd: { Event: '', Site: '', Date: '', Round: '',
          White: '', Black: '', Result: '' },    /* header */
    
    fromhist:function(o) {      /* generates tree from history */

    var B2 = o.clone();
    B2.setFEN( o.sFEN );
    var T = this.t;
    var N = this.ns;
    for(var i=0;i<o.hist.length;i++)
     {
      var h = o.hist[i];
      B2.movegen();
      var r=-1;
      for(var k=0;r<0 && k<B2.genml.length;k++)
        {
        var l=B2.genml[k];
        if(l.f==h.f && l.t==h.t && (h.p==0 || l.p==h.p)) r=k;
        }
      if(r<0) break;
      var u = B2.m2uci(h);
      T.push ( { k:r, uci:u, n:N, sm:B2.sm, t:[] } );
      T = T[0].t;
      B2.mkmove(r);
      if(B2.sm>0) N++;
     }
    
    },
    
    uciMoves:function() {       /*tree to uci move list */
    
    var s='';
    var T = this.t
    while(T.length>0)
     {
      var m=T[0];
      s+=(s.length==0 ? '' : ' ')+ m.uci;
      T=m.t;
     } 
    return s;
    },

    cutwrd:function(s) {
    
    for(var a = 0;a<s.length; a++)
     {
     var i=(" .,;)(}{][").indexOf( s.charAt(a) );
     if(i>=0) return s.substr(0,a);
     }
    return s;
    },
   
    PGN:function(s_pgn) {      /* analyses PGN string and generates the tree */
    
    if(this.idk.length==0)
	{
	this.idk= (_pgn_Rnd + _pgn_Pt.length).toString();
	}

    var B2 = _c1.clone();
    
    s_pgn = this.prepPGN(s_pgn);
    
    B2.setFEN( this.startFEN );
    
    this.t = [];        // tree of variants
    var T = this.t;

    var v=[];           // arr. of current variants
    var s=s_pgn;
    var Mn=0;
    var vf=0;               // flags
    this.shtm = '';        // html-text as result
    
    if(this.hd.White.length>0)
     {
     this.shtm += '<div class="_pgn_header">' +
         this.hd.White + ' ' + this.hd.Result + ' ' + this.hd.Black +
         ' </div><div class="_pgn_header2">(' +
         this.hd.Date + (this.hd.Event.length>0 ? ',' : '' ) + ' ' +
         ReplUrl( this.hd.Event ) + (this.hd.Site.length>0 ? ',' : '' ) + ' ' +
         ReplUrl( this.hd.Site ) + (this.hd.Round.length>0 ? ', round ' : '' ) + ' ' +
         this.hd.Round + ' )</div><br>';
     }
     
    for(var i=0; i<s.length; i++)
    {
     var cId = parseInt(this.shtm.length/this.canv.Dist)+1;
     if(v.length==0 && cId>this.canv.Cnt)
        {
        this.shtm+='<canvas id="_pgn_D_'+this.idk+cId.toString()+
            '" width="' + this.canv.Size + '" height="' + this.canv.Size +
            '" style="float:'+(cId%2==1 ? 'left' : 'right' ) +'"></canvas>';
        this.canv.Cnt=cId;
        }
        
     var c=s.charAt(i);
     if(('\\ ,.+=-/').indexOf(c)>=0) { if(this.fast==0 || v.length==0) this.shtm+=c; continue; }
     if(c=='<') { if(this.fast==0 || v.length==0) this.shtm+='&lt;'; continue; }
     if(c=='>') { if(this.fast==0 || v.length==0) this.shtm+='&gt;'; continue; }
     
     var t=this.cutwrd( s.substr(i,30) );
     
     if(c=='$' && t.length>1)
        {
        var nI = parseInt(t.substr(1));
        var nag = _ng[nI];
        if(nag==null || nag.length==0) { if(this.fast==0 || v.length==0) this.shtm+=t }
        else
            {
            if(nI<5) nag = '<b>'+nag+'</b>';
            else nag = replaceAll2(nag,['<','>'], ['&lt;','&gt;']);
            if(this.fast==0) this.shtm+=' <div class="_pgn_nag">'+nag+'</div> ';
            }
        i+=(t.length-1);    // skip            
        }
     else if( t=='Z0' || c=='%' ||
            (t.length>2 && ('  1:0 1-0 0:1 0-1 1/2-1/2 1/2:1/2 1/2 ').indexOf(' '+t+' ')>0 ))
        {
        if(this.fast==0 || v.length==0) this.shtm+=t;
        i+=(t.length-1);    // skip
        }
     else
     { 
     
     if(c=='{' || c=='(')
     {
      v.push( { c:c, i:i, T:T, n:Mn, sm:B2.sm, f:0 } );   // new variant
      if(this.fast==0)
       {
        if(T.length>0)
        {
         var Ti=T[T.length-1];
         T=Ti.t;
        }
        if(this.tabs) this.shtm+=this.someSpaces(v);
        this.shtm+='<div class="_pgn' + (c=='{' ? '2' : '3') + '">';
        if(!this.tabs) this.shtm+=c;
       } 
      continue;  
     }
     
     var v1=null;

     if(v.length>0)
      {
      v1 = v[v.length-1];   
      if((v1.c=='(' && c==')') || (v1.c=='{' && c=='}'))
        {
        if(this.fast==0)
         {
         T = v1.T;        
         for(var u=v.length-1;u>=0;u--)
          {
          v1 = v[u];
          var Tv = v1.T;
          if(Tv.length>0)
            {
            var z=Tv[Tv.length-1];
            this.setPosAndMove( B2, z );
            Mn=z.n;if(B2.sm>0) Mn++;
            break;
            }
          }     
         }    
        v.pop();        // end of last variant
        vf=1;
        if(this.fast==0)
         {
         if(!this.tabs) this.shtm+=c;  
         this.shtm+='</div>';
         if(this.tabs) this.shtm+=this.someSpaces(v);
         }
        continue; 
        }
      }
      
     if(c>'0' && c<='9')    // move number
      {
        var col='';
        for(var w=i+1;w<s.length;w++)
         {
          c=s.charAt(w);
          if(!(c==' ' || isdigit(c))) break;
         }
        if(w<s.length)
         { 
            if(s.charAt(w)=='.')
             {
              if(v.length==0 || (this.fast==0 && v1.f<3))
               {
                var Was={n:Mn,sm:B2.sm};
                var nm = parseInt(s.substr(i,w-i));
                Mn=nm;

                if(v.length==0 && this.ns==0) this.ns = Mn;   // number starts from
                if(v.length>0)
                 {
                  if(s.substr(w,2)=='..') { if(B2.sm>0) B2.sm=-B2.sm; }
                  else { if(B2.sm<0) B2.sm=-B2.sm; }
                  if(v1.f==0)
                   {
                   v1.n = Mn; v1.sm = B2.sm; v1.f=1;    // starting point for variant
                   }
                  if((Was.n==Mn && Was.sm>B2.sm) || (Was.n<Mn)) v1.f=3; //move point ahead
                  
                  for(var u=v.length-1;v1.f<2 && u>=0;u--)
                   {
                   v1 = v[u];
                   var Tq = v1.T;
                   if(Tq.length>0)
                    {
                    for(var j=Tq.length-1;j>=0;j--)
                     {
                      var Tw=Tq[j];
                      if((Tw.n==Mn-1 && Tw.sm<B2.sm) || (Tw.n==Mn && Tw.sm>B2.sm))
                        {
                         this.setPosAndMove( B2,Tw );
                         u=-99;
                         break;
                        }
                     }
                    }
                   }
                  if(u>-99 && u<0)
                      {
                      if(Mn==this.ns) B2.setFEN( this.startFEN );
                      }      // don't know which move                                    
                 }
               }
              col = (s.substr(w,2)=='..' ? 'b' : 'w');
              for(;w<s.length && s.charAt(w)=='.';w++); 
             } 
         }
         
        if(this.fast==0 || (v.length==0 && col=='w')) this.shtm+=s.substr(i,w-i);
        i+=(w-i-1);
      }
     else
      {
       if(t.length>0 && this.ns>0 && Mn>0 && (v.length==0 || (this.fast==0 && v1.f<3)))
       {
       // analyse move       
       var k=this.moveTry(B2, t);
     
       if(k>=0 && (v.length==0 || v1.f>0 || t.length>2))
        {   
        var m = B2.I2long(k);
        var h=B2.genml[k];
        var u = B2.m2uci(h);
        var fen = B2.getFEN();
        var curmv = Mn+(B2.sm>0 ? "w" : "b");
        T.push ( { k:k, uci:u, n:Mn, sm:B2.sm, t:[], FEN: fen } );
        B2.mkmove(k);
        B2.movegen();
        if(B2.sm>0) Mn++;
        var p =  (_pgn_Rnd + _pgn_Pt.length).toString();       
        this.shtm+='<div class="_pgn1" id="_pI'+p+'" onclick="_pOc('+p+')">' +
                     s.substr(i,t.length) + '</div>';
        
        _pgn_Pt.push( { idk: this.idk, uci: u, FEN: fen, v:v.length, m:curmv, vf:vf } );
        if(vf>0) vf=0;
        if(v.length>0 && v1.f<2) v1.f=2;
        }
       else
        {
         if(this.fast==0 || v.length==0) this.shtm+=s.substr(i,t.length);
        }       
        
        i+=(t.length-1);       
        
       }
       else if(this.fast==0 || v.length==0) this.shtm+=c;
      }
      
    }
          
    }    
    
    if(this.fast==0 && this.tabs) this.shtm = this.elimSpaces(this.shtm);
    
    if(this.compr) this.shtm = '<scr' + 'ipt type=' + '"text/javascript" >' +
                    "document.write(_c1_dcd('" +
                    _c1_ecd(this.shtm) + "'));"+'</scr' + 'ipt>';
     
    },
       
    setPosAndMove: function( B, To ) {  /* set FEN & move */
        B.setFEN(To.FEN);
        B.mkmove(To.k);
        B.movegen();
    },
    
    someSpaces: function(v) {
    var s='<br>';
    for(var l=0;l<v.length;l++) s+='&nbsp;&nbsp;&nbsp;&nbsp;';
    return s;
    },
    
    elimSpaces: function(s) {
    var k=0;
    for(var i=0;i<s.length;i++)
     {
     if(s.substr(i,4)=='<br>')
      {
      for(var k=i+4;k<s.length;)
       {
        if(s.charAt(k)==' ') k++;
        else if(s.substr(k,6)=='&nbsp;') k+=6;
        else if(s.substr(k,4)=='<br>')
            {
            s=s.substr(0,i+4)+s.substr(k+4);
            break;
            }
        else break;    
       }
      }
     for(var j=0;j<2;j++)
      { 
       var z=(j==0?'<div':'</div>');
       if(s.substr(i,z.length)==z)
        {
        var d=(j==0?19:6);
        var f=0;
        for(k=i+d;k<s.length;)
         {
          if(s.charAt(k)==' ') { k++; f=1; }
          else if(s.substr(k,6)=='&nbsp;') { k+=6; f=1; }
          else if(s.substr(k,4)=='<br>') { k+=4; f=1; }
          else if(s.substr(k,z.length)==z)
             {
              if(f>0) s=s.substr(0,i+d)+s.substr(k);
              break;
             }
          else break;
         }
        }
      }  
     }     
    return s;    
    },
    
    moveTry: function(B, s) {      /* s=notated move, tries to move */
    
    if(s.length<2) return -1;
    else if(("O0NBRQKabcdefgh").indexOf( s.charAt(0) )<0) return -1;

    var r=-1;
    
    var sc=replaceAll2(s,['O','ep'],['0','']);
    if(startsWith(sc,'0-0'))
     {
     for(var j=0;j<B.genml.length;j++)
        {
        var z=B.genml[j];
        var scl = sc.length;
        if((z.k==1 && scl<4) || (z.k==2 && scl>4)) return j;
        }
     return -1;   
     }

    s = sc;
    for(var k=0;r<0 && k<B.genml.length;k++)
        {
        var l=B.genml[k];

        var qf = B.i2sq(l.f);
        var qt = B.i2sq(l.t);
                  
        var f=0;           
        var p1 = l.z.toUpperCase();       
        if(p1=='P')
         {
          if(("NBRQK").indexOf(s.charAt(0))>=0) continue;
         }
        else
         {
          if(p1!=s.charAt(0)) continue;
          f++;
         }
         
        var c=' ';
        var cp=''; 
        var w=0;    // 4-statuses to analyse: a1a4
        var cont = false;
        
        for(;w<4 && f<s.length;f++)
         {
          c=s.charAt(f);
          if(("+#!?").indexOf(c)>=0) {/*skip*/}
          else if(l.p!=0 && w>2 && (c=='=' || ("qrbn").indexOf(c)>=0 )) w=4;
          else if(("Xx-").indexOf(c)>=0) { if(w<2) w=2; }          
          else if(c_isAH(c) && w<3)
          {
           if(w==1) w++;
           if(w==0 && qt==s.substr(f,2)) w=4;
           if(w==0) if(qf.charAt(0)!=c) { cont=true; break; }
           if(w==2) if(qt.charAt(0)!=c) { cont=true; break; }
           w++;          
          }
          else if(c_is18(c))
          {
           if(w==0 || w==2) w++;
           if(w==1) if(qf.charAt(1)!=c) { cont=true; break; }
           if(w==3) if(qt.charAt(1)!=c) { cont=true; break; }
           w++;                  
          }
          else { cont=true; break; }
          if(w>2) cp=s.substr(f);          
         }
        if(cont) continue; 
         
        if(l.p!=0)
         {
         var v=s.indexOf('='); if(v>0) cp=s.substr(v+1);
         cp = cp.toUpperCase();
         if( cp.indexOf( l.p.toUpperCase() ) <0 ) continue; 
         }        
        
        r=k;
        }
        
     return r;
     
    },
 
        
    prepPGN: function(s) { /*prepares PGN string */
    var s2 = '';
    for(var a = 0;a<s.length; a++)
     {
     var c = s.charAt(a);
     var c1 = c.charCodeAt(0);
     var i=CrLf_.indexOf(c);
     if(c1==10 || c1==13) s2+=' ';
     else if(c=='�') s2+= '-';
     else s2+=c;
     }
    
    // skip headers, read info
    var k=0;
    var q=0;
    for(;;k++)
     {
      var e=s2.charAt(k);
      if(q==0 && e==' ') continue;
      q+= (e=='[' ? 1: (e==']' ? -1 : 0));
      if(q==1 && e=='[')
        {
         var s3=trim(s2.substr(k+1,250));
         var s3u = s3.toUpperCase();
         if(startsWith(s3u,'FEN ')) this.startFEN = this.hd_pekas(s3);
         else if(startsWith(s3u,'EVENT ')) this.hd.Event = this.hd_pekas(s3);
         else if(startsWith(s3u,'DATE ')) this.hd.Date = this.hd_pekas(s3);
         else if(startsWith(s3u,'ROUND ')) this.hd.Round = this.hd_pekas(s3);
         else if(startsWith(s3u,'SITE ')) this.hd.Site = this.hd_pekas(s3);
         else if(startsWith(s3u,'WHITE ')) this.hd.White = this.hd_pekas(s3);
         else if(startsWith(s3u,'BLACK ')) this.hd.Black = this.hd_pekas(s3);
         else if(startsWith(s3u,'RESULT ')) this.hd.Result = this.hd_pekas(s3);
        }
      if(q==0 && ("] [").indexOf(e)<0) break;
     }
    return s2.substr(k);
    },
      
    hd_pekas: function (s) {
        var a = s.indexOf('"');
        if(a>=0)
            {
             var s2=s.substr(a+1);
             a = s2.indexOf('"');
             if(a>0) return replaceAll3(s2.substr(0,a),'?','');
            }
        return '';
    },
    
    clone: function () { return new clone(this) }
    
 }

function _c1_ecd(s)     /* encode compressed html */
{
 var w='';
 for(var i=0;i<s.length;i++)
   {
   var j=_c1_em.length-1;
   for(;j>=0;j--)
    {
     var q=_c1_em[j];
     if(s.substr(i,q.length)==q) { w+='@'+j.toString(32); i+=q.length-1; break; }
    }
   if(j<0) w+=s.charAt(i);
   }
 return w;
}


/* Array of NAG codes */
_ngWrds=["has", "Black", "White", "advantage", "the", "played", "control", "well",
 "placement", "very", "poorly", "moderate", "decisive", "slight", "good", "poor",
  "pawn", "time", "ending", "middlegame", "opening", "structure", "king", "move",
   "counterplay", "protected", "queenside", "kingside", "center", "deficit", "material",
    "for", "compensation", "(development)", "space", "Bishops", "pressure", "coordination",
     "piece", "queen", "rook", "bishop", "knight", "strong", "moderately", "weak", "placed",
      "rank", "first", "initiative", "position", "pawns", "color", "pair", "severe", "a5)",
       "(DR:x", "vulnerable", "(<<)", "(>>)", "(#)", "adequate", "than", "more", "(=/&)",
        "sufficient", "insufficient", "(->)", "attack", "lasting", "(^)", "(@)", "(O)",
         "(zz)", "zugzwang", "resign)", "should", "crushing", "chances,", "equal"];

var _ng=['', '!', '?', '!!', '??', 'speculative %17 ("!?")', 'questionable %17 ("?!")',
'forced %17 (all others lose quickly)', 'singular %17 (no reasonable alternatives)', 'worst %17',
'drawish %32', '%4F %4E quiet %32 (=)', '%4F %4E active %32 (ECO ->/<-)', 'unclear %32 (emerging &)',
'%2 %0 a %D %3 (+=)', '%1 %0 a %D %3 (=+)', '%2 %0 a %B %3 (+/-)', '%1 %0 a %B %3 (-/+)',
'%2 %0 a %C %3 (+-)', '%1 %0 a %C %3 (-+)', '%2 %0 a %4D %3 (%1 %4C %4B (+--)',
'%1 %0 a %4D %3 (%2 %4C %4B (--+)', '%2 is in %4A %49', '%1 is in %4A %49', '%2 %0 a %D %22 %3',
'%1 %0 a %D %22 %3', '%2 %0 a %B %22 %3 %48', '%1 %0 a %B %22 %3 %48', '%2 %0 a %C %22 %3',
'%1 %0 a %C %22 %3', '%2 %0 a %D %11 %21 %3', '%1 %0 a %D %11 %21 %3', '%2 %0 a %B %11 %21 %3 %47',
'%1 %0 a %B %11 %21 %3 %47', '%2 %0 a %C %11 %21 %3', '%1 %0 a %C %11 %21 %3', '%2 %0 %4 %31 %46',
'%1 %0 %4 %31 %46', '%2 %0 a %45 %31', '%1 %0 a %45 %31', '%2 %0 %4 %44 %43', '%1 %0 %4 %44 %43',
'%2 %0 in%41 %20 %1F %1E %1D', '%1 %0 in%41 %20 %1F %1E %1D', '%2 %0 %41 %20 %1F %1E %1D %40',
'%1 %0 %41 %20 %1F %1E %1D %40', '%2 %0 %3F %3E %3D %20 %1F %1E %1D', '%1 %0 %3F %3E %3D %20 %1F %1E %1D',
'%2 %0 a %D %1C %6 %3', '%1 %0 a %D %1C %6 %3', '%2 %0 a %B %1C %6 %3 %3C', '%1 %0 a %B %1C %6 %3 %3C',
'%2 %0 a %C %1C %6 %3', '%1 %0 a %C %1C %6 %3', '%2 %0 a %D %1B %6 %3', '%1 %0 a %D %1B %6 %3',
'%2 %0 a %B %1B %6 %3 %3B', '%1 %0 a %B %1B %6 %3 %3B', '%2 %0 a %C %1B %6 %3', '%1 %0 a %C %1B %6 %3',
'%2 %0 a %D %1A %6 %3', '%1 %0 a %D %1A %6 %3', '%2 %0 a %B %1A %6 %3 %3A', '%1 %0 a %B %1A %6 %3 %3A ',
'%2 %0 a %C %1A %6 %3', '%1 %0 a %C %1A %6 %3', '%2 %0 a %39 %30 %2F', '%1 %0 a %39 %30 %2F',
'%2 %0 a %7 %19 %30 %2F', '%1 %0 a %7 %19 %30 %2F', '%2 %0 a %A %19 %16', '%1 %0 a %A %19 %16',
'%2 %0 a %7 %19 %16', '%1 %0 a %7 %19 %16', '%2 %0 a %A %2E %16', '%1 %0 a %A %2E %16',
'%2 %0 a %7 %2E %16', '%1 %0 a %7 %2E %16', '%2 %0 a %9 %2D %10 %15', '%1 %0 a %9 %2D %10 %15',
'%2 %0 a %2C %2D %10 %15 %38 %37', '%1 %0 a %2C %2D %10 %15 %38 %37', '%2 %0 a %2C %2B %10 %15',
'%1 %0 a %2C %2B %10 %15', '%2 %0 a %9 %2B %10 %15', '%1 %0 a %9 %2B %10 %15', '%2 %0 %F %2A %8',
'%1 %0 %F %2A %8', '%2 %0 %E %2A %8', '%1 %0 %E %2A %8', '%2 %0 %F %29 %8', '%1 %0 %F %29 %8',
'%2 %0 %E %29 %8 (diagonal)', '%1 %0 %E %29 %8', '%2 %0 %F %28 %8', '%1 %0 %F %28 %8',
'%2 %0 %E %28 %8 (%2F <=> file ||)', '%1 %0 %E %28 %8', '%2 %0 %F %27 %8', '%1 %0 %F %27 %8',
'%2 %0 %E %27 %8', '%1 %0 %E %27 %8', '%2 %0 %F %26 %25', '%1 %0 %F %26 %25', '%2 %0 %E %26 %25',
'%1 %0 %E %26 %25', '%2 %0 %5 %4 %14 %9 %A', '%1 %0 %5 %4 %14 %9 %A', '%2 %0 %5 %4 %14 %A',
'%1 %0 %5 %4 %14 %A', '%2 %0 %5 %4 %14 %7', '%1 %0 %5 %4 %14 %7', '%2 %0 %5 %4 %14 %9 %7',
'%1 %0 %5 %4 %14 %9 %7', '%2 %0 %5 %4 %13 %9 %A', '%1 %0 %5 %4 %13 %9 %A', '%2 %0 %5 %4 %13 %A',
'%1 %0 %5 %4 %13 %A', '%2 %0 %5 %4 %13 %7', '%1 %0 %5 %4 %13 %7', '%2 %0 %5 %4 %13 %9 %7',
'%1 %0 %5 %4 %13 %9 %7', '%2 %0 %5 %4 %12 %9 %A', '%1 %0 %5 %4 %12 %9 %A', '%2 %0 %5 %4 %12 %A',
'%1 %0 %5 %4 %12 %A', '%2 %0 %5 %4 %12 %7', '%1 %0 %5 %4 %12 %7', '%2 %0 %5 %4 %12 %9 %7',
'%1 %0 %5 %4 %12 %9 %7', '%2 %0 %D %18', '%1 %0 %D %18', '%2 %0 %B %18 (->/<-)', '%1 %0 %B %18',
'%2 %0 %C %18', '%1 %0 %C %18', '%2 %0 %B %11 %6 %24', '%1 %0 %B %11 %6 %24', '%2 %0 %36 %11 %6 %24',
'%1 %0 %36 %11 %6 %24', 'With %4 idea', 'Aimed against', 'Better %17', 'Worse %17', 'Equivalent %17',
'Editors Remark ("RR")', 'Novelty ("N")', 'Weak point', 'Endgame', 'Line', 'Diagonal',
'%2 %0 a %35 of %23', '%1 %0 a %35 of %23', '%23 of opposite %34', '%23 of same %34',
'','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','',
'','','','Etc.', 'Doubled %33', 'Isolated %10', 'Connected %33', 'Hanging %33', 'Backwards %10',
'','','','','','Diagram ("D", "#")'];

function prepare_ngArr()
{
 for(var i=0;i<202;i++)
    {
    var s=_ng[i];
    for(;;) {
          var a=s.indexOf('%');  if(a<0) break;
          var z=s.substr(a+1,3); var b=z.indexOf(' '); var n=(b<0?z:z.substr(0,b));
          s=s.substr(0,a)+_ngWrds[parseInt(n,16)]+s.substr(a+n.length+1);
         }
    _ng[i] = s;
    }
}
prepare_ngArr();

_pgn_Rnd = 0;	// bugz on multi-post blogs...

