// All code not explictly labled as others work is copyright Jon DuBois 2011.  All rights reserved.

#include "vutil.h"

extern int _fltused = 0;

static LARGE_INTEGER vhpFrequency;
static LARGE_INTEGER vclock;
static LARGE_INTEGER vstartClock;

static u32 vallocCount = 0;
static u32 vallocBuffSize = 0;
static void** vallocs = NULL;
static u32* vallocCounts = NULL;
static u32* vallocBuffSizes = NULL;

void* vsmalloc( u32 sz ){
  void* nb = GlobalAlloc( GMEM_FIXED, sz );
  if( nb == NULL ) 
    vdie( "Unable to allocate memory!." );
  return nb;
}


u32 vmalloc( u32 asz ){
  u32 sz = asz ? asz : 1;
  if( vallocs == NULL ){
    vallocs = vsmalloc( sizeof( void* ) );
    vallocCounts = vsmalloc( sizeof( u32 ) );
    vallocBuffSizes = vsmalloc( sizeof( u32 ) );
    vallocCount = 0;
    vallocBuffSize = 1;
  }
  while( vallocCount >= vallocBuffSize ){
    void* nb = vsmalloc( sizeof( void* ) * vallocBuffSize * 2 );
    vallocBuffSize *= 2;
    vmemcpy( nb, vallocs, sizeof( void* ) * vallocCount );
    vsfree( vallocs ); vallocs = nb;
    nb = vsmalloc( sizeof( u32 ) * vallocBuffSize );
    vmemcpy( nb, vallocCounts, sizeof( u32 ) * vallocCount );
    vsfree( vallocCounts ); vallocCounts = nb;
    nb = vsmalloc( sizeof( u32 ) * vallocBuffSize );
    vmemcpy( nb, vallocBuffSizes, sizeof( u32 ) * vallocCount );
    vsfree( vallocBuffSizes ); vallocBuffSizes = nb;
  }
  vallocs[ vallocCount ] = vsmalloc( sz );
  vallocCounts[ vallocCount ] = 0;
  vallocBuffSizes[ vallocCount ] = sz;
  return ++vallocCount;
}
void vsfree( void* data ){
  GlobalFree( data );
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
    vdie( "Min width too large in vintToString." );
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
    vdie( "Min width too large in vintToString." );
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
    return "A";
  if( i == -1 )
    return "-A";
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
  vappendString( i, buf );
  if( ln < 10 )
    buf[ 0 ] = '0' + ln;
  else
    buf[ 0 ] = 'A' + ln - 10;
  vappendString( i, buf );
}
void vappendStringNoNul( u32 i, const u8* msg ){
  vappendString( i, msg );
  --vallocCounts[ i - 1 ];
}
void vpopNul( u32 i ){
  if( vallocCounts[ i - 1 ] && !( ( (u8**)vallocs )[ i - 1 ][ vallocCounts[ i - 1 ] - 1 ] ) )
    vpop( i, 1 );  
}
void vcleanup( void ){
  u32 i;


  if( vallocs != NULL ){
    for( i = 0; i < vallocCount; ++i )
      if( vallocs[ i ] != NULL )
        vsfree( vallocs[ i ] );
    vsfree( vallocs );
    vallocs = NULL;
    vallocCount = 0;
    vallocBuffSize = 0;
    if( vallocCounts != NULL )
      vsfree( vallocCounts );
    vallocCounts = NULL;
    if( vallocBuffSizes != NULL )
      vsfree( vallocBuffSizes );
    vallocBuffSizes = NULL;
  }
  ExitProcess( 0 );
}
u32 vloadFile( const u8* file ){
  u32 ans = 0;
  int done = 0;
  static u8 buf[ 4100 ];
  DWORD br;
  HANDLE f = CreateFile( file, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
  if( f == INVALID_HANDLE_VALUE )
    return 0;
  ans = vmalloc( 1 );
  while( !done ){
    if( FALSE == ReadFile( f, &buf, 4096, &br, NULL ) )
      done = 1;
    if( !br )
      done = 1;
    if( !done )
      vappend( ans, buf, br );
  }
  vappend( ans, "", 1 );
  vpopNul( ans );
  CloseHandle( f );
  return ans;
}
u32 vloadFileOrDie( const u8* file ){
  u32 ans = vloadFile( file );
  if( !ans )
    vdie( "Failed to load file!" );
  return ans;
}
int vwriteFile( const u8* file, const u8* data, u32 sz ){
  int ans = 1;
  DWORD bw;
  HANDLE f = CreateFile( file, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
  if( f == INVALID_HANDLE_VALUE )
    return 0;
  if( FALSE == WriteFile( f, data, sz, &bw, NULL ) )
    ans = 0;
  if( !bw )
    ans = 0;
  CloseHandle( f );
  return ans;
}
void vwriteFileOrDie( const u8* file, const u8* data, u32 sz ){
  int ans = vwriteFile( file, data, sz );
  if( !ans )
    vdie( "Failed to write file!" );
}
void vdie( const u8* msg ){
  u32 msz = vstrlen( msg );

  if( MessageBox( NULL, msg, "System Error. Copy message to clipboard?", MB_OK | MB_ICONEXCLAMATION | MB_SYSTEMMODAL | MB_YESNO ) == IDYES ){
    u8* tp = NULL;
    HANDLE hnd;
    if( !OpenClipboard( NULL ) ){
      vcleanup(); return;
    }
    if( !EmptyClipboard() ){
      CloseClipboard();
      vcleanup(); return;
    }
    if( ( ( hnd = GlobalAlloc( GMEM_MOVEABLE, msz + 2 ) ) == NULL ) || ( ( tp = GlobalLock( hnd ) ) == NULL ) ){
      if( hnd != NULL )
        GlobalFree( hnd );
      CloseClipboard();
      vcleanup(); return;
    }

    vmemcpy( tp, msg, msz + 1 );
    GlobalUnlock( hnd );
    SetClipboardData( CF_TEXT, tp );
    CloseClipboard(); 
  }
  vcleanup();
}



// This is a Mersenne Twister implimenatation based on public domain code by Michael Brundage.
#define MT_LEN 624

void vseed( u32 s, vrandState* vs ){
  u32 i;
  vs->index = 0;
  vs->buffer[ 0 ] = (u32)s;
  for( i = 1; i < 624; ++i )
    vs->buffer[ i ] = ( 1812433253 * ( vs->buffer[ i - 1 ] ^ ( ( vs->buffer[ i - 1 ] ) >> 30 ) ) + i );
}

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
f32 vrand( f32 min, f32 max, vrandState* vs ){ return (f32)( pvrand( vs ) ) / (f32)UINT_MAX * ( max - min ) + min; }
u32 vrand32( u32 min, u32 max, vrandState* vs ){ return ( pvrand( vs ) % ( 1 + max - min ) ) + min; }
u32 vrand32allBits( vrandState* vs ){ return pvrand( vs ); }

vbitField* vresetBitField( vbitField* bf ){ 
  if( bf != NULL ){
    bf->bits = 0;
    verase( bf->data );
    return bf;
  } else{
    vbitField* ans = vmem( vmalloc( sizeof( vbitField ) ) );
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
  return vmem( ans );
}
f32 vcurTime( void ){
  LARGE_INTEGER vclock;
  if( !QueryPerformanceCounter( &vclock ) )
    vdie( "Unable to query high performance timer" );
  return (f32)( vclock.QuadPart - vstartClock.QuadPart ) / (f32)( vhpFrequency.QuadPart );
}
void vtimeNonce( u32* lononce, u32* hinonce ){
  static const u8 mkey[ 32 ] = { 224, 64, 187, 5, 235, 1, 254, 61, 157, 63, 20, 128, 99, 62, 27, 85, 121, 218, 245, 39, 118, 166, 19, 58, 57, 68, 138, 120, 240, 154, 64, 57 };
  static u32 lastlo = 0;
  static u32 lasthi = 0;
  static u32 fudge = 0;
  SYSTEMTIME st;
  FILETIME ft;
  GetSystemTime( &st );
  SystemTimeToFileTime( &st, &ft );
  if( lastlo == ft.dwLowDateTime && lasthi == ft.dwHighDateTime )
    ++fudge;

  lastlo = ft.dwLowDateTime;
  lasthi = ft.dwHighDateTime;

  {
    u32 i;
    static u8 pt[ 16 ];
    u32 lh = lasthi + fudge;
    u32 ll = lastlo + fudge;
    for( i = 0; i < 2; ++i ){
      pt[ i * 8 + 0 ] = ( lh >> 0 ) % 256;    
      pt[ i * 8 + 1 ] = ( ll >> 0 ) % 256;
      pt[ i * 8 + 2 ] = ( lh >> 8 ) % 256;
      pt[ i * 8 + 3 ] = ( ll >> 8 ) % 256;
      pt[ i * 8 + 4 ] = ( lh >> 16 ) % 256;    
      pt[ i * 8 + 5 ] = ( ll >> 16 ) % 256;
      pt[ i * 8 + 6 ] = ( lh >> 24 ) % 256;
      pt[ i * 8 + 7 ] = ( ll >> 24 ) % 256;  
    }
    vencrypt( mkey, pt );
    *lononce = *( ( (u32*)pt ) + 0 );
    *hinonce = *( ( (u32*)pt ) + 1 );
  }
}




f32 vabs( f32 x ){
  float ans;
  __asm{
    fld [x]
    fabs
      fstp [ans]
  }
  return ans;
}
f32 vsqrt( f32 x ){
  float ans;
  __asm{
    fld [x]
    fsqrt
      fstp [ans]
  }
  return ans;
}
f32 vsin( f32 x ){
  float ans;
  __asm{
    fld [x]
    fsin
      fstp [ans]
  }
  return ans;
}
f32 vcos( f32 x ){
  float ans;
  __asm{
    fld [x]
    fcos
      fstp [ans]
  }
  return ans;
}
#pragma warning( push )
#pragma warning( disable:4725 )
f32 vtan( f32 x ){
  float ans;
  __asm{
    fld [x]
    fptan
      fstp [ans]
    fstp [ans]
  }
  return ans;
}
f32 vrem( f32 x, f32 q ){
  float ans, d;
  __asm{
    fld [q]
    fld [x]
    fprem
      fstp [ans]
    fstp [d]
  }
  return ans;
}
#pragma warning( pop )
void vmemset( void* d, u8 c, u32 sz ){
  __asm{
    cld
      mov    edi, d
      mov    ecx, [sz]
    mov    al, [c]
    repz stos al
  }
}
void vstrcpy( u8* d, const u8* s ){
  while( *s )
    *d++ = *s++;
  *d = '\0';
}
int vstrcmp( const u8* c1, const u8* c2 ){
  while( *c1 && *c2 ){
    if( *c1 < *c2 )
      return -1;
    if( *c1 > *c2 )
      return 1;
    ++c1;
    ++c2;
  }
  if( *c1 )
    return 1;
  if( *c2 )
    return -1;
  return 0;
}
u32 vstrlen( const u8* s ){
  const u8* e = s;
  while( *e )
    ++e;
  return e - s;
}
const u8* vstrstr( const u8* s, const u8* ss ){
  const u8* se = s;
  const u8* sse = ss;
  const u8* b;
  u32 sslen;
  u32 sslst;

  while( *se )
    ++se;
  while( *sse )
    ++sse;
  sslen = sse - ss;
  sslst = sslen - 1;

  if( !sslen )
    return s;
  if( (u32)( se - s ) < sslen )
    return NULL;

  for( b = s + sslst; b < se; ++b ){
    const u8* n = ss + sslst;
    const u8* h = b;
    do
    if( *h != *n )
      goto loop;
    while( --n >= ss && --h >= s );
    return h;
loop:;
  }

  return NULL;
}
int visspace( int c ){
  return ( c == '\n' ) || ( c == '\f' ) || ( c == '\r' ) || ( c == '\t' ) || ( c == '\v' ) || ( c == ' ' );
}

// This qsort is based on a public domain version by Nelson Beebe.  

#define QSORT_THRESHHOLD  4
#define QSORT_MTHRESHHOLD 6

static int qsz;
static int thresh;
static int mthresh;

static int (*qcmp)( u8*, u8* );
static void qst( u8* base, u8* max );

void vqsort( u8* base, int n, int size, int (*compar)( u8*, u8* ) ){
  u8* i;
  u8* j;
  u8* lo;
  u8* hi;
  u8* min;
  u8 c;
  u8* max;

  if( n <= 1 ) return;
  qsz = size;
  qcmp = compar;
  thresh = qsz * QSORT_THRESHHOLD;
  mthresh = qsz * QSORT_MTHRESHHOLD;
  max = base + n * qsz;
  if( n >= QSORT_THRESHHOLD ) {
    qst( base, max );
    hi = base + thresh;
  }else 
    hi = max;
  for( j = lo = base; (lo += qsz) < hi; )
    if( (*qcmp)( j, lo ) > 0 )
      j = lo;
  if( j != base )
    for( i = base, hi = base + qsz; i < hi; ){
      c = *j;
      *j++ = *i;
      *i++ = c;
    }
    for( min = base; ( hi = min += qsz ) < max; ){
      while( (*qcmp)( hi -= qsz, min ) > 0);
      if( ( hi += qsz ) != min ){
        for( lo = min + qsz; --lo >= min; ) {
          c = *lo;
          for( i = j = lo; ( j -= qsz ) >= hi; i = j )
            *i = *j;
          *i = c;
        }
      }
    }
}
static void qst( u8* base, u8* max ){
  u8* i;
  u8* j;
  u8* jj;
  u8* mid;
  int ii;
  u8 c;
  u8 *tmp;
  int lo;
  int hi;

  lo = (int)( max - base );
  do{
    mid = i = base + qsz * ( (unsigned)( lo / qsz ) >> 1 );
    if( lo >= mthresh ){
      j = ( (*qcmp)( ( jj = base ), i ) > 0 ? jj : i );
      if ( (*qcmp)( j, ( tmp = max - qsz ) ) > 0 ){
        j = ( j == jj ? i : jj );
        if( (*qcmp)( j, tmp ) < 0 ) j = tmp;
      }
      if( j != i ){
        ii = qsz;
        do{
          c = *i;
          *i++ = *j;
          *j++ = c;
        }while( --ii );
      }
    }
    for( i = base, j = max - qsz;; ){
      while( i < mid && (*qcmp)( i, mid ) <= 0 )
        i += qsz;
      while( j > mid ){
        if( (*qcmp)( mid, j ) <= 0 ){
          j -= qsz;
          continue;
        }
        tmp = i + qsz;
        if( i == mid ){	
          mid = jj = j;
        }else {
          jj = j;
          j -= qsz;
        }
        goto swap;
      }
      if( i == mid ){
        break;
      }else {
        jj = mid;
        tmp = mid = i;
        j -= qsz;
      }
swap:
      ii = qsz;
      do{
        c = *i;
        *i++ = *jj;
        *jj++ = c;
      } while( --ii );
      i = tmp;
    }
    i = ( j = mid ) + qsz;
    if( ( lo = (int)( j - base ) ) <= ( hi = (int)( max - i ) ) ){
      if( lo >= thresh )
        qst( base, j );
      base = i;
      lo = hi;
    }else {
      if( hi >= thresh )
        qst( i, max );
      max = j;
    }
  } while( lo >= thresh );
}

