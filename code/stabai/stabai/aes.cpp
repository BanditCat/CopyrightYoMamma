#include "stabai.h"





static u32 vallocCount = 0;
static u32 vallocBuffSize = 0;
static void** vallocs = NULL;
static u32* vallocCounts = NULL;
static u32* vallocBuffSizes = NULL;

//static LARGE_INTEGER vhpFrequency;
//static LARGE_INTEGER vclock;
//static LARGE_INTEGER vstartClock;

void* vsmalloc( u32 sz ){
  void* nb = new u8[ sz ];
  return nb;
}

void pvnquit( void );
void vmemcpy( void* dst, const void* src, u32 sz );
void vsfree( void* data );
void vappendString( u32 i, const u8* msg );
void vmemset( void* d, u8 c, u32 sz );

typedef struct{
  u32 data;
  u32 bits;
} vbitField;
void vmemset( void* d, u8 c, u32 sz ){
  __asm{
    cld
      mov    edi, d
      mov    ecx, [sz]
    mov    al, [c]
    repz stos al
  }
}
// if bf is NULL creates a new one.
vbitField* vresetBitField( vbitField* bf );
void vaddBits( vbitField* bf, const u8* data, u32 bits );
int vgetBit( const vbitField* bf, u32 bit );
u32 vgetBits( const vbitField* bf, u32 bit, u32 num );
void vsetBit( vbitField* bf, u32 bit, int val );
#define vbfSize( x ) ( ( (x)->bits ) ? ( ( (x)->bits - 1 ) / 8 + 1 ) : 0 )



// Encrypts a single 128-bit data block using the AES-256 key key.
void vencrypt( const u8 key[ 32 ], u8 data[ 16 ] );
// Generates a key from a 64-bit nonce.
void vgenKey( u32 lononce, u32 hinonce, u8 key[ 32 ] );
// Generates a one-time pad from a 64-bit nonce using AES-256 in counter mode.
// The returned pointer must be vsfree'd.
u8* vgenPad( u32 lononce, u32 hinonce, u32 sz ); 


typedef struct{
  u32 index;
  u32 buffer[ 624 ];
} vrandState;
void vseeds( u32 seed, vrandState* vs );
void vseed( u32 seed );
// Returns a value between min and max.
f32 vrands( f32 min, f32 max, vrandState* vs );
f32 vrand( f32 min, f32 max );
u32 vrand32s( u32 min, u32 max, vrandState* vs );
u32 vrand32( u32 min, u32 max );
u32 vrand32allBitss( vrandState* vs );
u32 vrand32allBits( void );


u32 vmalloc( u32 asz ){
  u32 sz = asz ? asz : 1;
  if( vallocs == NULL ){
    vallocs = (void**)vsmalloc( sizeof( void* ) );
    vallocCounts = (u32*)vsmalloc( sizeof( u32 ) );
    vallocBuffSizes = (u32*)vsmalloc( sizeof( u32 ) );
    vallocCount = 0;
    vallocBuffSize = 1;
  }
  while( vallocCount >= vallocBuffSize ){
    void* nb = vsmalloc( sizeof( void* ) * vallocBuffSize * 2 );
    vallocBuffSize *= 2;
    vmemcpy( nb, vallocs, sizeof( void* ) * vallocCount );
    vsfree( vallocs ); vallocs = (void**)nb;
    nb = vsmalloc( sizeof( u32 ) * vallocBuffSize );
    vmemcpy( nb, vallocCounts, sizeof( u32 ) * vallocCount );
    vsfree( vallocCounts ); vallocCounts = (u32*)nb;
    nb = vsmalloc( sizeof( u32 ) * vallocBuffSize );
    vmemcpy( nb, vallocBuffSizes, sizeof( u32 ) * vallocCount );
    vsfree( vallocBuffSizes ); vallocBuffSizes = (u32*)nb;
  }
  vallocs[ vallocCount ] = vsmalloc( sz );
  vallocCounts[ vallocCount ] = 0;
  vallocBuffSizes[ vallocCount ] = sz;
  return ++vallocCount;
}
void vsfree( void* data ){
  delete data;
}
void vmemcpy( void* dst, const void* src, u32 sz ){
  while( sz-- )
    ( (u8*)dst )[ sz ] = ( (u8*)src )[ sz ];
}
void* vmem( u32 i ){
  assess( !( !i || ( i > vallocCount ) ), "Invalid pointer reference in vmem!" );
  return vallocs[ i - 1 ];
}
u32 vsize( u32 i ){
  assess( !( !i || ( i > vallocCount ) ), "Invalid pointer reference in vsize!" );
  return vallocCounts[ i - 1 ];
}
void verase( u32 i ){
  vsfree( vallocs[ i - 1 ] );
  vallocCounts[ i - 1 ] = 0;
  vallocBuffSizes[ i - 1 ] = 1;
  vallocs[ i - 1 ] = vsmalloc( 1 );
}
void vappend( u32 i, const void* nm, u32 sz ){
  u32 m = i - 1;
  assess( !( !i || ( i > vallocCount ) ), "Invalid pointer reference in vappend!" );
  if( vallocCounts[ m ] + sz >= vallocBuffSizes[ m ] ){
    void* nb;
    u32 nsz = ( vallocBuffSizes[ m ] ? vallocBuffSizes[ m ] : 1 )* 2;
    while( nsz < vallocCounts[ m ] + sz )
      nsz *= 2;
    nb = vsmalloc( nsz );
    vallocBuffSizes[ m ] = nsz;
    vmemcpy( nb, vallocs[ m ], vallocCounts[ m ] );
    vsfree( vallocs[ m ] );
    vallocs[ m ] = nb;
  }
  vmemcpy( (u8*)vallocs[ m ] + vallocCounts[ m ], nm, sz );
  vallocCounts[ m ] += sz;
}
void vgrow( u32 i, u32 sz ){
  u32 m = i - 1;
  assess( !( !i || ( i > vallocCount ) ), "Invalid pointer reference in vgrow!" );
  if( vallocCounts[ m ] + sz >= vallocBuffSizes[ m ] ){
    void* nb;
    u32 nsz = vallocBuffSizes[ m ] * 2;
    while( nsz < vallocCounts[ m ] + sz )
      nsz *= 2;
    nb = vsmalloc( nsz );
    vallocBuffSizes[ m ] = nsz;
    vmemcpy( nb, vallocs[ m ], vallocCounts[ m ] );
    vsfree( vallocs[ m ] );
    vallocs[ m ] = nb;
  }
  {
    u8* mp = (u8*)vallocs[ m ] + vallocCounts[ m ];
    vmemset( mp, 0, sz );
  }
  vallocCounts[ m ] += sz;
}
void vpop( u32 i, u32 sz ){
  u32 m = i - 1;
  assess( !( !i || ( i > vallocCount ) ), "Invalid pointer reference in vpop!" );
  if( sz < vallocCounts[ m ] )
    vallocCounts[ m ] -= sz;
  else
    vallocCounts[ m ] = 0;
}
void vappendInt( u32 i, int msg, u32 mw ){
  static u8 s[ 100 ];
  static u8 buf[ 100 ];
  u32 j = 0, k;
  int neg = 0;
  if( mw >= 98 )
    throw "Min width too large in vintToString.";
  if( !msg ){
    s[ j++ ] = '0';
  }else{
    if( msg < 0 ){
      neg = 1;
      msg *= -1;
    }
    while( msg ){
      buf[ j++ ] = '0' + (u8)( msg % 10 );
      msg /= 10;
    }
    if( neg )
      buf[ j++ ] = '-';
    for( k = 0; k < j; ++k )
      s[ k ] = buf[ j - k - 1 ];
  }
  if( mw > j ){
    vmemset( s + j, ' ', mw - j );
    j = mw;
  }
  s[ j ] = '\0';
  vappendString( i, s );
}
const u8* vintToString( int msg, u32 mw ){
  static u8 s[ 100 ];
  static u8 buf[ 100 ];
  u32 j = 0, k;
  int neg = 0;
  if( mw >= 98 )
    throw "Min width too large in vintToString.";
  if( !msg ){
    s[ j++ ] = '0';
  }else{
    if( msg < 0 ){
      neg = 1;
      msg *= -1;
    }
    while( msg ){
      buf[ j++ ] = '0' + (u8)( msg % 10 );
      msg /= 10;
    }
    if( neg )
      buf[ j++ ] = '-';
    for( k = 0; k < j; ++k )
      s[ k ] = buf[ j - k - 1 ];
  }
  if( mw > j ){
    vmemset( s + j, ' ', mw - j );
    j = mw;
  }
  s[ j ] = '\0';
  return s;
}
const u8* vintToName( int i ){
  static u8 ans[ 100 ];
  static u8 buf[ 100 ];
  u32 p = 0, m = i;
  if( !i )
    return (const u8*)"A";
  if( i == -1 )
    return (const u8*)"-A";
  if( i < 0 ){
    m = -1 - i;
  }
  {
    u32 ap = 0;
    while( m ){
      if( p )
        --m;
      buf[ p++ ] = 'A' + m % 26;
      m /= 26;
    }
    if( i < 0 )
      ans[ ap++ ] = '-';
    while( p )
      ans[ ap++ ] = buf[ --p ];
    ans[ ap ] = '\0';
    return ans;
  }
}
u32 vreadInt( const u8* p ){
  u32 ans = 0;
  while( *p >= '0' && *p <= '9' ){
    ans *= 10;
    ans += *p - '0';
    ++p;
  }
  return ans;
}

void vappendString( u32 i, const u8* msg ){
  u32 m = i - 1;
  u32 sz = 0;
  while( msg[ sz ] )
    ++sz;
  assess( !( !i || ( i > vallocCount ) ), "Invalid pointer reference in vappend!" );
  if( sz ){
    if( vallocCounts[ m ] && !( (u8**)vallocs )[ m ][ vallocCounts[ m ] - 1 ] ){
      ( (u8**)vallocs )[ m ][ vallocCounts[ m ] - 1 ] = *msg;
      vappend( i, msg + 1, sz - 1 );
    }else
      vappend( i, msg, sz );
  }
  if( ( vallocCounts[ m ] && ( (u8**)vallocs )[ m ][ vallocCounts[ m ] - 1 ] ) || !vallocCounts[ m ] )
    vappend( i, "\0", 1 );
}
void vappendHexByte( u32 i, u8 b ){
  static char buf[ 2 ] = "0";
  u8 hn = b / 16;
  u8 ln = b % 16;
  if( hn < 10 )
    buf[ 0 ] = '0' + hn;
  else
    buf[ 0 ] = 'A' + hn - 10;
  vappendString( i, (const u8*)buf );
  if( ln < 10 )
    buf[ 0 ] = '0' + ln;
  else
    buf[ 0 ] = 'A' + ln - 10;
  vappendString( i, (const u8*)buf );
}
void vappendStringNoNul( u32 i, const u8* msg ){
  vappendString( i, msg );
  --vallocCounts[ i - 1 ];
}
void vpopNul( u32 i ){
  if( vallocCounts[ i - 1 ] && !( ( (u8**)vallocs )[ i - 1 ][ vallocCounts[ i - 1 ] - 1 ] ) )
    vpop( i, 1 );  
}



// This is a Mersenne Twister implimenatation based on public domain code by Michael Brundage.
#define MT_LEN 624
static vrandState pgvrand = { 0, 0 };

void vseeds( u32 s, vrandState* vs ){
  u32 i;
  vs->index = 0;
  vs->buffer[ 0 ] = (u32)s;
  for( i = 1; i < 624; ++i )
    vs->buffer[ i ] = ( 1812433253 * ( vs->buffer[ i - 1 ] ^ ( ( vs->buffer[ i - 1 ] ) >> 30 ) ) + i );
}
void vseed( u32 s ){ vseeds( s, &pgvrand ); }

#define MT_IA           397
#define MT_IB           (MT_LEN - MT_IA)
#define UPPER_MASK      0x80000000
#define LOWER_MASK      0x7FFFFFFF
#define MATRIX_A        0x9908B0DF
#define TWIST(b,i,j)    ((b)[i] & UPPER_MASK) | ((b)[j] & LOWER_MASK)
#define MAGIC(s)        (((s)&1)*MATRIX_A)

u32 pvrand( vrandState* vs ){
  u32 * b = vs->buffer;
  int idx = vs->index;
  u32 s;
  int i;

  if (idx == MT_LEN*sizeof(u32))
  {
    idx = 0;
    i = 0;
    for (; i < MT_IB; i++) {
      s = TWIST(b, i, i+1);
      b[i] = b[i + MT_IA] ^ (s >> 1) ^ MAGIC(s);
    }
    for (; i < MT_LEN-1; i++) {
      s = TWIST(b, i, i+1);
      b[i] = b[i - MT_IB] ^ (s >> 1) ^ MAGIC(s);
    }

    s = TWIST(b, MT_LEN-1, 0);
    b[MT_LEN-1] = b[MT_IA-1] ^ (s >> 1) ^ MAGIC(s);
  }
  vs->index = idx + sizeof(u32);
  { 
    u32 ans = *(u32 *)( (u8*)b + idx);
    ans ^= (ans >> 11);
    ans ^= (ans << 7) & 0x9D2C5680;
    ans ^= (ans << 15) & 0xEFC60000;
    ans ^= (ans >> 18);
    return ans;
  }
}
f32 vrands( f32 min, f32 max, vrandState* vs ){
  return (f32)( pvrand( vs ) ) / (f32)UINT_MAX * ( max - min ) + min;
}
f32 vrand( f32 min, f32 max ){ return vrands( min, max, &pgvrand ); }
u32 vrand32s( u32 min, u32 max, vrandState* vs ){
  return ( pvrand( vs ) % ( 1 + max - min ) ) + min;
}
u32 vrand32( u32 min, u32 max ){ return vrand32s( min, max, &pgvrand ); }
u32 vrand32allBitss( vrandState* vs ){
  return pvrand( vs );
}
u32 vrand32allBits( void ){ return vrand32allBitss( &pgvrand ); }

vbitField* vresetBitField( vbitField* bf ){ 
  if( bf != NULL ){
    bf->bits = 0;
    verase( bf->data );
    return bf;
  } else{
    vbitField* ans = (vbitField*)vmem( vmalloc( sizeof( vbitField ) ) );
    ans->data = vmalloc( 0 );
    ans->bits = 0;
    return ans;
  }
}
void vaddBits( vbitField* bf, const u8* data, u32 bits ){
  static const u8 z = 0;
  const u32 nsz = bits + bf->bits;
  const u8* dp = data;
  u8 dm = 1;
  while( vsize( bf->data ) < ( nsz / 8 + 1 ) )
    vappend( bf->data, &z, 1 );
  while( bf->bits < nsz ){
    ++bf->bits;
    vsetBit( bf, bf->bits - 1, *dp & dm );
    if( dm == 128 ){
      ++dp;
      dm = 1;
    }else
      dm *= 2;
  }
}
int vgetBit( const vbitField* bf, u32 bit ){
  assess( bit < bf->bits, "invalid bit index in vgetBit!" );
  return ( ((u8*)vmem( bf->data ) )[ bit / 8 ] ) & ( 1 << ( bit % 8 ) ) ? 1 : 0;
}
u32 vgetBits( const vbitField* bf, u32 bit, u32 num ){
  u8 bt = 1 << ( bit % 8 );
  u8* dp;
  u32 ans = 0;
  u32 abt = 1;
  u32 i;
  assess( ( bit + num - 1 ) < bf->bits, "invalid bit indices in vgetBits!" );
  assess( num <= 32, "Attempt to get more than 32 bits with vgetBits!" );
  dp = ( (u8*)vmem( bf->data ) ) + ( bit / 8 );
  for( i = 0; i < num; ++i ){
    if( *dp & bt )
      ans += abt;
    if( bt == 128 ){
      ++dp;
      bt = 1;
    } else
      bt *= 2;
    abt *= 2;
  }
  return ans;
}
void vsetBit( vbitField* bf, u32 bit, int val ){
  u8 bt = ( 1 << ( bit % 8 ) );
  u8 msk = ~bt;
  u8* cb;
  assess( bit < bf->bits, "invalid bit index in vgetBit!" );
  cb = ((u8*)vmem( bf->data ) ) + ( bit / 8 );
  *cb = ( *cb & msk ) + ( val ? bt : 0 );
}
const u8* vbitFieldToString( const vbitField* bf, u32 lw ){
  static u32 ans = 0;
  u32 i;
  if( !ans )
    ans = vmalloc( 0 );
  else
    verase( ans );
  for( i = 0; i < bf->bits; ++i ){
    vappend( ans, vgetBit( bf, i ) ? "1" : "0", 1 );
    if( !( ( i + 1 ) % lw ) )
      vappend( ans, "\n", 1 );
  }
  vappend( ans, "", 1 );
  return (const u8*)vmem( ans );
}
//f32 vcurTime( void ){
//  LARGE_INTEGER vclock;
//  if( !QueryPerformanceCounter( &vclock ) )
//    throw "Unable to query high performance timer";
//  return (f32)( vclock.QuadPart - vstartClock.QuadPart ) / (f32)( vhpFrequency.QuadPart );
//}
//void vtimeNonce( u32* lononce, u32* hinonce ){
//  static const u8 mkey[ 32 ] = { 224, 64, 187, 5, 235, 1, 254, 61, 157, 63, 20, 128, 99, 62, 27, 85, 121, 218, 245, 39, 118, 166, 19, 58, 57, 68, 138, 120, 240, 154, 64, 57 };
//  static u32 lastlo = 0;
//  static u32 lasthi = 0;
//  static u32 fudge = 0;
//  SYSTEMTIME st;
//  FILETIME ft;
//  GetSystemTime( &st );
//  SystemTimeToFileTime( &st, &ft );
//  if( lastlo == ft.dwLowDateTime && lasthi == ft.dwHighDateTime )
//    ++fudge;
//
//  lastlo = ft.dwLowDateTime;
//  lasthi = ft.dwHighDateTime;
//
//  {
//    u32 i;
//    static u8 pt[ 16 ];
//    u32 lh = lasthi + fudge;
//    u32 ll = lastlo + fudge;
//    for( i = 0; i < 2; ++i ){
//      pt[ i * 8 + 0 ] = ( lh >> 0 ) % 256;    
//      pt[ i * 8 + 1 ] = ( ll >> 0 ) % 256;
//      pt[ i * 8 + 2 ] = ( lh >> 8 ) % 256;
//      pt[ i * 8 + 3 ] = ( ll >> 8 ) % 256;
//      pt[ i * 8 + 4 ] = ( lh >> 16 ) % 256;    
//      pt[ i * 8 + 5 ] = ( ll >> 16 ) % 256;
//      pt[ i * 8 + 6 ] = ( lh >> 24 ) % 256;
//      pt[ i * 8 + 7 ] = ( ll >> 24 ) % 256;  
//    }
//    vencrypt( mkey, pt );
//    *lononce = *( ( (u32*)pt ) + 0 );
//    *hinonce = *( ( (u32*)pt ) + 1 );
//  }
//}









// This is based off public domain code that had this header:
/* 
****************************************************************** 
**       Advanced Encryption Standard implementation in C.      ** 
**       By Niyaz PK                                            ** 
**       E-mail: niyazpk@gmail.com                              ** 
**       Downloaded from Website: www.hoozi.com                 ** 
****************************************************************** 
This is the source code for encryption using the latest AES algorithm. 
****************************************************************** 
*/  

// The number of columns comprising a state in AES. This is a constant in AES. Value=4
#define Nb 4

typedef struct {
  u8 key[ 32 ];
  u8 roundKey[ 240 ];
  u8 in[ 16 ];
  u8 out[ 16 ];
  u8 state[ 4 ][ 4 ];
} vaes;

const static int numRounds = 14;

const static int numKeys = 8;

static int sbox[ 256 ] = {
  0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
  0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
  0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
  0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
  0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
  0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
  0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
  0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
  0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
  0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
  0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
  0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
  0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
  0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
  0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
  0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16 
};

static int rcon[ 255 ] = {
  0x8d, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36, 0x6c, 0xd8, 0xab, 0x4d, 0x9a, 
  0x2f, 0x5e, 0xbc, 0x63, 0xc6, 0x97, 0x35, 0x6a, 0xd4, 0xb3, 0x7d, 0xfa, 0xef, 0xc5, 0x91, 0x39, 
  0x72, 0xe4, 0xd3, 0xbd, 0x61, 0xc2, 0x9f, 0x25, 0x4a, 0x94, 0x33, 0x66, 0xcc, 0x83, 0x1d, 0x3a, 
  0x74, 0xe8, 0xcb, 0x8d, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36, 0x6c, 0xd8, 
  0xab, 0x4d, 0x9a, 0x2f, 0x5e, 0xbc, 0x63, 0xc6, 0x97, 0x35, 0x6a, 0xd4, 0xb3, 0x7d, 0xfa, 0xef, 
  0xc5, 0x91, 0x39, 0x72, 0xe4, 0xd3, 0xbd, 0x61, 0xc2, 0x9f, 0x25, 0x4a, 0x94, 0x33, 0x66, 0xcc, 
  0x83, 0x1d, 0x3a, 0x74, 0xe8, 0xcb, 0x8d, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 
  0x36, 0x6c, 0xd8, 0xab, 0x4d, 0x9a, 0x2f, 0x5e, 0xbc, 0x63, 0xc6, 0x97, 0x35, 0x6a, 0xd4, 0xb3, 
  0x7d, 0xfa, 0xef, 0xc5, 0x91, 0x39, 0x72, 0xe4, 0xd3, 0xbd, 0x61, 0xc2, 0x9f, 0x25, 0x4a, 0x94, 
  0x33, 0x66, 0xcc, 0x83, 0x1d, 0x3a, 0x74, 0xe8, 0xcb, 0x8d, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 
  0x40, 0x80, 0x1b, 0x36, 0x6c, 0xd8, 0xab, 0x4d, 0x9a, 0x2f, 0x5e, 0xbc, 0x63, 0xc6, 0x97, 0x35, 
  0x6a, 0xd4, 0xb3, 0x7d, 0xfa, 0xef, 0xc5, 0x91, 0x39, 0x72, 0xe4, 0xd3, 0xbd, 0x61, 0xc2, 0x9f, 
  0x25, 0x4a, 0x94, 0x33, 0x66, 0xcc, 0x83, 0x1d, 0x3a, 0x74, 0xe8, 0xcb, 0x8d, 0x01, 0x02, 0x04, 
  0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36, 0x6c, 0xd8, 0xab, 0x4d, 0x9a, 0x2f, 0x5e, 0xbc, 0x63, 
  0xc6, 0x97, 0x35, 0x6a, 0xd4, 0xb3, 0x7d, 0xfa, 0xef, 0xc5, 0x91, 0x39, 0x72, 0xe4, 0xd3, 0xbd, 
  0x61, 0xc2, 0x9f, 0x25, 0x4a, 0x94, 0x33, 0x66, 0xcc, 0x83, 0x1d, 0x3a, 0x74, 0xe8, 0xcb
};

static void expandKeys( vaes* v ){
  int i, j;
  u8 temp[ 4 ],k;

  for( i = 0; i < numKeys; i++ ){
    v->roundKey[ i * 4 ] = v->key[ i * 4 ];
    v->roundKey[ i * 4 + 1 ] = v->key[ i * 4 + 1 ];
    v->roundKey[ i * 4 + 2 ] = v->key[ i * 4 + 2 ];
    v->roundKey[ i * 4 + 3 ] = v->key[ i * 4 + 3 ];
  }

  while( i < ( Nb * ( numRounds + 1 ) ) ){
    for( j = 0; j < 4; j++ )
      temp[ j ] = v->roundKey[ ( i - 1 ) * 4 + j ];
    if( i % numKeys == 0 ){
      k = temp[ 0 ];
      temp[ 0 ] = temp[ 1 ];
      temp[ 1 ] = temp[ 2 ];
      temp[ 2 ] = temp[ 3 ];
      temp[ 3 ] = k;
      temp[ 0 ] = (u8)sbox[ temp[ 0 ] ];
      temp[ 1 ] = (u8)sbox[ temp[ 1 ] ];
      temp[ 2 ] = (u8)sbox[ temp[ 2 ] ];
      temp[ 3 ] = (u8)sbox[ temp[ 3 ] ];
      temp[ 0 ] = temp[ 0 ] ^ (u8)rcon[ i /numKeys ];
    }else if( /*numKeys > 6 &&*/ i % numKeys == 4 ){
      temp[ 0 ] = (u8)sbox[ temp[ 0 ] ];
      temp[ 1 ] = (u8)sbox[ temp[ 1 ] ];
      temp[ 2 ] = (u8)sbox[ temp[ 2 ] ];
      temp[ 3 ] = (u8)sbox[ temp[ 3 ] ];
    }
    v->roundKey[ i * 4 + 0 ] = v->roundKey[ ( i - numKeys ) * 4 + 0 ] ^ temp[ 0 ];
    v->roundKey[ i * 4 + 1 ] = v->roundKey[ ( i - numKeys ) * 4 + 1 ] ^ temp[ 1 ];
    v->roundKey[ i * 4 + 2 ] = v->roundKey[ ( i - numKeys ) * 4 + 2 ] ^ temp[ 2 ];
    v->roundKey[ i * 4 + 3 ] = v->roundKey[ ( i - numKeys ) * 4 + 3 ] ^ temp[ 3 ];
    i++;
  }
}


static void addRoundKey( int round, vaes* v ){
  int i, j;
  for( i = 0; i < 4; i++ )
    for( j = 0; j < 4; j++ )
      v->state[ j ][ i ] ^= v->roundKey[ round * Nb * 4 + i * Nb + j ];
}

static void subBytes( vaes* v ){
  int i, j;
  for( i = 0; i < 4; i++ )
    for( j = 0; j < 4; j++ )
      v->state[ i ][ j ] = (u8)sbox[ v->state[ i ][ j ] ];
}

static void shiftRows( vaes* v ){
  u8 temp;

  temp = v->state[ 1 ][ 0 ];
  v->state[ 1 ][ 0 ] = v->state[ 1 ][ 1 ];
  v->state[ 1 ][ 1 ] = v->state[ 1 ][ 2 ];
  v->state[ 1 ][ 2 ] = v->state[ 1 ][ 3 ];
  v->state[ 1 ][ 3 ] = temp;
   
  temp = v->state[ 2 ][ 0 ];
  v->state[ 2 ][ 0 ] = v->state[ 2 ][ 2 ];
  v->state[ 2 ][ 2 ] = temp;

  temp = v->state[ 2 ][ 1 ];
  v->state[ 2 ][ 1 ] = v->state[ 2 ][ 3 ];
  v->state[ 2 ][ 3 ] = temp;

  temp = v->state[ 3 ][ 0 ];
  v->state[ 3 ][ 0 ] = v->state[ 3 ][ 3 ];
  v->state[ 3 ][ 3 ] = v->state[ 3 ][ 2 ];
  v->state[ 3 ][ 2 ] = v->state[ 3 ][ 1 ];
  v->state[ 3 ][ 1 ] = temp;
}


#define xtime(x)   ((x<<1) ^ (((x>>7) & 1) * 0x1b))
static void mixColumns( vaes* v ){
  int i;
  u8 Tmp, Tm, t;
  for( i = 0; i < 4; i++ ){    
    t = v->state[ 0 ][ i ];
    Tmp = v->state[ 0 ][ i ] ^ v->state[ 1 ][ i ] ^ v->state[ 2 ][ i ] ^ v->state[ 3 ][ i ] ;
    Tm = v->state[ 0 ][ i ] ^ v->state[ 1 ][ i ] ; Tm = xtime( Tm ); v->state[ 0 ][ i ] ^= Tm ^ Tmp ;
    Tm = v->state[ 1 ][ i ] ^ v->state[ 2 ][ i ] ; Tm = xtime( Tm ); v->state[ 1 ][ i ] ^= Tm ^ Tmp ;
    Tm = v->state[ 2 ][ i ] ^ v->state[ 3 ][ i ] ; Tm = xtime( Tm ); v->state[ 2 ][ i ] ^= Tm ^ Tmp ;
    Tm = v->state[ 3 ][ i ] ^ t ; Tm = xtime( Tm ); v->state[ 3 ][ i ] ^= Tm ^ Tmp ;
  }
}

static void cipher( vaes* v ){
  int i, j, round = 0;

  for( i = 0; i < 4; i++ )
    for( j = 0; j < 4; j++ )
      v->state[ j ][ i ] = v->in[ i * 4 + j ];
 
  addRoundKey( 0, v ); 

  for( round = 1; round < numRounds; round++ ){
    subBytes( v );
    shiftRows( v );
    mixColumns( v );
    addRoundKey( round, v );
  }

  subBytes( v );
  shiftRows( v );
  addRoundKey( numRounds, v );

  for( i = 0; i < 4; i++ )
    for( j = 0; j < 4; j++ )
      v->out[ i * 4 + j ] = v->state[ j ][ i ];
}
void vencrypt( const u8 key[ 32 ], u8 data[ 16 ] ){
  vaes v;
  vmemcpy( v.in, data, 16 );
  vmemcpy( v.key, key, 32 );
  expandKeys( &v );
  cipher( &v );
  vmemcpy( data, v.out, 16 );
}


void vgenKey( u32 lononce, u32 hinonce, u8 key[ 32 ] ){
  u32 i, k;
  static u8 keys[ 2 ][ 32 ] = { { 110, 138, 5, 19, 223, 141, 84, 93, 44, 161, 226, 194, 149, 154, 51, 65, 66, 235, 226, 110, 122, 127, 1, 243, 253, 142, 69, 34, 55, 4, 158, 205 },
                                { 145, 162, 253, 34, 248, 129, 23, 201, 165, 205, 230, 66, 50, 27, 202, 61, 49, 240, 56, 116, 123, 9, 24, 2, 55, 97, 192, 185, 37, 100, 110, 137 } };
  vaes v;
  for( k = 0; k < 2; ++k ){
    for( i = 0; i < 2; ++i ){
      v.in[ i * 8 + 0 ] = ( hinonce >> 0 ) % 256;    
      v.in[ i * 8 + 1 ] = ( lononce >> 0 ) % 256;
      v.in[ i * 8 + 2 ] = ( hinonce >> 8 ) % 256;
      v.in[ i * 8 + 3 ] = ( lononce >> 8 ) % 256;
      v.in[ i * 8 + 4 ] = ( hinonce >> 16 ) % 256;    
      v.in[ i * 8 + 5 ] = ( lononce >> 16 ) % 256;
      v.in[ i * 8 + 6 ] = ( hinonce >> 24 ) % 256;
      v.in[ i * 8 + 7 ] = ( lononce >> 24 ) % 256;
    }
    vmemcpy( v.key, keys[ k ], 32 );
    expandKeys( &v );
    cipher( &v );
    for( i = 0; i < 16; ++i )
      key[ k * 16 + i ] = v.out[ i ];
  }
}

// This is a Mersenne Twister implimenatation based on public domain code by Michael Brundage.
#define MT_LEN 624
#define MT_IA           397
#define MT_IB           (MT_LEN - MT_IA)
#define UPPER_MASK      0x80000000
#define LOWER_MASK      0x7FFFFFFF
#define MATRIX_A        0x9908B0DF
#define TWIST(b,i,j)    ((b)[i] & UPPER_MASK) | ((b)[j] & LOWER_MASK)
#define MAGIC(s)        (((s)&1)*MATRIX_A)

#define prepmt 630
u8* vgenPad( u32 lononce, u32 hinonce, u32 sz ){
  u32 i, j, k;
  u8 pt[ 16 ];
  u32 pt32[ 4 ];
  u8* ans = (u8*)vsmalloc( sz );
  u8 pkey[ 32 ];
  vrandState vlo, vhi;
  vgenKey( lononce, hinonce, pkey );
  vseeds( lononce, &vlo );
  vseeds( hinonce, &vhi );
  for( i = 0; i < prepmt; ++i ){
    vrand32allBitss( &vlo );
    vrand32allBitss( &vhi );
  }
  k = 0;
  while( k < sz ){
    pt32[ 0 ] = vrand32allBitss( &vlo );
    pt32[ 1 ] = vrand32allBitss( &vhi );
    pt32[ 2 ] = vrand32allBitss( &vlo );
    pt32[ 3 ] = vrand32allBitss( &vhi );
    for( i = 0; i < 4; ++i )
      for( j = 0; j < 4; ++j )
        pt[ i * 4 + j ] = ( pt32[ i ] >> ( j * 8 ) ) % 256;
    vencrypt( pkey, pt );
    for( i = 0; i < 16; ++i )
      if( k + i < sz )
        ans[ k + i ] = pt[ i ];
    k += 16;
  }
  return ans;
}



class AESRandImpl{
public:
  AESRandImpl( u32 lononce, u32 hinonce, u32 size );
  ~AESRandImpl();

  const u32& operator[]( u32 idx );
private:
  u32 lononce, hinonce;
  u32 sz;
  u32* data;
};
AESRandImpl::AESRandImpl( u32 ln, u32 hn, u32 size ) : lononce( ln ), hinonce( hn ), sz( size ){
  data = (u32*)vgenPad( lononce, hinonce, sizeof( u32 ) * sz );
}
AESRandImpl::~AESRandImpl(){
  vsfree( data );
}
const u32& AESRandImpl::operator[]( u32 idx ){
  if( idx >= 2147483647 )
    throw "Index out of range (2GB) in AESRandImpl::operator[]";
  if( idx >= sz ){
    u32 nwsz = sz;
    while( nwsz <= idx ) 
      nwsz *= 2;
    vsfree( data );
    data = (u32*)vgenPad( lononce, hinonce, sizeof( u32 ) * nwsz );
    sz = nwsz;
  }
  return data[ idx ];
}
AESRand::AESRand( u64 nonce, u32 size ){ 
  impl = new AESRandImpl( (u32)nonce, (u32)( nonce >> 32 ), size );
}
AESRand::~AESRand(){
  delete impl;
}
const u32& AESRand::operator[]( u32 idx ){
  return (*impl)[ idx ];
}

  