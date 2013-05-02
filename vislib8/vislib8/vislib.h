#ifndef VL_VISLIB_H
#define VL_VISLIB_H



#define WIN32_LEAN_AND_MEAN
#pragma warning( push )
#pragma warning( disable: 4668 4820 4255 )
#include <windows.h>
#include <Wingdi.h>
#include <gl\gl.h>
#include "glext.h"
#include "wglext.h"
#include <limits.h>
#pragma warning( pop )

#define minwidth 100
#define minheight 100

#define logy 40
#define logx 80
#define logwidth 700
#define logheight 500

#define maxMouseDelta 1000.0f

#define VISLIB_VERSION "8.0a"
//bugbug
#define vislibRequiredOpenGLMajorVersion 2
#define vislibRequiredOpenGLMinorVersion 1


// If a frame is rendered in under this number of milliseconds, vislib will sleep to let the operating system have some time.
// 250 fps seems good, that would still be 125 fps stereoscopically.
#ifndef SAFE
#define VL_MINIMUM_MILLISECONDS 1
#else
#define VL_MINIMUM_MILLISECONDS 1
#endif

#include "glprot.h"

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
#error Alien build envioronment, check vislib.h for correct typedefs.
#endif 
#if INT_MAX != 2147483647 || INT_MIN != -2147483648
#error Alien build envioronment, check vislib.h for correct typedefs.
#endif 
#if UINT_MAX != 4294967295 
#error Alien build envioronment, check vislib.h for correct typedefs.
#endif 
#if SHRT_MAX != 32767 || SHRT_MIN != -32768 
#error Alien build envioronment, check vislib.h for correct typedefs.
#endif 
#if USHRT_MAX != 65535 
#error Alien build envioronment, check vislib.h for correct typedefs.
#endif 
#if SCHAR_MAX != 127 || SCHAR_MIN != -128
#error Alien build envioronment, check vislib.h for correct typedefs.
#endif 
#if UCHAR_MAX != 255 
#error Alien build envioronment, check vislib.h for correct typedefs.
#endif 
#if LLONG_MAX != 9223372036854775807 || LLONG_MIN != 9223372036854775808
#error Alien build envioronment, check vislib.h for correct typedefs.
#endif 
#if ULLONG_MAX != 18446744073709551615
#error Alien build envioronment, check vislib.h for correct typedefs.
#endif 


#include "geometry.h"

extern int vkeys[ 256 ];
extern int vkeyReleases[ 256 ];
extern int vkeyPresses[ 256 ];
extern u32 vxpos;
extern u32 vypos;
extern u32 vwidth;
extern u32 vheight;
extern u32 vscreenWidth;
extern u32 vscreenHeight;
extern int vactive;
extern int vfullscreen;

#define VL_LBUTTON 0
#define VL_RBUTTON 1
#define VL_MBUTTON 2
#define VL_X1BUTTON 3
#define VL_X2BUTTON 4

extern int vmouseButtons[ 5 ];


#define vsleep Sleep

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

// if bf is NULL creates a new one.
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

// This logs an event into a dynamic memory resident log that gets freed at program termination.  It does nothing if
// passed a NULL pointer. It always returns a pointer to the log, the pointer is only valid until the next call to vlog.
const u8* vlog( const u8* msg );
const u8* vlogInfo( const u8* msg );
const u8* vlogWarning( const u8* msg );
const u8* vlogError( const u8* msg );
// This returns a string containing system information.
const u8* vsysInfo( void );
// This returns a string containing the OpenGL extensions.
const u8* vglExtensions( void );
// This rteturns 1 if the extension is present, and 0 otherwise.
int vglExtensionAvailable( const u8* ext );



// Activates OpenGL capabilities, this must be called in order to get a render context.
void vginit( u8* title, u32 width, u32 height, u32 x, u32 y, u32 bits, int border );

// returns 0 on failure.  data must be freed with vsfree.
int vloadImageFromFile( const u8* filename, u8** data, u32* width, u32* height );
// returns 0 on failure.  data must be freed with vsfree.
int vloadImageFromBytes( const u8* inData, u32 inSize, u8** data, u32* width, u32* height );



typedef struct{
  u32 bpp, redBits, greenBits, blueBits, alphaBits, redShift, greenShift, blueShift, alphaShift, depthBits, stencilBits, auxBuffers, accumBits;
  int doubleBuffer, format;
} pixelFormat;
u32 vglNumPixelFormats( void );
// This returns a string containing information on all supported pixel formats.
const u8* vglPixelFormatsString( void );
// This returns a string containing information on a specific pixel format.  The pointer
// returned is a static buffer that is only valid until the next call to this function.
const u8* vglPixelFormatString( const pixelFormat* pf );
// This returns the pixel format meeting the requirments with the least storage requirments, or NULL if there are none.
// if a max number is negative, then there is no maximum for that value.
const pixelFormat* vglFindPixelFormat( int minRedBits, int maxRedBits, int minGreenBits, int maxGreenBits, int minBlueBits, int maxBlueBits, int minAlphaBits, int maxAlphaBits, int minDepthBits, int maxDepthBits, int minStencilBits, int maxStencilBits, int minAccumBits, int maxAccumBits, int minAuxBuffers, int maxAuxBuffers, int requireDB, int requireNoDB, int requireBGR, int requireRGB );
// Vsync Control
void vglSwapControl( u32 f );
// Checks for OpenGL errors.
void vglGetError( void );
// replacement for gluPerspective
void vglPerspective( f32 fov, f32 aspect, f32 near, f32 far );


// This loads a file and returns a handle to it's contents in memory. it returns 0 if the file couldn't be loaded.
u32 vloadFile( const u8* file );
// The same, but never returns NULL, just dies.
u32 vloadFileOrDie( const u8* file );
// ditto for file writing.
int vwriteFile( const u8* file, const u8* data, u32 sz );
void vwriteFileOrDie( const u8* file, const u8* data, u32 sz );

// Dies.
void vdie( const u8* msg );

// Informs the user of something.
void vinform( const u8* msg, const u8* title );
// Asks a yes or no question, returns 1 for yes, 0 for no.
int vquery( const u8* qst );

// Random functions (mersenne twister).  The ones that dont take a
// vrandState pointer use a global one.
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


// This adjusts the window.
void vmoveWindow( s32 nx, s32 ny, s32 nw, s32 nh );
void vactivate( void );
void vdeactivate( void );
void vshowRenderWindow( void );
void vhideRenderWindow( void );
void vshowLogWindow( void );
void vhideLogWindow( void );

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


// Compresses data in a crytopgraphicly secure way, the returned memory name is reset with each call.
u32 vcompress( const u8* data, u32 sz );
u32 vdecompress( const u8* data, u32 sz );


// Archives
typedef struct{
  u32 size;
  u8** names;
  u8** datas;
  u32* sizes;
//private
  u32 buffsize;
} varchive;

varchive* vnewArchive( void );
varchive* vnewArchiveFromBytes( const u8* data, u32 sz );
void vdeleteArchive( varchive* varc );
// If this name is already used, its data will be overwritten.
void vaddToArchive( varchive* varc, const u8* name, const u8* data, u32 size );
// Returns the archives size if the name doesn't exist.
u32 vgetArchiveIndexByName( varchive* varc, const u8* name );
// Returns NULL if the name doesn't exist.
const u8* vgetArchiveDataByName( varchive* varc, const u8* name );
// Returns an archive as a single block of data, the returned memory
// name is only valid until the next call to this function.
u32 varchiveToBytes( varchive* varc );



extern HDC vdc;
extern HGLRC vrc;

#include "glfuncs.h"
#include "shaders.h"
#include "surfaces.h"
#include "net.h"

extern surface vscreenTex;

#endif // VL_VISLIB_H




// This is a structure containing the information vislib needs to start. 
typedef struct {
  u32 logX, logY;
  u32 logWidth, logHeight;
  const u8* logName;
  int showLog;
} vinitInfo;

// These are the functions that do the good part :D  These must all
// be supplied by the client to this library

// commandline is a single string containing everything on the 
// command line when the program was invoked, not including the module name.
// init might get called with a NULL commandline because Windows(TM)
// is terrible.  Be sure to check for NULL.  It may also be
// an empty string if it was able to get the command line but there
// were no arguments.
// a reminder: DO NOT CALL GRAPHICS CODE UNTIL YOU CALL VGINIT
vinitInfo* init( const u8* commandline );
void resize( u32 x, u32 y, u32 width, u32 height );
// delta is the number of seconds elapsed since last tick, x and ydelta are the relative mouse movement in mickeys.
// If this returns non-zero then the program terminates.
int tick( f32 delta, f32 xdelta, f32 ydelta, f32 mwheelDelta );
void cleanup( void );
