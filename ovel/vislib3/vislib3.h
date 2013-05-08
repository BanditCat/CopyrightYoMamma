#ifndef VISLIB3_H
#define VISLIB3_H




// This is the "main" function instead of int main( int, char** ).
int vlMain( int argc, const char** argv );

#define VISLIB_VERSION_MAJOR 3
#define VISLIB_VERSION_MINOR 0
#define VISLIB_VERSION_STATUS 'a'
#define VISLIB_VERSION_STRING "3.0a"

#include "types.h"



typedef struct vlWidgetStruct vlWidget;



//////////////////////////////////////////////////////////////////////////////
// General
//////////////////////////////////////////////////////////////////////////////
#define pi 3.1415926535897932384626433832795028841971693993751058
#define torad ( dpi * (f64)2 / (f64)360 )

#ifndef PROD
#define vlAssess( t, msg ) { if( !( t ) ) vlDie( msg ); }
#else
#define vlAssess( t, msg ) 
#endif

void vlRunString( const u8* src, const u8* srcName );
void vlRunStringNonFatal( const u8* src, const u8* srcName );
const u8* vlLoadString( const u8* src, const u8* srcName );

#define vlCtrl 1
#define vlAlt 4
#define vlShift 16
#define vlSystem 64

#define vlItalic 4
#define vlBold 16
#define vlUnderline 64

typedef struct{
  u8 modifiers;
  u8 code;
  u8 chr;
} vlKey;
#include "keys.h"
// Returns a string representation of a key in a static buffer that only remains valid until this function is called again.
const u8* vlKeyString( const vlKey* key );
// Gets a key from a string.  Returns 1 on success and 0 on failure.
int vlStringKey( const u8* name, vlKey* key );

// This is the struct that holds all global data.
typedef struct{
  u32 numThreads;
  u32 dispatchThread;
  void* luaGlobal;
  void* osGlobal;
} vlGlobal;

// This takes access to the global state, IT MUST BE RELEASED TO PREVENT RACE CONDITIONS!
vlGlobal* vlGetGlobal( void );
void vlReleaseGlobal( void );


// Initialization function.  You must call this before using any functions.
void vlInit( void );
// This should be called to exit the program instead of exit.
void vlQuit( void );
// Exits with an error message, if msg is NULL, it says something generic.
void vlDie( const u8* msg );
// Informs the user with a msgbox
void vlInform( const u8* msg, const u8* title );
int vlQuery( const u8* msg, const u8* title );
void vlInformError( const u8* msg, const u8* title );
#define vlInformInt( n ) {\
  u8 tbf[ 250 ];\
  sprintf( tbf, "num: %i", n );\
  vlInform( tbf, "A Number!" );\
}
// Suspends thread execution for a certian number of milliseconds, passing control back to the OS.
void vlSleep( u32 msec );
// This is the function that checks for and dispatches messages.
void vlWaitMessage( void );
// This gets the thread id of the calling thread.
u32 vlGetThreadId( void );

/////////////////////////////////////////
// UTF functions.
//
// These last argument to all these functions is a error function
// that gets called with an error message if there is a error in the
// unicode. It will continue in spite of the error if the error handler
// returns.

// Converts from a utf-32 code point into a little-endian
// utf-16 code point. The upper word will be 0 if it is not
// a suragote pair.  Result is undefined for an invalid code point.
u32 vlUTF32ToUTF16( u32 cp );
// Same as above for UTF-8.  The most signifigant byte of the result is the first
// UTF-8 byte.  All non-used bytes are 0.  Result is undefined for an invalid code point.
u32 vlUTF32ToUTF8( u32 cp );

// This calls func with each 32-bit code point in str, along with a memory
// handle to the result which will be returned.
// str is of length sz.  This function calls errfunc on any invalid utf-8
// encodings in str.
// BCJBCJ BE CAREFUL! This returns a memory handle you may want to vlAutoFree when done with it.
void vlUTF8Meta( const u8* str, u32 sz, u32 ans, void (*func)( u32 cp, u32 data ), void (*errfunc)( const u8* estr ) );
// Same as above for UTF-16. handles either endianness.
void vlUTF16Meta( const u8* str, u32 sz, u32 ans, int defaultToLittleEndian, void (*func)( u32 cp, u32 data ), void (*errfunc)( const u8* estr ) );
// These convert from UTF-8 to other translation formats.
// BCJBCJ BE CAREFUL! These returns a memory handle you may want to vlAutoFree when done with it.
// if le is non-zero then it is little endian, big endian otherwise, if
// bom is non-zero then a byte order marker is inserted at the beginning.
u32 vlUTF8ToUTF16( const u8* str, u32 sz, void (*errfunc)( const u8* estr ), int bom, int le );
// Converts from UTF-16 to UTF-8.  In the absence of a byte order marker, defaultToLittleEndian determines
// wether it is interpreted as UTF-16LE or UTF-16BE.
u32 vlUTF16ToUTF8( const u8* str, u32 sz, int defaultToLittleEndian, void (*errfunc)( const u8* estr ) );
// These give the length, in bytes, of a zero terminated UTF string.
u32 vlUTF16Len( const u8* str );
u32 vlUTF32Len( const u8* str );
// This returns the byte position in a UTF8 string from a character position.
// The result is undefined if the utf-8 string has errors.
u32 vlUTF8BytePos( const u8* str, u32 sz, u32 cp );
// Inverse of the above: this function returns the character position from a byte
// position.  This is also undefined for invalid UTF strings.
u32 vlUTF8CharPos( const u8* str, u32 sz, u32 bp );

//////////////////////////////////////////////////////////////////////////////
// Memory functions
//////////////////////////////////////////////////////////////////////////////
// This mallocs but never returns NULL, it dies if out of memory.
void* vlMalloc( u32 sz );
// Frees vlMalloc'd memory.
void vlFree( void* data );
// Reallocs as per ansi realloc.
void* vlRealloc( void* data, u32 sz );

// These are reentrant global versions that return pointers that can be
// shared across threads.  These should almost never be used.
void* vlGlobalMalloc( u32 sz );
void vlGlobalFree( void* data );


// This allocates some memory and returns a name that can be used to refer to that memory.  
// The memory will be automatically freed at program termination if it still exists.
// This never returns 0: valid indices start at 1.
u32 vlAutoMalloc( u32 sz );
// The compliment to the above.
void vlAutoFree( u32 i );
// This returns a pointer to the memory corresponding to a name.  The returned pointer
// is only valid as long as no other functions are called with this name.
void* vlMem( u32 i );
// This returns the size of a mem.
u32 vlSize( u32 i );
// This removes a memory chunk of size sz at position pos from the memory named i, and copy anything above down.
void vlRemove( u32 i, u32 pos, u32 sz );
// This pops a trailing '\0' from a memory if it has one.
void vlPopNul( u32 i );
// This appends information onto the back of the memory named i, growing as neccesary.
void vlAppend( u32 i, const void* m, u32 sz );
// This grows a memory by sz bytes, padding with zeroes.
void vlGrow( u32 i, u32 sz );
// Same, but for a null terminated string.  This will automatically insert a trailing null.
void vlAppendString( u32 i, const u8* msg );
// Appends the string representation of an int.
void vlAppendInt( u32 i, s64 msg );

//////////////////////////////////////////////////////////////////////////////
// File functions
//////////////////////////////////////////////////////////////////////////////
// Returns 0 or a memory handle to the loaded file.
// BCJBCJ BE CAREFULL! you probably want to vlAutoFree it as soon as you're done.
u32 vlLoadFile( const u8* name );
// Returns non-zero on success or 0 on falure.  The fourth paramter specifies whether to
// overwrite an exiting file, if it is zero then writing to an existing file fails.
int vlSaveFile( const u8* name, const u8* data, u32 sz, int overwrite );


//////////////////////////////////////////////////////////////////////////////
// Thread functions
//////////////////////////////////////////////////////////////////////////////
// The number of currently running threads.
u32 vlNumThreads( void );
// Creates a new thread running src, with parent thread id pid and inheritance inh
// You better use vlGlobalMalloc instead of vlMalloc to allocate these strings or you will
// have problems.
void vlNewThread( const u8* src, const u8* pid, const u8* inh, const u8* srcName );
// Posts an interthread communication message.  Does nothing if the specified
// thread is invalid.
void vlItcPost( u32 thread, const u8* msg, u32 msz ); 
// Retrieves an ITC and returns a memory handle to it, or 0 if there is none.
// BCJBCJ BE CAREFULL! you probably want to vlAutoFree it as soon as you're done.
u32 vlItcRetrieve( u32 thread );
// Returns the thread number of the ide thread if it exists and is running, or 0
// otherwise.
u32 vlIdeThread( void );

//////////////////////////////////////////////////////////////////////////////
// Misc functions
//////////////////////////////////////////////////////////////////////////////
// This returns the number of equals needed to long quote a lua string.
u32 vlGetNumQuoteEquals ( const u8* );
// Returns a memory handle to the short version of the path.
// BCJBCJ BE CAREFULL! you probably want to vlAutoFree it as soon as you're done.
u32 vlStripPath( const u8* name );
// Prints to the console. If err is nonzero, it goes to stderr else to stdout.
void vlConsolePrint( const u8* str, u32 ssz, int err );
// Returns a memory handle to the full path and name or just the name of the executable.
// BCJBCJ BE CAREFULL! you probably want to vlAutoFree it as soon as you're done.
u32 vlSelfFullName( void );
u32 vlSelfName( void );
// Resource loader, it modifies arguments one and two to reflect the resource.  
// The third argument is the resource type. This returns
// 0 on failure and non-zero on success.
int vlLoadResource( u32 handle, void** mem, u32* sz, const u8* type );
// Returns a memory handle to the uncompressed version of mem, or 0.
// BCJBCJ BE CAREFULL! you probably want to vlAutoFree it as soon as you're done
u32 vlUncompress( const u8* mem, u32 sz );
// Determines whether a handle is attached or not (either to a consoel or file).
int vlIsHandleAttached( int );
// 1 if all this threads windows are idle (no messages).
int vlIsIdle( void );

// Prints to gui and console if they exist.  If neither of them exists, prints
// into a buffer that is automatically displayed at thread exit.
void vlPrint( const u8* str, u32 ssz, int err );
// This optionally swaps the byte positions in a u16 or u32, switch from little
// endian to big endian or vice-versa.
u16 vlSwab16( u16 val, int swab );
u32 vlSwab32( u32 val, int swab );



//////////////////////////////////////////////////////////////////////////////
// GUI enums
//////////////////////////////////////////////////////////////////////////////
typedef enum { app = 1, toolbox, borderless } vlWindowStyle;
typedef enum { text = 1, textNowrap, statusbar } vlWidgetStyle;
typedef enum { checked = 1, unchecked, disabled } vlMenuItemState;
typedef enum { save = 1 } vlFileDialogOptions;


#endif //VISLIB3_H