
//--------------------------------------
// SCRIPTS for int 64-bit cases
//--------------------------------------

// constants
g_FF=0xFFFFFFFF;
g_F=0xFFFF;
g_10=0x100000000;
g_1=0x10000;
g_F0=i64_ax(g_FF,0);
g_0F=i64_ax(0,g_FF);
g_FF0=i64_ax(0,0xFF0000);
g_Fo=i64_ax(0xFF00,0);
g_32=32000;
g_37=32768;


cs64 = { MIN_VALUE:i64(), MAX_VALUE:i64_ax(g_FF,g_FF) };

	//powers of 10 in 64 bits	
pw10=[ i64_ax(0,0x1),
i64_ax(0,0xA),
i64_ax(0,0x64),
i64_ax(0,0x3E8),
i64_ax(0,0x2710),
i64_ax(0,0x186A0),
i64_ax(0,0xF4240),
i64_ax(0,0x989680),
i64_ax(0,0x5F5E100),
i64_ax(0,0x3B9ACA00),
i64_ax(0x2,0x540BE400),
i64_ax(0x17,0x4876E800),
i64_ax(0xE8,0xD4A51000),
i64_ax(0x918,0x4E72A000),
i64_ax(0x5AF3,0x107A4000),
i64_ax(0x38D7E,0xA4C68000),
i64_ax(0x2386F2,0x6FC10000),
i64_ax(0x1634578,0x5D8A0000),
i64_ax(0xDE0B6B3,0xA7640000),
i64_ax(0x8AC72304,0x89E80000) ];

/* Define */

function i64() { return { h:0, l:0 } }	// constructor
function i64_al(v) { return { h:0, l:v } }	// +assign 32-bit value
function i64_ax(h,l) { return { h:h, l:l } }	// +assign 64-bit v.as 2 regs
function i64s(s)				// +assign by string
 {
  var a=s.indexOf("0x"); if(a<0) return "input error";
  return { h: parseInt("0x"+s.substr(a+2,8)), l: parseInt("0x"+s.substr(a+10,8)) }
 }

// for convenience
function i64v(v) { return { h:0, l:v } }	// +assign 32-bit value
function i64c(a) { return { h:a.h, l:a.l } }	// clone

function i64_fromString(s)
 {
  var a=s.indexOf("0x"); if(a<0) return "input error";
  return { h: parseInt("0x"+s.substr(a+2,8)), l: parseInt("0x"+s.substr(a+10,8)) }	// +assign by string
 }

// originally was ((x>>>1) + (x&1) )% g_10
// modulus is slow and >>>0 is faster
function i64u(x) { return ( ((x >>> 0) & g_FF) >>> 0 ); }	// keeps [0..g_FFF] 

/* Type conversions */

function i64_toint(a)
 {	// Javascript supports value=2^53
  return (a.h>0 ? (a.l + (a.h * g_10)) : a.l);
 }

function i64_toString(a)
 {
  var s1=(a.l).toString(16);
  var s2=(a.h).toString(16);
  var s3="0000000000000000";
  s3=s3.substr(0,16-s1.length)+s1;
  s3=s3.substr(0,8-s2.length)+s2+s3.substr(8);
  return "0x"+s3.toUpperCase();
  }

/* Bitwise operators (the main functionality) */

function i64_and(a,b) { return { h:( a.h & b.h )>>>0, l:( a.l & b.l )>>>0 } }
function i64_or(a,b) { return { h:( a.h | b.h )>>>0, l:( a.l | b.l )>>>0 } }
function i64_xor(a,b) { return { h:( a.h ^ b.h )>>>0, l:( a.l ^ b.l )>>>0 } }
function i64_not(a) { return { h:(~a.h)>>>0, l:(~a.l)>>>0 } }

/* Simple Math-functions */

// just to add, not rounded for overflows
function i64_add(a,b)
 { var o = { h:a.h+b.h, l:a.l+b.l }; var r = o.l-g_FF;
   if( r>0 ) { o.h++; o.l =--r; } return o; }

// verify a>=b before usage
function i64_sub(a,b)
 { var o = { h:a.h-b.h, l:a.l-b.l };
   if( o.l<0 ) { o.h--; o.l+=g_10; } return o; }

// x n-times, better not to use for large n
function i64_txmul(a,n)
{ var o = { h:a.h, l:a.l }; for(var i=1; i<n; i++ ) o = i64_add(o,a); return o; }


// multiplication arithmetic
// (it gives small mistake without checksum for too large numbers)
function i64_mul(a,b)
 {

 /*
	// slow but working checksum to get right result
 if(i64_bithighestat(a)+i64_bithighestat(b)>63)
  {
  return i64_bitmul(a,b);
  }
 */

  var o = { h:(a.h * b.l) + (b.h * a.l), l:(a.l * b.l) };

  var o3 = (o.h & g_FF)>>>0;	// to see how
  var o4 = ((o.h+1) & g_FF)>>>0;	// it changes
  if( o3==o4 ) return i64_bitmul(a,b);	//fast checksum for 53-bit javascript overflow

  var oC=0;
  if( o.l>g_FF )
   {
    oC=Math.floor(o.l/g_10);
    o.h += oC;
    o.l =(o.l & g_FF)>>>0;
   }
  o.h =(o.h & g_FF)>>>0;

  if( (o3+oC)!=o.h ) return i64_bitmul(a,b);	// checksum

  return o;
 }

// multiplication by shifting bits, good for few-bits manipulation
// (slow)
function i64_bitmul(a,b)
 {
  var o = { h:0, l:0 };
  var m = { h:b.h, l:b.l };
  var t = { h:a.h, l:a.l };
  for(var j=63;j>0;j--)
   {
    if( !(m.l | m.h) ) break; 
    if(m.l & 1) o=i64_add(o,t);
    m = i64_rshift(m,1);
    t = i64_lshift(t,1);
   }
  o.l =(o.l & g_FF)>>>0;
  o.h =(o.h & g_FF)>>>0;
  return o;
 }
// bitwise division calculates and returns [rs,rm],
// where a=(b*rs)+rm, better not to use for simple math
function i64_bitdiv(a,b)
 {
  var rs = { h:0,l:0 };
  var rm = { h:a.h,l:a.l };
  if( (b.l | b.h) && i64_gt(a,b) )
  {
  var d = { h:b.h, l:b.l };
  var p = { h:d.h, l:d.l };
  var y = 0;
  var w = i64_bithighestat(d);
  while((w<64) && (d.l | d.h) && i64_le(d,rm) )
   {
    p = { h:d.h, l:d.l };
    var hbit=i64_bithighestat(d);
    d = i64_lshift(d,1);
    y++; w++;
   }

  while((y>0) || (p.l | p.h))
   {
    rs=i64_lshift(rs,1); y--;
    if( i64_le(p,rm) ) { rm=i64_sub(rm,p); rs.l=(rs.l|1)>>>0; }
    if( (y<=0) && i64_gt(b,rm) ) break;
    p = i64_rshift(p,1);
   }
  }
  return [rs,rm];
 }

// x n-times multiplied by self, slow
function i64_txpow(a,n)
{ var o = { h:a.h, l:a.l }; for(var i=1; i<n; i++ ) o = i64_mul(o,a); return o; }


/* Bit-shifting */

function i64_lshift(a,n)
 {
  switch(n)
  {
  case 0: return { h:a.h, l:a.l };
  case 32: return { h:a.l, l:0 };
  default:
   if(n<32) return { h:((a.h << n)>>>0)+(a.l >>> (32-n)), l:(a.l << n)>>>0 };
   else return { h:(a.l << (n-32))>>>0, l:0 };
  }
 }

function i64_rshift(a,n)
 {
  switch(n)
  {
  case 0: return { h:a.h, l:a.l };
  case 32: return { h:0, l:a.h };
  default:
   if(n<32) return {  h:(a.h >>> n), l:(a.l >>> n)+(a.h << (32-n))>>>0 };
   else return { h:0, l:(a.h >>> (n-32)) };
  }
 }

// gets bit at position n
function i64_bitat(a,n) { return ( (n<32) ? (a.l & (1<<n)) : (a.h & (1<<(n-32))) ); } 

// sets bit at position n (on)
function i64_bitset(a,n)
 { if(n<32) a.l = (a.l | (1<<n))>>>0; else a.h = (a.h | (1<<(n-32)))>>>0; } 

// clears bit at position n (off)
function i64_bitclear(a,n)
 { if(n<32) a.l = (a.l & (~(1<<n)))>>>0; else a.h = (a.h & (~(1<<(n-32))))>>>0; } 

// toggles bit at position n (on/off)
function i64_bittoggle(a,n)
 { if(n<32) a.l = (a.l ^ (1<<n))>>>0; else a.h = (a.h ^ (1<<(n-32)))>>>0; } 

/* bitwise calcs just fast as possible */

bitcnt_ = [];
bitlsb_ = [];
bithsb_ = [];
function i64_prep_bclwbhb()
{
var i,c,o,m,k;
for(i=0;i<g_1;i++)
 {
 c=0; o=i;
 for(var k=0;k<32;k++) { c+=((o&1)>>>0); o>>>=1; };
 bitcnt_[i] = c;
 }

for(i=0;i<g_1;i++)
 {
 c=-1; o=i;
 for(var k=0;c<0 && k<32;k++) { if(o&1) c=k; o>>>=1; };
 bitlsb_[i] = c;
 }

for(i=0;i<g_1;i++)
{
 c=-1; o=i; m=(1<<31);
 for(k=31;c<0 && k>=0;k--) { if(o&m) c=k; m>>>=1; };
 bithsb_[i] = c;
 }
}
i64_prep_bclwbhb();

// calculates count of bits =[0..63]
function i64_bitcount(a)
{

  return ( bitcnt_[ (a.l & g_F)>>>0 ] + bitcnt_[ ((a.l>>>16) & g_F)>>>0 ] +
     bitcnt_[ (a.h & g_F)>>>0 ] + bitcnt_[ ((a.h>>>16) & g_F)>>>0 ] );
}


// finds lowest bit (position [0..63] or -1)
function i64_bitlowestat(a)
{

  var q = (a.l & g_F)>>>0;
  if(q) return bitlsb_[q];
  q = ((a.l>>>16) & g_F)>>>0;
  if(q) return (bitlsb_[q] | 16);
  q = (a.h & g_F)>>>0;
  if(q) return (bitlsb_[q] | 32);
  q = ((a.h>>>16) & g_F)>>>0;
  if(q) return (bitlsb_[q] | 48);
  return -1;
}

// finds highest bit (position [0..63] or -1)
function i64_bithighestat(a)
{

  var q = ((a.h>>>16) & g_F)>>>0;
  if(q) return (bithsb_[q] | 48);
  q = (a.h & g_F)>>>0;
  if(q) return (bithsb_[q] | 32);
  q = ((a.l>>>16) & g_F)>>>0;
  if(q) return (bithsb_[q] | 16);
  var q = (a.l & g_F)>>>0;
  if(q) return bithsb_[q];
  return -1;
}

// returns object for a bit[0..63]
function i64bitObj(n)
{
 if(typeof(bitObjat_)=="undefined")
   {
   bitObjat_ = [];
   var o=i64_al(1);
   for(var i=0;i<64;i++) { bitObjat_[i]={h:o.h,l:o.l}; o=i64_lshift(o,1); }
   }
 return bitObjat_[n];
}


/* Comparisons */

function i64_eq(a,b) { return ((a.h == b.h) && (a.l == b.l)); }
function i64_ne(a,b) { return ((a.h != b.h) || (a.l != b.l)); }
function i64_gt(a,b) { return ((a.h > b.h) || ((a.h == b.h) && (a.l >  b.l))); }
function i64_ge(a,b) { return ((a.h > b.h) || ((a.h == b.h) && (a.l >= b.l))); }
function i64_lt(a,b) { return ((a.h < b.h) || ((a.h == b.h) && (a.l <  b.l))); }
function i64_le(a,b) { return ((a.h < b.h) || ((a.h == b.h) && (a.l <= b.l))); }
function i64_is0(a) { return (!(a.l | a.h)); }	// is zero?
function i64_nz(a) { return (a.l|a.h); }	// is not zero?

/* Decimal conversions */

// converts to decimal string
function i64_todecString(a)
{
 var s=""; var rz=[ {h:0,l:0},{h:a.h,l:a.l} ];
 for(var n=19;n>=0;n--)
  {
   rz=i64_bitdiv(rz[1],pw10[n]); var d=rz[0].l;
   if((!n && (!s.length)) || (d>0) || (s.length>0)) s+=d.toString();
  }
 return s;
}

// converts decimal string to 64-bit object
function i64_fromdecString(s)
{
 var o={h:0,l:0};
 var c=s.length;
 for(var n=0;(n<=19) && ((c--)>0);n++)
  {
   var d=s.charCodeAt(c)-48;
   if((d>0) && (d<=9))
    {
     var o2=i64_bitmul(pw10[n],i64_al(d)); o=i64_add(o,o2);
    }
  }
 return o;
}

/* Other basic functions */


function i64_max(a,b) { return (i64_ge(a,b) ? a : b ); }	// max
function i64_min(a,b) { return (i64_le(a,b) ? a : b ); }	// min
function i64_neg(a)   { return i64_add(i64_not(a),i64_al(1)); }	// negative value as unsigned
function i64_mod(a,b) { var rz=i64_bitdiv(a,b); return rz[1]; }	// modulus
function i64_isodd(a,b) { return ((a.l&1)==0); }	// is odd
function i64_next(a) { return i64_add(a,{h:0,l:1}); }	// next number
function i64_pre(a) { return i64_sub(a,{h:0,l:1}); }	// previous number
function i64_is0(a) { return ((a.l | a.h)==0); }	// is zero?
function i64_not0(a) { return ((a.l | a.h)!=0); }	// not zero?

//--------------------------------------
// END OF SCRIPTS for int 64-bit cases
//--------------------------------------
