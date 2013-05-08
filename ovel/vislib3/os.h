#ifndef VISLIB_OS_H
#define VISLIB_OS_H

#ifdef WINDOWS

#include <windows.h>
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
#include <stdlib.h>
#include "types.h"
#include "vislib3.h"
#include <process.h>


typedef struct {
  ATOM editClass;
  ATOM ovelClass;
  WNDPROC oldTextCB;
} osGlobal;




// The maximum size of use input in a edit control
#define maxEditCharacters ( 64 * 1024 * 1024 )


// Maximum size allowable for a windows registry key.
#define maxWindowsRegKeySize 65535
// Chunk size that key is broken into.
#define maxWindowsRegValueSize 2048
// Chunk size for writing to the console.
#define consoleChunkSize 32767


typedef struct{
  int active;
  lua_State* ls;

  // This is the thread heap.
  HANDLE heap;
  // This is a spot to store some temp data, assume it is overwritten regularly. it will be auto
  // osFreed at cleanup.
  void* tbuf;
  // This is a spot to store error strings.  All functions are responsible for
  // vlAutoFreeing before re-using it (it should always be allocated).
  u32 pvlErrorBuf;

  /////////////////////////////////////////////////////////////////////////////
  // vislib private
  u32 pvlAllocCount;
  u32 pvlAllocBuffSize;
  void** pvlAllocs;
  u32* pvlFrees;
  u32 pvlNumFrees;
  u32* pvlAllocCounts;
  u32* pvlAllocBuffSizes;
  u32 pvlogName;
#ifdef VLDEBUG
  u32 pvlMallocCount;
#endif
  // For storing key strings
  u8* pvlKeyBuf;
  u8* pvlAppendIntBuf;
  u8* pvlAppendIntFub;

  /////////////////////////////////////////////////////////////////////////////
  // os base font
  f64 posBaseFontSize;
  f64 posBaseFontR;
  f64 posBaseFontG;
  f64 posBaseFontB;
  u32 posBaseFontFmt;
  const u8* posBaseFontFname;

  /////////////////////////////////////////////////////////////////////////////
  // os private
  u32 posIndex;
  u32 posCallbacks;
  u32 posWindows;
  u32 posWidgets;
  HACCEL posAccel;
  HANDLE posIcon;
  HFONT posFont;
  u32 posOsName;
  MSG* posCurrentMsg;
  u8 posKeys[ 256 ];


  /////////////////////////////////////////////////////////////////////////////
  // vl privates
  u32 pvlIdeThread;
  HANDLE pvlOutHandle;
  HANDLE pvlErrorHandle;
  u32 pvlOBuff;
  u32 pvlEBuff;
  u32 pvlReadPos;
  u32 pvlReadSize;
  vlGlobal* pvlGlobal;
} vlState;



#ifndef PROD
#define vlCheckState( s ) { if( s == NULL || !s->active ) vlDie( s, "Bad state!" ); }
#else
#define vlCheckState( s ) {}
#endif

vlGlobal* osGetGlobal( void );
void osReleaseGlobal( void );

vlState* osGetTLS( void );

int osLuaCall( lua_State* s, int args, int rets );

// Resource loader, it modifies arguments one and two to reflect the resource.  
// The third argument is the resource type. This returns
// 0 on failure and non-zero on success.
int osLoadResource( u32 handle, void** mem, u32* sz, const u8* type );

void osInitTLS( void );
void osInit( void );
void osCleanup( void );
void osCreateThread( void (*func)( void* ), void* v );
#define osGetThreadId() GetCurrentThreadId()
int osIsThreadRunning( u32 tid );

typedef struct{
  u32 bpp;
  u32 width;
  u32 height;
  u32 frequency;
  u32 display;
  u32 identifier;
} osDisplay;

s32 osVirtualDisplayWidth( void );
s32 osVirtualDisplayHeight( void );
s32 osDesktopX( void );
s32 osDesktopY( void );
s32 osDesktopHeight( void );
s32 osDesktopWidth( void );
const u8* osGetVersion( void );
// BCJBCJ BE CAREFUL! This returns a memory handle that you may want to vlAutoFree.
u32 osGetDisplays( void );
// BCJBCJ BE CAREFUL! This returns a memory handle that you may want to vlAutoFree.
u32 osFileDialog( u32 opts );

void osInform( const u8* msg, const u8* title );
void osInformError( const u8* msg, const u8* title );
// Prompts the user with a yes/no dialog, returns 0 for no and 1 for yes.
int osQuery( const u8* msg, const u8* title );


#define osDie( n ) { _endthreadex( n ); CloseHandle( GetCurrentThread() ); }
#define osDieError( msg ) { osInformError( msg, "Error" ); osDie( 0 ); }
void* osGlobalMalloc( size_t sz );
void* osGlobalRealloc( void* d, u32 sz, u32 osz );
#define osGlobalFree( data ) GlobalFree( data )
// These never fail, if allocation fails, it throws an exception.
#define osHeapMalloc( sz ) HeapAlloc( osGetTLS()->heap, 0, sz )
#define osHeapRealloc( data, sz ) HeapReAlloc( osGetTLS()->heap, 0, data, sz )
#define osHeapFree( data ) HeapFree( osGetTLS()->heap, 0, data )

// Gives back execution to the OS for a given amount of time.
#define osSleep( msec ) Sleep( msec )
// Returns the unix time (since jan 1 1970).
f64 osUnixTime( void );
// Returns the value of the "high performance counter".
u64 osClock( void );
// Returns the frequency of the "high performance counter".
u64 osClockFrequency( void );


// Returns NULL or a non-nul-terminated UTF-8 string.
// BCJBCJ if this returns non-null YOU MUST vlFree IT!!!
u32 osGetPasteBuffer( void );
void osSetPasteBuffer( const u8* v, u32 sz );

void osShowWindow( void* w, int show );
void osFocusWindow( void* w );
void* osCreateWindow( u32 x, u32 y, u32 width, u32 height, u8* title, vlWindowStyle style );
void osDestroyWindow( void* w );
void osGetPosWindow( const void* w, s32* x, s32* y, s32* width, s32* height, int client );
void osGetPosWidget( const void* w, s32* x, s32* y, s32* width, s32* height );
void osSetTitleWindow( void* w, const u8* t );
vlWindowStyle osGetStyleWindow( const void* win );
void osSelectTextWidget( void* w, u32 b, u32 e );
void osGetSelectionTextWidget( const void* w, u32* b, u32* e );
// BCJBCJ BE CAREFUL! You might want to free the memory handle returned by this.
u32 osGetSelectionTextTextWidget( const void* w );
typedef enum { replaceSelection = 1, replaceAll = 2 } osChangeType;
void osChangeFormatedTextWidget( void* w, f64 sz, f64 r, f64 g, f64 b, u32 fmt, const u8* fname, const u8* msg, u32 msz, f64 firstLineIndent, f64 leftIndent, f64 rightIndent, osChangeType ct, int bullet );
void osReplaceSelectionWithHRTextWidget( void* w, f64 sz, f64 r, f64 g, f64 b );
void osUndoTextWidget( void* w );
void osRedoTextWidget( void* w );
// Maps a font from a generic name to a system dependent font name, returns a
// freshly allocated string THAT MUST BE DEALLOCATED WITH vlFree!  Returns NULL
// if no compatible font is found.
u8* osGetFont( const u8* f );
void osSetPartsWidget( void* w, int* poss, u8** txts, u32 sz );
void osSetPartTextWidget( void* w, u32 part, const u8* txt );
void osRaiseWindow( void* w );
// Shouldnt need this, but leaving code just in case.
//void osSendMessage( void* w, u8* msg );
int osIsVisibleWindow( const void* w );
void osResizeWindow( void* w, s32 x, s32 y, s32 width, s32 height, int client );
// BCJBCJ BE CAREFUL! You must free the memory returned by this!
u8* osGetTextWindow( const void* w );
// BCJBCJ BE CAREFUL! You must free the memory returned by this!
u8* osGetTextWidget( const void* w );

// These get or set the modification status of a widget.
int osGetModifyWidget( const void* w );
void osSetModifyWidget( const void* w, int );

u32 osGetLineCountWidget( const void* w );
// Returns last line for an invalid character pos.
u32 osLineFromCharWidget( const void* w, u32 c );
// Returns -1 if it is an invalid line number.
u32 osCharFromLineWidget( const void* w, u32 c );

vlWidgetStyle osGetStyleWidget( const void* wid );
void osScrollWidget( void* w, f64 pcntg );
void osRelativeScrollWidget( void* w, s64 delta );
void osAddMenu( void* w, const u8* name, u32 id, u32 parent );
void osRemoveMenu( void* w, u32 id );
void osSetMenuItemState( void* w, u32 id, vlMenuItemState check );
void* osAddWidget( void* win, u32 x, u32 y, u32 width, u32 height, vlWidgetStyle style );
void osLoop( void );

// Returns 0 or a memory handle to the registry value.
// BCJBCJ BE CAREFULL! you probably want to vlAutoFree it as soon as you're done.
u32 osGetUserData( const u8* key );
// 1 is success, 0 is failure.
int osSetUserData( const u8* key, const u8* value, u32 sz );

// Returns 0 or a memory handle to the loaded file.
// BCJBCJ BE CAREFULL! you probably want to vlAutoFree it as soon as you're done.
u32 osLoadFile( const u8* name );
// Returns non-zero on success or 0 on falure.  The fourth paramter specifies whether to
// overwrite an exiting file, if it is zero then writing to an existing file fails.
int osSaveFile( const u8* name, const u8* data, u32 sz, int overwrite );
// Returns a memory handle to the current working directory or 0 if it can't get it.
// This does not include a terminating directory mark.
// BCJBCJ BE CAREFULL! you probably want to vlAutoFree it as soon as you're done.
u32 osGetCWD( void );
// changes the CWD.
int osSetCWD( const u8* nd );
// Returns a memory handle to the short version of the path.
// BCJBCJ BE CAREFULL! you probably want to vlAutoFree it as soon as you're done.
u32 osStripPath( const u8* name );
// Returns 0 or a memory handle to the long version of the path.  This can be used
// to test whether a file exists.
// BCJBCJ BE CAREFULL! you probably want to vlAutoFree it as soon as you're done.
u32 osGetFullPath( const u8* name );
// Prints to the console, if err is nonzero, it goes to stderr else to stdout.
void osConsolePrint( const u8* str, u32 ssz, int err );
// Returns a memory handle to the full path and name of the executable.
// BCJBCJ BE CAREFULL! you probably want to vlAutoFree it as soon as you're done.
u32 osSelfName( void );
int osIsIdle( void );

// This returns 1 if a window is valid, 0 otherwise.
int osWindowStatus( void* w );

#define osWaitMessage() WaitMessage()

#else //WINDOWS
#error Only windows supported right now, sorry :|
#endif //WINDOWS
#endif //VISLIB_OS_H