// All code not explictly labled as others work is copyright Jon DuBois 2011.  All rights reserved.

#ifndef VUTIL_H
#define VUTIL_H



#define WIN32_LEAN_AND_MEAN
#pragma warning( push )
#pragma warning( disable: 4668 4820 4255 )
#include <windows.h>
#include <limits.h>
#pragma warning( pop )

#define maxMouseDelta 1000.0f

#define VUTIL_VERSION "8.0a"

#ifdef SAFE
#define assess( t, msg ) if( !( t ) ) vdie( msg )
#else
#define assess( t, msg ) 
#endif

#define assert( t ) if( !( t ) ) vdie( "Assertion failed!" );

#define pi 3.14159265358979323846264338327950288419716939937510
#define torad ( 3.14159265358979323846264338327950288419716939937510f / 180.0f )
#define rootthree 1.7320508075688772935274463415059f


// Types.
typedef unsigned char u8;
typedef unsigned short int u16;
typedef unsigned int u32;
typedef unsigned long long int u64;
typedef signed char s8;
typedef signed short int s16;
typedef signed int s32;
typedef signed long long int s64;
typedef float f32;
typedef double f64;



#if CHAR_BIT != 8 
#error Alien build envioronment, check vutil.h for correct typedefs.
#endif 
#if INT_MAX != 2147483647 || INT_MIN != -2147483648
#error Alien build envioronment, check vutil.h for correct typedefs.
#endif 
#if UINT_MAX != 4294967295 
#error Alien build envioronment, check vutil.h for correct typedefs.
#endif 
#if SHRT_MAX != 32767 || SHRT_MIN != -32768 
#error Alien build envioronment, check vutil.h for correct typedefs.
#endif 
#if USHRT_MAX != 65535 
#error Alien build envioronment, check vutil.h for correct typedefs.
#endif 
#if SCHAR_MAX != 127 || SCHAR_MIN != -128
#error Alien build envioronment, check vutil.h for correct typedefs.
#endif 
#if UCHAR_MAX != 255 
#error Alien build envioronment, check vutil.h for correct typedefs.
#endif 
#if LLONG_MAX != 9223372036854775807 || LLONG_MIN != 9223372036854775808
#error Alien build envioronment, check vutil.h for correct typedefs.
#endif 
#if ULLONG_MAX != 18446744073709551615
#error Alien build envioronment, check vutil.h for correct typedefs.
#endif 

void vqsort( u8* base, int n, int size, int (*compar)( u8*, u8* ) );
// This mallocs but never returns NULL, it dies if out of memory.
void* vsmalloc( u32 sz );
// This allocates some memory and returns a name that can be used to refer to that memory.  
// The memory will be automatically freed at program termination, and should not be manually freed.
u32 vmalloc( u32 sz );
void vsfree( void* data );
void vmemcpy( void* dst, const void* src, u32 sz );
// This returns a pointer to the memory corresponding to a name.  This is only valid as 
// long as no other functions are called with this name.
void* vmem( u32 i );
// Returns the amount of memory allocated to a name.
u32 vsize( u32 i );
// This erases a name so it empty, it does not shrink the buffer.
void verase( u32 i );
// This appends information onto the back of the memory named i, growing as neccesary.
void vappend( u32 i, const void* m, u32 sz );
// This grows a memory by sz u8, padding with zeroes.
void vgrow( u32 i, u32 sz );
// This shrinks the size by sz.
void vpop( u32 i, u32 sz );
// Same, but for a null terminated string.  This will automatically insert a trailing nul.
void vappendString( u32 i, const u8* msg );
// Appends a 2 character hexcode for b onto i
void vappendHexByte( u32 i, u8 b );
// Same, but dosnt terminate with a nul.
void vappendStringNoNul( u32 i, const u8* msg );
// Pops a nul off the end if there is one.
void vpopNul( u32 i );
// Appends the string representation of an int.
void vappendInt( u32 i, int msg, u32 minWidth );
// Returns the value of an int.  Parses until the first non-digit character.
u32 vreadInt( const u8* p );

// Only valid until next call
const u8* vintToString( int i, u32 minWidth );
// "                         ", returns A for 0, Z for 26, AA for 27, -A for -1, -Z for -25 etc. 
const u8* vintToName( int i );

const u8* vgetResourceOrDie( u32 handle, u32* sz, const u8* type );

typedef struct{
  u32 data;
  u32 bits;
} vbitField;

// if bf is NULL this creates a new one.
vbitField* vresetBitField( vbitField* bf );
void vaddBits( vbitField* bf, const u8* data, u32 bits );
int vgetBit( const vbitField* bf, u32 bit );
u32 vgetBits( const vbitField* bf, u32 bit, u32 num );
void vsetBit( vbitField* bf, u32 bit, int val );
#define vbfSize( x ) ( ( (x)->bits ) ? ( ( (x)->bits - 1 ) / 8 + 1 ) : 0 )

// return string is only valid until next call to this function.
const u8* vbitFieldToString( const vbitField* bf, u32 bitsperline );

// The number of seconds since program start.
f32 vcurTime( void );
// a 64-bit nonce based on the current UTC that is unlikley to repeat.
void vtimeNonce( u32* lononce, u32* hinonce );



// This loads a file and returns a handle to it's contents in memory. it returns 0 if the file couldn't be loaded.
u32 vloadFile( const u8* file );
// The same, but never returns NULL, just dies.
u32 vloadFileOrDie( const u8* file );
// ditto for file writing.
int vwriteFile( const u8* file, const u8* data, u32 sz );
void vwriteFileOrDie( const u8* file, const u8* data, u32 sz );

// Dies.
void vdie( const u8* msg );

// Asks a yes or no question, returns 1 for yes, 0 for no.
int vquery( const u8* qst );

// Random functions (mersenne twister).
typedef struct{
  u32 index;
  u32 buffer[ 624 ];
} vrandState;
void vseed( u32 seed, vrandState* vs );
// Returns a value between min and max.
f32 vrand( f32 min, f32 max, vrandState* vs );
u32 vrand32( u32 min, u32 max, vrandState* vs );
u32 vrand32allBits( vrandState* vs );


f32 vabs( f32 x );
f32 vsqrt( f32 x );
f32 vsin( f32 x );
f32 vcos( f32 x );
f32 vtan( f32 x );
f32 vrem( f32 x, f32 q );

void vmemset( void* d, u8 c, u32 sz );
#define vzero( x ) vmemset( &x, 0, sizeof( x ) )
int vstrcmp( const u8* c1, const u8* c2 );
void vstrcpy( u8* d, const u8* s );
u32 vstrlen( const u8* );
const u8* vstrstr( const u8* s, const u8* ss );
int visspace( int c );


// This returns a pointer to a static buffer, it will be reset
// by each call to this funnctions
u8* vwordToBytes( u32 v );
u32 vbytesToWord( const u8* p );

void QuickSort(void *base, int count, int size,
  int (*compare)(const void *, const void *) );

// Encrypts a single 128-bit data block using the AES-256 key key.
void vencrypt( const u8 key[ 32 ], u8 data[ 16 ] );
// Generates a key from a 64-bit nonce.
void vgenKey( u32 lononce, u32 hinonce, u8 key[ 32 ] );
// Generates a one-time pad from a 64-bit nonce using AES-256 in counter mode.
// The returned pointer must be vsfree'd.
u8* vgenPad( u32 lononce, u32 hinonce, u32 sz ); 




#endif // VUTIL_H


