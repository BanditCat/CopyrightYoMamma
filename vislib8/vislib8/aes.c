#include "vislib.h"

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
    }else if( numKeys > 6 && i % numKeys == 4 ){
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
  u8* ans = vsmalloc( sz );
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


