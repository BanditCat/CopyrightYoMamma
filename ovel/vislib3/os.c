#include <windows.h>
#include <richedit.h>
#include <commctrl.h>
#include <string.h>
#include <stdio.h>
#include "os.h"
#include "vislib3.h" 
#include "..\ovel\rc.h"
#include <process.h>
#include <math.h>



// BUGBUG BUGBUG this is for debugging
//#define vlDf( y ) void vlDie##y( const u8* msg ){ vlInform( #y, "vldf" ); vlDie( msg ); } 
//vlDf( 1 )
//vlDf( 2 )
//vlDf( 3 )
//vlDf( 4 )
//vlDf( 5 )
//vlDf( 6 )
//vlDf( 7 )
//vlDf( 8 )
//vlDf( 9 )
//vlDf( 19 )






// File operation size in bytes
#define osFOSize 4096


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ALERT! ALERT! ALERT! ALERT! ALERT! ALERT! ALERT! ALERT! ALERT! ALERT! ALERT! ALERT! ALERT! ALERT! ALERT! ALERT! 
// These should be the ONLY NON-CONST GLOBALS OR STATICS ANYWHERE!

// This is the thread local storage index that hold all per-thread state.
static u32 posTLS = TLS_OUT_OF_INDEXES;
// This is the sturct that holds all global data.
static vlGlobal posGlobal;
// This is the global state access critical section.
static CRITICAL_SECTION posGlobalCS;

// ALERT! ALERT! ALERT! ALERT! ALERT! ALERT! ALERT! ALERT! ALERT! ALERT! ALERT! ALERT! ALERT! ALERT! ALERT! ALERT! 
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////


struct posWindowDataStruct{
  HWND hwnd;
  u32 style;
  u32 styleEx;
  int menu;
};
typedef struct posWindowDataStruct posWindowData;

#define posHwnd( w ) ( ( (posWindowData*)(w->osdata) )->hwnd )


vlState* osGetTLS( void ){
  void* ans;
#ifdef VLDEBUG
  if( posTLS == TLS_OUT_OF_INDEXES ){
    osInformError( "Bad thread local storage!", "Error" );  
    osDie( 0 );
  }
#endif
  ans = TlsGetValue( posTLS );
#ifdef VLDEBUG
  if( ans == NULL ){
    osInformError( "NULL thread local storage!", "Error" );
    osDie( 0 );
  }
#endif
  return ans;
}
static int posErrFunc( lua_State* s ){
  vlState* vltls = osGetTLS();
  vlAppendString( vltls->pvlEBuff, lua_tostring( s, -1 ) );
  return 1;
}
int osLuaCall( lua_State* s, int args, int rets ){
  lua_checkstack( s, 4 );
  lua_pushcfunction( s, posErrFunc );
  lua_insert( s, 1 );
  lua_pcall( s, args, rets, 1 );  
  lua_remove( s, 1 );
  if( vlSize( osGetTLS()->pvlEBuff ) )
    return 0;
  else
    return 1;
}
vlGlobal* osGetGlobal( void ){ EnterCriticalSection( &posGlobalCS ); return &posGlobal; }
void osReleaseGlobal( void ){ LeaveCriticalSection( &posGlobalCS ); }
int osLoadResource( u32 handle, void** mem, u32* sz, const u8* type ){
  HGLOBAL hg;
  u32 tu = vlUTF8ToUTF16( type, strlen( type ) + 1, vlDie, 0, 1 );
  HRSRC rs = FindResourceW( NULL, MAKEINTRESOURCE( handle ), vlMem( tu ) ); 
  if( rs == NULL )
    return 0;
  *sz = SizeofResource( NULL, rs );
  hg = LoadResource( NULL, rs );
  if( hg == NULL )
    return 0;
  *mem = LockResource( hg );
  if( *mem == NULL )
    return 0;
  return 1;
}
static void* posAlloc( void* ud, void* ptr, size_t osize, size_t nsize ){
  if( !nsize ){
    if( osize )
      osGlobalFree( ptr );
    return NULL;
  }else
    return osGlobalRealloc( ptr, nsize, osize );
}
void osInitTLS( void ){
  vlState* vltls;
  if( posTLS == TLS_OUT_OF_INDEXES ){
    posTLS = TlsAlloc();
    if( posTLS == TLS_OUT_OF_INDEXES )
      osDieError( "Failed to create thread local storage!" );
    if( !InitializeCriticalSectionAndSpinCount( &posGlobalCS, 4096 ) )
      osDieError( "Failed to create critical section!" );
    {
      vlGlobal* vls = vlGetGlobal();
      vls->osGlobal = osGlobalMalloc( sizeof( osGlobal ) );
      vls->luaGlobal = lua_newstate( posAlloc, NULL );
      vls->numThreads = 0;
      ( (osGlobal*)( vls->osGlobal ) )->ovelClass = 0;
      ( (osGlobal*)( vls->osGlobal ) )->editClass = 0;
      ( (osGlobal*)( vls->osGlobal ) )->oldTextCB = NULL;
      vlReleaseGlobal();
    }
  }
  ++( vlGetGlobal()->numThreads ); vlReleaseGlobal();
  TlsSetValue( posTLS, osGlobalMalloc( sizeof( vlState ) ) );
  vltls = osGetTLS();
  vltls->heap = HeapCreate( HEAP_NO_SERIALIZE | HEAP_GENERATE_EXCEPTIONS, 0, 0 );

  vltls->posBaseFontSize = 10;
  vltls->posBaseFontR = 0;
  vltls->posBaseFontG = 0;
  vltls->posBaseFontB = 0;
  vltls->posBaseFontFmt = 0;
  vltls->posBaseFontFname = "monospace";


  vltls->posIndex = 0;
  vltls->posCallbacks = 0;
  vltls->posWindows = 0;
  vltls->posWidgets = 0;
  vltls->posAccel = NULL;
  vltls->posIcon = NULL;
  vltls->posFont = NULL;
  vltls->posOsName = 0;
  vltls->posCurrentMsg = NULL;
  vltls->pvlGlobal = NULL;
}
void osInit( void ){
  vlState* vltls = osGetTLS();
  vltls->posCallbacks = vlAutoMalloc( 0 );
  vltls->posWindows = vlAutoMalloc( 0 );
  vltls->posWidgets = vlAutoMalloc( 0 );
  vltls->posFont = CreateFont( (s32)-MulDiv( (int)vltls->posBaseFontSize, GetDeviceCaps( GetDC( NULL ), LOGPIXELSY ), 72 ), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_MODERN, NULL);
  if( vltls->posFont == NULL )
    vlDie( "Failed to create Windows font!" );
  vltls->posIcon = LoadImage( GetModuleHandle( NULL ), MAKEINTRESOURCE( BCJICO ), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_SHARED );
  if( vltls->posIcon == NULL )
    vlDie( "Failed to load icon!" );
  // Attempt to attach to the console if not redirected
  vltls->pvlErrorHandle = GetStdHandle( STD_ERROR_HANDLE );
  vltls->pvlOutHandle = GetStdHandle( STD_OUTPUT_HANDLE );
  AttachConsole( ATTACH_PARENT_PROCESS );
  if( vltls->pvlOutHandle != GetStdHandle( STD_OUTPUT_HANDLE ) )
    SetStdHandle( STD_OUTPUT_HANDLE, vltls->pvlOutHandle );
  if( vltls->pvlErrorHandle != GetStdHandle( STD_ERROR_HANDLE ) )
    SetStdHandle( STD_ERROR_HANDLE, vltls->pvlErrorHandle );
}
BOOL CALLBACK posCleanupCallback( HWND h, LPARAM lp ){
  ShowWindow( h, SW_HIDE );
  return TRUE;
}
void osCleanup( void ){
  vlState* vltls = osGetTLS();
  if( vltls->posAccel != NULL )
    DestroyAcceleratorTable( vltls->posAccel );
  vltls->active = 0;
  ReleaseCapture();
  EnumThreadWindows( GetCurrentThreadId(), posCleanupCallback, 0 );  
  UnregisterClassW( L"ovel", GetModuleHandle( NULL ) );
  UnregisterClassW( L"ovelText", GetModuleHandle( NULL ) );
#ifdef VLDEBUG
  if( !HeapValidate( vltls->heap, 0, NULL ) )
    osDieError( "Corrupt heap detected!" );
#endif 

  {
    vlGlobal* vlg = vlGetGlobal();
    if( vlg->numThreads == 1 ){
      DeleteCriticalSection( &posGlobalCS );
      TlsFree( posTLS ); 
      if( vlg->luaGlobal != NULL )
        lua_close( vlg->luaGlobal );
      osGlobalFree( vlg->osGlobal );
      FreeConsole();
      exit( 0 );
    } else
      --vlg->numThreads;
    vlReleaseGlobal();
  }

  DeleteObject( osGetTLS()->posFont );
  HeapDestroy( vltls->heap );
  osGlobalFree( osGetTLS() );
}
typedef struct{
  void (*func)( void* );
  void* v;
} posTFS;
static unsigned int _stdcall posThreadFunc( void* lp ){
  posTFS tp;
  tp = *( (posTFS*)lp );
  osGlobalFree( lp );
  tp.func( tp.v );
  return 0;
}
void osCreateThread( void (*func)( void* ), void* v ){
  u32 tid;
  posTFS* tp;
  tp = osGlobalMalloc( sizeof( posTFS ) ); 
  tp->func = func;
  tp->v = v;

  _beginthreadex( NULL, 0, posThreadFunc, tp, 0, &tid );
}
int osIsThreadRunning( u32 tid ){
  u32 ec;
  HANDLE th = OpenThread( THREAD_QUERY_INFORMATION, 0, tid );
  if( th == NULL )
    return 0;
  if( !GetExitCodeThread( OpenThread( THREAD_QUERY_INFORMATION, 0, tid ), (LPDWORD)&ec ) )
    osDieError( "GetExitCodeThread failed!" );
  CloseHandle( th );

  return ec == STILL_ACTIVE;
}

s32 osDesktopWidth( void ){ RECT ra; SystemParametersInfo( SPI_GETWORKAREA, 0,&ra, 0 ); return (s32)ra.right - (s32)ra.left; }
s32 osDesktopHeight( void ){ RECT ra; SystemParametersInfo( SPI_GETWORKAREA, 0,&ra, 0 ); return (s32)ra.bottom - (s32)ra.top; }
s32 osDesktopX( void ){ RECT ra; SystemParametersInfo( SPI_GETWORKAREA, 0,&ra, 0 ); return (s32)ra.left; }
s32 osDesktopY( void ){ RECT ra; SystemParametersInfo( SPI_GETWORKAREA, 0,&ra, 0 ); return (s32)ra.top; }
s32 osVirtualDisplayWidth( void ){ return (s32)GetSystemMetrics( SM_CXVIRTUALSCREEN ); }
s32 osVirtualDisplayHeight( void ){ return (s32)GetSystemMetrics( SM_CYVIRTUALSCREEN ); }
// Returns a contiguous block of memory that starts with a u32 with the number of displays followed by an 
// array of osDisplays with their information.
u32 osGetDisplays( void ){
  DISPLAY_DEVICE dd;
  osDisplay os = { 0 };
  u32 n = 0;
  u32 d = 0;
  u32 ans = vlAutoMalloc( 0 );
  vlAppend( ans, "\0\0\0\0", 4 );
  dd.cb = sizeof( DISPLAY_DEVICE );
  while( 1 ){
    if( !EnumDisplayDevicesW( NULL, d, &dd, 0 ) )
      break;
    ++d;
    if( dd.StateFlags & DISPLAY_DEVICE_ACTIVE ){
      DEVMODE dm;
      dm.dmSize = sizeof( DEVMODE );
      dm.dmDriverExtra = 0;
      if( !EnumDisplaySettingsW( dd.DeviceName, ENUM_CURRENT_SETTINGS, &dm ) )
        vlDie( "Failed to get display settings!" );
      os.bpp = dm.dmBitsPerPel;
      os.frequency = dm.dmDisplayFrequency;
      os.width = dm.dmPelsWidth;
      os.height = dm.dmPelsHeight;
      ++n;
      os.display = vlUTF16ToUTF8( (u8*)( dd.DeviceString ), vlUTF16Len( (u8*)dd.DeviceString ), 1, vlDie );
      os.identifier = vlUTF16ToUTF8( (u8*)( dd.DeviceName ), vlUTF16Len( (u8*)dd.DeviceName ), 1, vlDie );
      vlAppend( ans, &os, sizeof( osDisplay ) );
    }
  }
  *( (u32*)vlMem( ans ) ) = n;
  return ans;
}



const u8* osGetVersion( void ){
  vlState* vltls = osGetTLS();
  if( vltls->posOsName == 0 ){
    vltls->posOsName = vlAutoMalloc( 0 );
    vlAppendString( vltls->posOsName, "Windows v" );
    vlAppendInt( vltls->posOsName, LOBYTE(LOWORD(GetVersion())) );  
    vlAppendString( vltls->posOsName, "." );  
    vlAppendInt( vltls->posOsName, HIBYTE(LOWORD(GetVersion())) );  
  }
  return vlMem( vltls->posOsName );
}
u32 osFileDialog( vlFileDialogOptions opts ){
  u32 sts;
  OPENFILENAME ofn;
  ofn.lStructSize = sizeof( OPENFILENAME );
  ofn.hwndOwner = NULL;
  ofn.hInstance = 0;
  ofn.lpstrFilter = NULL;
  ofn.lpstrCustomFilter = NULL;
  ofn.nMaxCustFilter = 0;
  ofn.nFilterIndex = 1;
  ofn.lpstrFile = vlMalloc( 32767 );
  ofn.lpstrFile[ 0 ] = '\0';
  ofn.nMaxFile = 32760;
  ofn.lpstrFileTitle = NULL;  
  ofn.nMaxFileTitle = 0;
  ofn.lpstrInitialDir = NULL;
  ofn.lpstrTitle = NULL;
  ofn.FlagsEx = 0;
  ofn.lpTemplateName = NULL;
  ofn.lpfnHook = NULL;
  ofn.lCustData = 0;
  ofn.nFileExtension = 0;
  ofn.nFileOffset = 0;
  ofn.Flags = 0;
  ofn.pvReserved = NULL;
  ofn.dwReserved = 0;
  ofn.lpstrDefExt = NULL;

  if( opts && save )
    sts = GetSaveFileName( &ofn );
  else 
    sts = GetOpenFileName( &ofn );
  if( !sts ){
    vlFree( ofn.lpstrFile );
    return 0;
  }

  { 
    u32 ans = vlUTF16ToUTF8( (const u8*)ofn.lpstrFile, vlUTF16Len( (const u8*)ofn.lpstrFile ), 1, vlDie );
    vlFree( ofn.lpstrFile );
    return ans;
  }
}
void osInform( const u8* msg, const u8* title ){
  u32 tmsg = vlUTF8ToUTF16( msg, strlen( msg ) + 1, vlDie, 0, 1 );
  u32 ttitle = vlUTF8ToUTF16( title, strlen( title ) + 1, vlDie, 0, 1 );
  MessageBoxW( NULL, (LPCWSTR)vlMem( tmsg ), (LPCWSTR)vlMem( ttitle ), MB_ICONASTERISK | MB_TASKMODAL );
  vlAutoFree( tmsg );
  vlAutoFree( ttitle );
}
void osInformError( const u8* msg, const u8* title ){
  u32 tmsg = vlUTF8ToUTF16( msg, strlen( msg ) + 1, vlDie, 0, 1 );
  u32 ttitle = vlUTF8ToUTF16( title, strlen( title ) + 1, vlDie, 0, 1 );
  MessageBoxW( NULL, (LPCWSTR)vlMem( tmsg ), (LPCWSTR)vlMem( ttitle ), MB_ICONERROR | MB_TASKMODAL );
  vlAutoFree( tmsg );
  vlAutoFree( ttitle );
}
int osQuery( const u8* msg, const u8* title ){
  int ret;
  u32 tmsg = vlUTF8ToUTF16( msg, strlen( msg ) + 1, vlDie, 0, 1 );
  u32 ttitle = vlUTF8ToUTF16( title, strlen( title ) + 1, vlDie, 0, 1 );
  ret = MessageBoxW( NULL, (LPCWSTR)vlMem( tmsg ), (LPCWSTR)vlMem( ttitle ), MB_ICONASTERISK | MB_YESNO | MB_TASKMODAL );
  vlAutoFree( tmsg );
  vlAutoFree( ttitle );
  return ( ( ret == IDYES ) ? 1 : 0 );
}
void* osGlobalMalloc( size_t sz ){
  void* nb = GlobalAlloc( GMEM_FIXED, sz );
  if( nb == NULL )
    osDieError( "Unable to allocate memory!." );
  return nb;
}
void* osGlobalRealloc( void* d, u32 sz, u32 osz ){
  if( d == NULL )
    return osGlobalMalloc( sz );
  if( !sz ){
    osGlobalFree( d );
    return NULL;
  }
  {
    // BCJBCJ GlobalReAlloc was failing for no obvoius reason, so we don't use it here.
    void* ans = GlobalAlloc( GMEM_FIXED, sz ); 
    if( ans == NULL )
      osDieError( "GlobalReAlloc failed!" );
    memcpy( ans, d, ( osz > sz ) ? sz : osz );
    osGlobalFree( d );
    return ans;
  }
}
f64 osUnixTime( void ){
  u64 ans, ans2;
  SYSTEMTIME st, st2;
  FILETIME ft, ft2;
  st2.wYear = 1970;
  st2.wMonth = 1;
  st2.wDay = 1;
  st2.wDayOfWeek = 1;
  st2.wHour = st2.wMilliseconds = st2.wMinute = st2.wSecond = 0;
  GetSystemTime( &st );
  if( !SystemTimeToFileTime( &st, &ft ) )
    osDieError( "SystemTimeToFileTime failed!" );
  if( !SystemTimeToFileTime( &st2, &ft2 ) )
    osDieError( "SystemTimeToFileTime failed!" );
  ans = (u64)ft.dwLowDateTime + ( (u64)ft.dwHighDateTime << 32 );
  ans2 = (u64)ft2.dwLowDateTime + ( (u64)ft2.dwHighDateTime << 32 );
  
  return ( ( (f64)ans ) - ( (f64)ans2 ) ) / ( (f64)10000000 );
}
u64 osClock( void ){
  LARGE_INTEGER tli;
  if( !QueryPerformanceCounter( &tli ) )
    vlDie( "Failed to get high performance counter!" );
  return tli.QuadPart;
}
u64 osClockFrequency( void ){
  LARGE_INTEGER tli;
  if( !QueryPerformanceFrequency( &tli ) )
    vlDie( "Failed to get high performance counter frequency!" );
  return tli.QuadPart;
}
// BCJBCJ if this returns non 0 YOU MUST vlAutoFree IT!!!
u32 osGetPasteBuffer( void ){
  u32 ans;
  u8* tp;
  u32 sz;
  HANDLE hnd;
  if( !OpenClipboard( NULL ) )
    return 0;
  if( ( hnd = GetClipboardData( CF_UNICODETEXT ) ) == NULL ){
    CloseClipboard();
    return 0;
  }
  if( ( tp = GlobalLock( hnd ) ) == NULL ){
    CloseClipboard();
    return 0;
  }
  sz = GlobalSize( hnd ) - 2;
  ans = vlUTF16ToUTF8( tp, sz, 1, vlDie );
  GlobalUnlock( hnd );
  CloseClipboard();
  return ans;
}
void osSetPasteBuffer( const u8* v, u32 sz ){
  u8* tp = NULL;
  u32 vu;
  HANDLE hnd;
  vlAssess( v != NULL, "NULL pointer in osSetPasteBuffer!" );
  if( !OpenClipboard( NULL ) )
    return;
  if( !EmptyClipboard() ){
    CloseClipboard();
    return;
  }
  vu = vlUTF8ToUTF16( v, sz, vlDie, 0, 1 );
  vlAppend( vu, "\0\0", 2 );
  if( ( ( hnd = GlobalAlloc( GMEM_MOVEABLE, vlSize( vu ) ) ) == NULL ) || ( ( tp = GlobalLock( hnd ) ) == NULL ) ){
    if( hnd != NULL )
      GlobalFree( hnd );
    CloseClipboard();
    vlAutoFree( vu );
    vlDie( "Fatal memory error!" );
  }
  
  memcpy( tp, vlMem( vu ), vlSize( vu ) );
  GlobalUnlock( hnd );
  SetClipboardData( CF_UNICODETEXT, tp );
  vlAutoFree( vu );
  CloseClipboard();
}


void posMakeKeyFromWP( vlKey* vk, WPARAM wp, LPARAM lp ){
  vlState* vltls = osGetTLS();
  WORD ch;
  vk->code = (u8)wp;
  vk->modifiers = 0;
  if( !GetKeyboardState( vltls->posKeys ) )
    osDieError( "Failed to get keyboard state!" );
  if( vk->code != VK_BACK && ToAscii( vk->code, ( ( lp >> 16 ) & 127 ) + ( lp >> 31 ) * 256, vltls->posKeys, &ch, 0 ) == 1 )
    vk->chr = (u8)ch;
  else
    vk->chr = 0;
  if( GetAsyncKeyState( VK_LSHIFT ) & 0x8000 || GetAsyncKeyState( VK_RSHIFT ) & 0x8000 )
    vk->modifiers |= vlShift;
  if( GetAsyncKeyState( VK_LCONTROL ) & 0x8000 || GetAsyncKeyState( VK_RCONTROL ) & 0x8000 )
    vk->modifiers |= vlCtrl;
  if( GetAsyncKeyState( VK_LMENU ) & 0x8000 || GetAsyncKeyState( VK_RMENU ) & 0x8000 )
    vk->modifiers |= vlAlt;
  if( GetAsyncKeyState( VK_LWIN ) & 0x8000 || GetAsyncKeyState( VK_RWIN ) & 0x8000 )
    vk->modifiers |= vlSystem;
}
void posHandleKey( lua_State* ls, vlKey* vk ){
  lua_createtable( ls, 0, 6 );
  lua_pushboolean( ls, vk->modifiers & vlAlt ); lua_setfield( ls, -2, "alt" );
  lua_pushboolean( ls, vk->modifiers & vlShift ); lua_setfield( ls, -2, "shift" );
  lua_pushboolean( ls, vk->modifiers & vlCtrl ); lua_setfield( ls, -2, "control" );
  lua_pushboolean( ls, vk->modifiers & vlSystem ); lua_setfield( ls, -2, "system" );
  lua_pushnumber( ls, vk->code ); lua_setfield( ls, -2, "code" );
  if( vk->chr ){
    lua_pushlstring( ls, &vk->chr, 1 ); 
    lua_setfield( ls, -2, "char" );
  }
  lua_pushstring( ls, vlKeyString( vk ) ); lua_setfield( ls, -2, "name" );
}
void posHandleMouse( HWND h, lua_State* ls, UINT tp, WPARAM wp, LPARAM lp, int txt ){
  lua_createtable( ls, 0, 6 );
  lua_pushnumber( ls, LOWORD( lp ) ); lua_setfield( ls, -2, "x" );
  lua_pushnumber( ls, HIWORD( lp ) ); lua_setfield( ls, -2, "y" );
  switch( tp ){
    case WM_XBUTTONDBLCLK: 
      lua_pushstring( ls, ( HIWORD( wp ) == XBUTTON1 ) ? "extra1" : "extra2" ); lua_setfield( ls, -2, "button" ); goto dblclknext;
    case WM_RBUTTONDBLCLK: 
      lua_pushstring( ls, "right" ); lua_setfield( ls, -2, "button" ); goto dblclknext;
    case WM_MBUTTONDBLCLK: 
      lua_pushstring( ls, "middle" ); lua_setfield( ls, -2, "button" ); goto dblclknext;
    case WM_LBUTTONDBLCLK:
      lua_pushstring( ls, "left" ); lua_setfield( ls, -2, "button" ); goto dblclknext;
dblclknext:
      lua_pushstring( ls, "doubleclick" ); lua_setfield( ls, -2, "type" );
      break;
    case WM_XBUTTONDOWN: 
      lua_pushstring( ls, ( HIWORD( wp ) == XBUTTON1 ) ? "extra1" : "extra2" ); lua_setfield( ls, -2, "button" ); goto btndwnnext;
    case WM_RBUTTONDOWN: 
      lua_pushstring( ls, "right" ); lua_setfield( ls, -2, "button" ); goto btndwnnext;
    case WM_MBUTTONDOWN: 
      lua_pushstring( ls, "middle" ); lua_setfield( ls, -2, "button" ); goto btndwnnext;
    case WM_LBUTTONDOWN:
      lua_pushstring( ls, "left" ); lua_setfield( ls, -2, "button" ); goto btndwnnext;
btndwnnext:
      lua_pushstring( ls, "buttondown" ); lua_setfield( ls, -2, "type" );
      break;
    case WM_MOUSEWHEEL:
      lua_pushstring( ls, "mousewheel" );
      lua_setfield( ls, -2, "type" );
      lua_pushnumber( ls, GET_WHEEL_DELTA_WPARAM( wp ) / WHEEL_DELTA );
      lua_setfield( ls, -2, "delta" );
      break;
    case WM_XBUTTONUP:
      lua_pushstring( ls, ( HIWORD( wp ) == XBUTTON1 ) ? "extra1" : "extra2" ); lua_setfield( ls, -2, "button" ); goto btnupnext;
    case WM_RBUTTONUP:
      lua_pushstring( ls, "right" ); lua_setfield( ls, -2, "button" ); goto btnupnext;
    case WM_MBUTTONUP:
      lua_pushstring( ls, "middle" ); lua_setfield( ls, -2, "button" ); goto btnupnext;
    case WM_LBUTTONUP:
      lua_pushstring( ls, "left" ); lua_setfield( ls, -2, "button" ); goto btnupnext;
btnupnext:  
      lua_pushstring( ls, "buttonup" ); lua_setfield( ls, -2, "type" );
      break;
    default:
      lua_pushstring( ls, "move" ); lua_setfield( ls, -2, "type" ); break;
  }
  if( h && txt ){
    POINTL pl;
    pl. x = LOWORD( lp ); pl.y = HIWORD( lp );
    lua_pushnumber( ls, (lua_Number)( SendMessageW( h, EM_CHARFROMPOS, 0, (LPARAM)( &pl ) ) ) );
    lua_setfield( ls, -2, "characterPosition" );
  }
}
LRESULT CALLBACK posCallback( HWND h, UINT msg, WPARAM wp, LPARAM lp ){
  vlState* vltls = osGetTLS();
  if( vltls->active ){
    lua_State* vlls = vltls->ls;
    u32 ct = lua_gettop( vlls );
    // BCJBCJ be carefull not to smash the stack! I get strange errors if this is below 512
    if( !lua_checkstack( vlls, 1024 ) )
      vlDie( "Unable to grow lua stack!" );
    if( msg >= WM_MOUSEFIRST && msg <= WM_MOUSELAST ){
      lua_pushstring( vlls, "mouse" );
      posHandleMouse( h, vlls, msg, wp, lp, 0 );
    } else
      switch( msg ){
        case WM_QUIT:
        case WM_CLOSE:
          lua_pushstring( vlls, "close" );  
          break;
        case WM_KEYUP:
        case WM_SYSKEYUP:
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
          lua_pushstring( vlls, ( msg == WM_KEYUP || msg == WM_SYSKEYUP )?"keyup":"keydown" );
          { 
            vlKey vk;
            posMakeKeyFromWP( &vk, wp, lp );
            posHandleKey( vlls, &vk );
          }
          break;
        case WM_ACTIVATE:
          if( LOWORD( wp ) == WA_INACTIVE )
            if( HIWORD( wp ) )
              lua_pushstring( vlls, "minimized" );
            else
              lua_pushstring( vlls, "deactivate" );
          else
            if( HIWORD( wp ) )
              lua_pushstring( vlls, "unminimized" );
            else
              lua_pushstring( vlls, "activate" );
          break;
        case WM_SIZE:
          if( wp == SIZE_MAXIMIZED )
            lua_pushstring( vlls, "maximize" );
          else if( wp == SIZE_MINIMIZED )
            lua_pushstring( vlls, "minimize" );
          else
            lua_pushstring( vlls, "resize" );
          break;
        case WM_MOVE:
          lua_pushstring( vlls, "move" );
          break;
        case WM_COMMAND:
          if( HIWORD( wp ) <= 2 ){
            lua_pushstring( vlls, "menu" );
            lua_pushnumber( vlls, LOWORD( wp ) );
            break;
          }else
            return DefWindowProcW( h, msg, wp, lp );
        default:
          return DefWindowProcW( h, msg, wp, lp );
    }
    lua_getfield( vlls, LUA_REGISTRYINDEX, "windows" ); lua_pushlightuserdata( vlls, (void*)h ); lua_gettable( vlls, -2 ); lua_remove( vlls, -2 );
    if( lua_istable( vlls, -1 ) ){
      lua_getfield( vlls, -1, "callback" ); lua_getfield( vlls, -2, "window" );
      lua_remove( vlls, -3 );
      if( lua_isfunction( vlls, -2 ) && lua_isuserdata( vlls, -1 ) ){
        s32 n = (s32)lua_gettop( vlls ) - (s32)ct - 2;
        s32 i;
        for( i = 0; i < n; ++i )
          lua_pushvalue( vlls, -2 - n );
        if( !osLuaCall( vlls, 1 + n, 1 ) ){
          if( lua_checkstack( vlls, 5 ) ){
            lua_pushstring( vlls, "Error during callback:\n\n" );
            lua_insert( vlls, -2 );
            lua_concat( vlls, 2 );
            vlDie( lua_tostring( vlls, -1 ) );
          }else
            vlDie( "Error during callback!" );
        }
      } else
        vlDie( "Bad window or callback!" );
    }else
      return DefWindowProcW( h, msg, wp, lp );
    { 
      int proc = lua_toboolean( vlls, -1 );
      lua_settop( vlls, ct );
      if( proc )
        return DefWindowProcW( h, msg, wp, lp );
      return 0;
    }
  }
  return DefWindowProcW( h, msg, wp, lp );
}
LRESULT CALLBACK posTextCallback( HWND h, UINT msg, WPARAM wp, LPARAM lp ){
  vlState* vltls = osGetTLS();
  WNDPROC otcb;
  {
    vlGlobal* vls = vlGetGlobal();
    otcb = ( (osGlobal*)( vls->osGlobal ) )->oldTextCB;
    vlReleaseGlobal();
  }
  if( vltls->active ){
    int proc = 1;
    lua_State* vlls = vltls->ls;
    u32 ct = lua_gettop( vlls );
    // BCJBCJ be carefull not to smash the stack!
    if( !lua_checkstack( vlls, 256 ) )
      vlDie( "Unable to grow lua stack!" );
    if( msg >= WM_MOUSEFIRST && msg <= WM_MOUSELAST ){
      lua_pushstring( vlls, "mouse" );
      posHandleMouse( h, vlls, msg, wp, lp, 1 );
    } else
      switch( msg ){
case EN_MAXTEXT:
  vlInform( "Your text operation was truncated because it exceeded the maximum possible size of the control.", "Warning" );
  return otcb( h, msg, wp, lp );
case WM_SYSKEYUP:
case WM_SYSKEYDOWN:
case WM_KEYUP:
case WM_KEYDOWN:
  lua_pushstring( vlls, ( msg == WM_KEYUP || msg == WM_SYSKEYUP )?"keyup":"keydown" );
  { 
    vlKey vk;
    posMakeKeyFromWP( &vk, wp, lp );
    if( ( ( vk.modifiers & vlCtrl ) && vk.code != vlkInsert && vk.code != vlkLeft && vk.code != vlkRight && vk.code != vlkUp && vk.code != vlkPageup && vk.code != vlkPagedown && vk.code != vlkHome && vk.code != vlkEnd ) || 
      ( vk.modifiers & vlAlt ) || 
      ( ( vk.modifiers & vlShift ) && ( vk.code == vlkInsert || vk.code == vlkBackspace || vk.code == vlkDelete ) ) )
      proc = 0;
    posHandleKey( vlls, &vk );
    if( vk.code == vlkAlt )
      proc = 1;
  }
  break;
default:
  return otcb( h, msg, wp, lp );
    }

    lua_getfield( vlls, LUA_REGISTRYINDEX, "widgets" ); lua_pushlightuserdata( vlls, (void*)h ); lua_gettable( vlls, -2 ); lua_remove( vlls, -2 );
    if( lua_istable( vlls, -1 ) ){
      lua_getfield( vlls, -1, "callback" ); lua_getfield( vlls, -2, "widget" );
      lua_remove( vlls, -3 );
      if( lua_isfunction( vlls, -2 ) && lua_isuserdata( vlls, -1 ) ){
        s32 n = (s32)lua_gettop( vlls ) - (s32)ct - 2;
        s32 i;
        for( i = 0; i < n; ++i )
          lua_pushvalue( vlls, -2 - n );
        if( !osLuaCall( vlls, 1 + n, 1 ) ){
          if( lua_checkstack( vlls, 5 ) ){
            lua_pushstring( vlls, "Error during callback:\n\n" );
            lua_insert( vlls, -2 );
            lua_concat( vlls, 2 );
            vlDie( lua_tostring( vlls, -1 ) );
          }else
            vlDie( "Error during callback!" );
        }
      } else
        vlDie( "Bad widget or callback!" );
    } else
      vlDie( "Widget not found!" );
    { 
      proc = proc && lua_toboolean( vlls, -1 );
      lua_settop( vlls, ct );
      if( proc && vltls->posCurrentMsg != NULL ){
        MSG mp = *( vltls->posCurrentMsg );
        vltls->posCurrentMsg = NULL;
        TranslateMessage( &mp );
        return otcb( mp.hwnd, mp.message, mp.wParam, mp.lParam );
      }
      return 0;
    }
  }

  return otcb( h, msg, wp, lp );
}
void* osCreateWindow( u32 x, u32 y, u32 width, u32 height, u8* title, vlWindowStyle style ){
  ATOM oc;
  vlState* vltls = osGetTLS();
  u32 stl = WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_OVERLAPPEDWINDOW;
  u32 stlex = 0;
  switch( style ){
    default:
    case app:
      stlex = WS_EX_APPWINDOW;
      break;
    case toolbox:
      stlex = WS_EX_TOOLWINDOW;
      break;
    case borderless:
      stlex = WS_EX_APPWINDOW;
      stl = WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
      stl |= WS_POPUP;
      break;
  }
  {
    vlGlobal* vls = vlGetGlobal();
    oc = ( (osGlobal*)( vls->osGlobal ) )->ovelClass;
    vlReleaseGlobal();
  }
  if( oc == 0 ){
    WNDCLASSEXW wndcls = { 0 };
    wndcls.cbSize = sizeof( WNDCLASSEX );
    wndcls.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    wndcls.lpfnWndProc = posCallback;
    wndcls.cbClsExtra = 0;
    wndcls.cbWndExtra = 0;
    wndcls.hInstance = GetModuleHandle( NULL );
    wndcls.hIcon = vltls->posIcon;
    wndcls.hCursor = LoadCursor( NULL, IDC_ARROW );	
    wndcls.hbrBackground = (HBRUSH)( COLOR_BTNFACE + 1 );
    wndcls.lpszMenuName = NULL;
    wndcls.lpszClassName = L"ovel";
    wndcls.hIconSm = vltls->posIcon;
    oc = RegisterClassExW( &wndcls );
    if( !oc )
      osDieError( "Failed to register window class!" );
    {
      vlGlobal* vls = vlGetGlobal();
      ( (osGlobal*)( vls->osGlobal ) )->ovelClass = oc;
      vlReleaseGlobal();
    }
  }
  {
    u32 nm;
    HWND hw;
    if( title == NULL )
      nm = vlUTF8ToUTF16( "Unnamed", 8, vlDie, 0, 1 );
    else
      nm = vlUTF8ToUTF16( title, strlen( title ) + 1, vlDie, 0, 1 );

    hw = CreateWindowExW( stlex, L"ovel", vlMem( nm ), stl, x, y, width, height, NULL, NULL, GetModuleHandle( NULL ), NULL );

    if( hw == NULL )
      osDieError( "Failed to create window!" );
    vlAutoFree( nm );
    return hw;
  }
}
void osDestroyWindow( void* w ){ 
  DestroyWindow( (HWND)w ); 
}
void osGetPosWindow( void* w, s32* x, s32* y, s32* width, s32* height, int client ){
  WINDOWINFO wi;
  wi.cbSize = sizeof( WINDOWINFO );
  GetWindowInfo( (HWND)w, &wi );
  {
    RECT* ra = client?&wi.rcClient:&wi.rcWindow;
    *x = ra->left;
    *y = ra->top;
    *width = ra->right - ra->left;
    *height = ra->bottom - ra->top;
  }
}
void osGetPosWidget( void* w, s32* x, s32* y, s32* width, s32* height ){
  RECT r;
  s32 px, py, pw, ph;
  GetWindowRect( (HWND)w, &r );
  osGetPosWindow( GetAncestor( (HWND)w, GA_PARENT ), &px, &py, &pw, &ph, 1 );
  *x = r.left - px; *y = r.top - py; *width = r.right - r.left; *height = r.bottom - r.top;
}
void osSetTitleWindow( void* w, const u8* t ){
  u32 ts = vlUTF8ToUTF16( t, strlen( t ) + 1, vlDie, 0, 1 );
  SetWindowTextW( (HWND)w, vlMem( ts ) );
  vlAutoFree( ts );
}
void osSelectTextWidget( void* w, u32 b, u32 e ){
  SendMessageW( w, EM_SETSEL, b, e );
}
void osGetSelectionTextWidget( const void* w, u32* b, u32* e ){
  SendMessageW( (HWND)w, EM_GETSEL, (WPARAM)( (LPDWORD)b ), (LPARAM)( (LPDWORD)e ) );
  if( *b > *e ){
    u32 t = *b;
    *b = *e;
    *e = t;
  }
}
u32 osGetSelectionTextTextWidget( const void* w ){
  u32 b, e, sz, asz, ans;
  u8* buf;
  osGetSelectionTextWidget( w, &b, &e );
  sz = e - b;
  buf = vlMalloc( sz * 2 + 2 );
  asz = SendMessageW( (HWND)w, EM_GETSELTEXT, (WPARAM)( 0 ), (LPARAM)( buf ) );
  if( asz > sz )
    osDieError( "EM_GETSELTEXT caused a buffer overflow!" );
  ans = vlUTF16ToUTF8( buf, sz * 2, 1, vlDie );
  vlPopNul( ans );
  vlFree( buf );
  return ans;
}
void osUndoTextWidget( void* w ){
  SendMessageW( (HWND)w, EM_UNDO, (WPARAM)0, (LPARAM)0 );
}
void osRedoTextWidget( void* w ){
  SendMessageW( (HWND)w, EM_REDO, (WPARAM)0, (LPARAM)0 );
}
u8* osGetFont( const u8* f ){
  u8* ans;
  u32 fl = strlen( f );
  if( fl > 250  || !fl )
    return NULL;
  ans = vlMalloc( 255 );
  if( *f == 's' || *f == 'S' ){
    if( !strcmp( f + 1, "erif" ) )
      strcpy( ans, "Times New Roman" );
    else if( !strcmp( f + 1, "ans-serif" ) )
      strcpy( ans, "Arial" );
    else{
      vlFree( ans );
      return NULL;
    }
  } else if( ( *f == 'M' || *f == 'm' ) && !strcmp( f + 1, "onospace" ) )
    strcpy( ans, "Courier New" );
  else{
    vlFree( ans );
    return NULL;
  }
  return ans;
}
static void pos8toRTFHelper( u32 cp, u32 ans ){
  vlAppendString( ans, "\\u" );
  vlAppendInt( ans, cp );
}
void osChangeFormatedTextWidget( void* w, f64 sz, f64 r, f64 g, f64 b, u32 fmt, const u8* fname, const u8* msg, u32 msz, f64 firstLineIndent, f64 leftIndent, f64 rightIndent, osChangeType ct, int bullet ){
  int nl = 0;
  f64 rgb[ 3 ] = { 0 };
  const u8* rgbn[ 3 ] = { "\\red", "\\green", "\\blue" };
  u32 str = vlAutoMalloc( 0 );
  u32 i;
  u32 tstr = vlAutoMalloc( 0 );
  if( msz && ( msg[ msz - 1 ] == '\n' ) ){
    nl = 1;
    --msz;
  }
  vlUTF8Meta( msg, msz, tstr, pos8toRTFHelper, vlDie );
  vlAppendString( str, "{\\rtf1\\ansi\\deff0{\\fonttbl{\\f0\\fcharset0 " );
  {
    u8* ts = osGetFont( fname );
    if( ts == NULL )
      osDieError( "Bad font in osReplaceSelectionFormatedTextWidget." );
    vlAppendString( str, ts );
    vlFree( ts );
  }
  vlAppendString( str, ";}}{\\colortbl;" );
  rgb[ 0 ] = r; rgb[ 1 ] = g; rgb[ 2 ] = b;
  for( i = 0; i < 3; ++i ){
    s64 f = (u64)( floor( rgb[ i ] * 255.5 + 0.5 ) );
    vlAppendString( str, rgbn[ i ] );
    vlAppendInt( str, ( ( f > 255 ) ? 255 : ( ( f < 0 ) ? 0 : f ) ) );
  }
  vlAppendString( str, ";}" );
  if( fmt & vlUnderline ) vlAppendString( str, "\\ul" ); 
  if( fmt & vlBold ) vlAppendString( str, "\\b" );
  if( fmt & vlItalic ) vlAppendString( str, "\\i" );
  vlAppendString( str, "\\uc0\\cf1\\fs" ); vlAppendInt( str, (u32)( ( sz * (f64)osGetTLS()->posBaseFontSize ) * 2 ) );
  vlAppendString( str, " " );
  vlPopNul( str );
  vlAppend( str, vlMem( tstr ), vlSize( tstr ) );
  if( nl )
    vlAppendString( str, "\\par}" );
  else
    vlAppendString( str, "}" );
  
  {
    PARAFORMAT2 pf;
    u32 i;
    f64 ind[ 3 ]; 
    LONG *pfp[ 3 ];
    f64 mul = GetDeviceCaps( GetDC( NULL ), LOGPIXELSX );
    pf.cbSize = sizeof( PARAFORMAT2 );
    pf.dwMask = PFM_STARTINDENT + PFM_RIGHTINDENT + PFM_OFFSET + PFM_NUMBERING + PFM_NUMBERINGSTART + PFM_NUMBERINGSTYLE;
    pf.wNumberingStyle = 0x300;
    if( bullet ){
      if( bullet == -1 )
        pf.wNumbering = PFN_BULLET;
      else{
        pf.wNumbering = 2;
        pf.wNumberingStart = (WORD)bullet;
      }
    } else
      pf.wNumbering = 0;
    leftIndent = leftIndent - firstLineIndent;
    ind[ 0 ] = leftIndent; ind[ 1 ] = rightIndent; ind[ 2 ] = firstLineIndent; pfp[ 0 ] = &pf.dxOffset; pfp[ 1 ] = &pf.dxRightIndent; pfp[ 2 ] = &pf.dxStartIndent;
    for( i = 0; i < 3; ++i )
      *pfp[ i ] = (LONG)( ( ind[ i ] * 1440.0 * sz ) / mul );
    SendMessageW( (HWND)w, EM_SETPARAFORMAT, (WPARAM)0, (LPARAM)( &pf ) );
  }

  if( ct == replaceSelection )
    SendMessageW( (HWND)w, EM_REPLACESEL, (WPARAM)TRUE, (LPARAM)( (LPCTSTR)vlMem( str ) ) );
  else if( ct == replaceAll ){
    SETTEXTEX st;
    st.codepage = 1200;
    st.flags = 0;
    SendMessageW( (HWND)w, EM_SETTEXTEX, (WPARAM)( &st ), (LPARAM)( (LPCTSTR)vlMem( str ) ) );
  }

  vlAutoFree( str );
  vlAutoFree( tstr );
}
void osReplaceSelectionWithHRTextWidget( void* w, f64 sz, f64 r, f64 g, f64 b ){
  f64 rgb[ 3 ] = { 0 };
  const u8* rgbn[ 3 ] = { "\\red", "\\green", "\\blue" };
  u32 str = vlAutoMalloc( 0 );
  u32 i;
  vlAppendString( str, "{\\rtf1\\ansi\\deff0{\\fonttbl{\\f0\\fcharset0 Courier New;}}{\\colortbl;" );
  rgb[ 0 ] = r; rgb[ 1 ] = g; rgb[ 2 ] = b;
  for( i = 0; i < 3; ++i ){
    s64 f = (u64)( floor( rgb[ i ] * 255.5 + 0.5 ) );
    vlAppendString( str, rgbn[ i ] );
    vlAppendInt( str, ( ( f > 255 ) ? 255 : ( ( f < 0 ) ? 0 : f ) ) );
  }
  vlAppendString( str, ";}\\ulth\\cf1\\fs" );
  vlAppendInt( str, (u32)( ( sz * (f64)osGetTLS()->posBaseFontSize ) * 2 ) );
  vlAppendString( str, " \\par" );
  for( i = 1; i < 1024; ++i )
    vlAppendString( str, "\\emspace" );
  vlAppendString( str, " }" );

  SendMessageW( (HWND)w, EM_REPLACESEL, (WPARAM)TRUE, (LPARAM)( (LPCTSTR)vlMem( str ) ) );

  vlAutoFree( str );
}

void osSetPartsWidget( void* w, int* poss, u8** txts, u32 sz ){
  u32 i;
  if( sz > 250 ) 
    sz = 250;
  SendMessageW( (HWND)w, SB_SETPARTS, sz, (LPARAM)poss );
  for( i = 0; i < sz; ++i ){
    u32 tu = vlUTF8ToUTF16( txts[ i ], strlen( txts[ i ] ) + 1, vlDie, 0, 1 );
    SendMessageW( (HWND)w, SB_SETTEXT, (WPARAM)i, (LPARAM)vlMem( tu ) );
    vlAutoFree( tu );
  }
}
void osSetPartTextWidget( void* w, u32 part, const u8* txt ){
  u8* tp = NULL;
  u32 tu = 0;
  if( strlen( txt ) > 125 ){
    tp = vlMalloc( 126 );
    memcpy( tp, txt, 125 );
    tp[ 125 ] = '\0';
  } 
  tu = vlUTF8ToUTF16( ( tp == NULL ) ? txt : tp, ( tp == NULL ) ? ( strlen( txt ) + 1 ) : 126, vlDie, 0, 1 );
  if( tp != NULL )
    vlFree( tp );
  SendMessageW( (HWND)w, SB_SETTEXT, (WPARAM)part, (LPARAM)( vlMem( tu ) ) );
  vlAutoFree( tu );
}
// BCJBCJ BE CAREFUL! You must free the memory returned by this!
u8* osGetTextWindow( const void* w ){
  u8* tbuf;
  u32 sz = GetWindowTextLength( (HWND)w ) * 2;
  tbuf = vlMalloc( sz + 4 );
  GetWindowTextW( (HWND)w, (LPWSTR)tbuf, sz );

  {
    u32 tabuf = vlUTF16ToUTF8( tbuf, sz, 1, vlDie );
    vlFree( tbuf );
    tbuf = vlMalloc( vlSize( tabuf ) + 1 );
    memcpy( tbuf, vlMem( tabuf ), vlSize( tabuf ) );
    tbuf[ vlSize( tabuf ) ] = '\0';
    vlAutoFree( tabuf );
  }

  return tbuf;
}
// BCJBCJ BE CAREFUL! You must free the memory returned by this!
u8* osGetTextWidget( const void* w ){
  GETTEXTEX gt;
  u8* tbuf;
  u32 sz = GetWindowTextLength( (HWND)w );
  tbuf = vlMalloc( sz * 2 + 4 );
  gt.cb = sz * 2 + 4; gt.codepage = 1200; gt.flags = GT_DEFAULT; gt.lpDefaultChar = NULL; gt.lpUsedDefChar = NULL;
  sz = SendMessageW( (HWND)w, EM_GETTEXTEX, (WPARAM)( &gt ), (LPARAM)( tbuf ) );

  {
    u32 tabuf = vlUTF16ToUTF8( tbuf, sz * 2, 1, vlDie );
    vlFree( tbuf );
    tbuf = vlMalloc( vlSize( tabuf ) + 1 );
    memcpy( tbuf, vlMem( tabuf ), vlSize( tabuf ) );
    tbuf[ vlSize( tabuf ) ] = '\0';
    vlAutoFree( tabuf );
  }

  return tbuf;
}
int osGetModifyWidget( const void* w ){
  return SendMessageW( (HWND)w, EM_GETMODIFY, (WPARAM)( 0 ), (LPARAM)( 0 ) );
}
void osSetModifyWidget( const void* w, int m ){
  SendMessageW( (HWND)w, EM_SETMODIFY, m != 0, (LPARAM)( 0 ) );
}
u32 osGetLineCountWidget( const void* w ){
  return SendMessageW( (HWND)w, EM_GETLINECOUNT, (WPARAM)0, (LPARAM)0 );
}
u32 osLineFromCharWidget( const void* w, u32 c ){
  return 1 + SendMessageW( (HWND)w, EM_LINEFROMCHAR, (WPARAM)c, (LPARAM)0 );
}
u32 osCharFromLineWidget( const void* w, u32 c ){
  return SendMessageW( (HWND)w, EM_LINEINDEX, (WPARAM)( c - 1 ), (LPARAM)0 );
}
vlWidgetStyle osGetStyleWidget( const void* wid ){
  ATOM ec;
  WINDOWINFO wi;
  {
    vlGlobal* vls = vlGetGlobal();
    ec = ( (osGlobal*)( vls->osGlobal ) )->editClass;
    vlReleaseGlobal();
  }
  if( !GetWindowInfo( (HWND)wid, &wi ) )
    osDieError( "Failed to get window info in osGetStyleWidget." );
  if( wi.atomWindowType == ec ){
    if( wi.dwStyle & WS_HSCROLL ) 
      return text;
    else
      return textNowrap;
  }else
    return statusbar;
}
vlWindowStyle osGetStyleWindow( const void* win ){
  WINDOWINFO wi;
  if( !GetWindowInfo( (HWND)win, &wi ) )
    osDieError( "Failed to get window info in osGetStyleWindow." );
  if( wi.dwExStyle & WS_EX_APPWINDOW ){
    if( wi.dwStyle & WS_POPUP ) 
      return borderless;
    else
      return app;
  }else
    return toolbox;
}
void osScrollWidget( void* w, f64 pcntg ){
  ATOM ec;
  if( pcntg <= 0.0 )
    SendMessageW( (HWND)w, EM_SCROLL, SB_TOP, 0 );
  else if( pcntg >= 100.0 )
    SendMessageW( (HWND)w, EM_SCROLL, SB_BOTTOM, 0 );
  else{
    SCROLLINFO si;
    si.cbSize = sizeof( si );
    si.fMask = SIF_ALL;
    if( GetScrollInfo( (HWND)w, SB_VERT, &si ) ){
      f64 pm = pcntg / 100.0;
      u32 p;
      if( pm > 1 )
        pm = 1;
      if( pm < 0 )
        pm = 0;
      p = (u32)( (f64)( si.nMax - si.nMin ) * pm + (f64)si.nMin );
      SetScrollPos( (HWND)w, SB_VERT, p, TRUE );
      {
        vlGlobal* vls = vlGetGlobal();
        ec = ( (osGlobal*)( vls->osGlobal ) )->editClass;
        vlReleaseGlobal();
      }
      if( GetClassLong( (HWND)w, GCW_ATOM ) == ec ){
        POINT pt; 
        RECT r;
        GetClientRect( (HWND)w, &r );
        pt.x = 0; pt.y = ( (s32)p > r.bottom )?( p - r.bottom ):0;
        SendMessageW( (HWND)w, EM_SETSCROLLPOS, (WPARAM)0, (LPARAM)&pt );
      }
    }
  }
}
void osRelativeScrollWidget( void* w, s64 delta ){
  if( delta < 0 )
    while( delta ){
      SendMessageW( w, EM_SCROLL, (WPARAM)( SB_LINEUP ), (LPARAM)0 );
      ++delta;
    }
  else
    while( delta ){
      SendMessageW( w, EM_SCROLL, (WPARAM)( SB_LINEDOWN ), (LPARAM)0 );
      --delta;
    }
}
void osScrollBottomWidget( void* w ){
  SendMessageW( (HWND)w, EM_SCROLL, SB_BOTTOM, 0 );
}
int osIsVisibleWindow( const void* w ){
  return IsWindowVisible( (HWND)w );
}
void osResizeWindow( void* w, s32 x, s32 y, s32 width, s32 height, int client ){
  WINDOWINFO wi;
  RECT ra;
  ra.left = x; ra.top = y; ra.right = x + width; ra.bottom = y + height;  
  if( client ){
    wi.cbSize = sizeof( WINDOWINFO );
    GetWindowInfo( (HWND)w, &wi );
    AdjustWindowRectEx( &ra, wi.dwStyle, ( GetMenu( (HWND)w ) != NULL ), wi.dwExStyle );
  }
  SetWindowPos( (HWND)w, HWND_BOTTOM, ra.left, ra.top, ra.right - ra.left, ra.bottom - ra.top, SWP_NOACTIVATE | SWP_NOZORDER );
}
int posFindMenu( HMENU* ans, u32* aind, const HMENU mn, u32 id ){
  u32 sz = GetMenuItemCount( mn );
  u32 i;
  for( i = 0; i < sz; ++i ){
    u32 tid = GetMenuItemID( mn, i );
    if( tid == (u32)-1 ){
      MENUITEMINFO ni = { 0 };
      ni.cbSize = sizeof( MENUITEMINFO );
      ni.dwTypeData = NULL;
      ni.fMask = MIIM_ID | MIIM_SUBMENU;
      GetMenuItemInfo( mn, i, TRUE, &ni );
      tid = ni.wID;
    }
    if( tid == id ){
      *ans = mn;
      *aind = i;
      return 1;
    }else {
      HMENU thm = GetSubMenu( mn, i );
      if( thm != NULL )
        if( posFindMenu( ans, aind, thm, id ) )
          return 1;
    }
  }
  return 0;
}
void osAddMenu( void* w, const u8* name, u32 id, u32 parent ){
  HMENU mmd = GetMenu( (HWND)w );
  u32 nsz = strlen( name );
  if( mmd == NULL ){
    mmd = CreateMenu();
    if( mmd == NULL )
      vlDie( "CreateMenu failed!" );    
    if( !SetMenu( (HWND)w, mmd ) )
      vlDie( "SetMenu failed!" );
  }
  {
    HMENU thm;
    u32 ti;
    if( posFindMenu( &thm, &ti, mmd, parent ) ){
      HMENU sm = GetSubMenu( thm, ti );
      if( sm == NULL ){
        sm = CreateMenu();
        if( sm == NULL )
          vlDie( "CreateMenu failed!" );
        {
          MENUITEMINFO ni = { 0 };
          ni.cbSize = sizeof( MENUITEMINFO );
          ni.dwTypeData = NULL;
          ni.fMask = MIIM_SUBMENU;
          GetMenuItemInfo( thm, ti, TRUE, &ni );
          ni.fMask = MIIM_SUBMENU;
          ni.hSubMenu = sm;
          SetMenuItemInfo( thm, ti, TRUE, &ni );
        }
      }
      mmd = sm;
    }
  }
  {
    u32 nmu;
    MENUITEMINFOW ni = { 0 };
    ni.cbSize = sizeof( MENUITEMINFOW );
    ni.fState = MFS_ENABLED;
    ni.wID = id;
    ni.hSubMenu = 0;
    ni.fMask = MIIM_SUBMENU | MIIM_STRING | MIIM_STATE | MIIM_ID;

    nmu = vlUTF8ToUTF16( name, strlen( name ) + 1, vlDie, 0, 1 );
    if( nsz ){
      ni.dwTypeData = (LPWSTR)vlMem( nmu );
      ni.cch = nsz + 1;
    } else
      ni.fType = MFT_MENUBARBREAK;
    if( !InsertMenuItem( mmd, 0, TRUE, &ni ) )
      vlDie( "InsertMenuItem failed!" );
    vlAutoFree( nmu );
  }

  DrawMenuBar( (HWND)w );
}
void osSetMenuItemState( void* w, u32 id, vlMenuItemState check ){
  HMENU men;
  u32 ind;
  HMENU mmd = GetMenu( (HWND)w );
  if( mmd != NULL ){
    posFindMenu( &men, &ind, mmd, id );
    {
      MENUITEMINFO ni = { 0 };
      ni.cbSize = sizeof( MENUITEMINFO );
      ni.dwTypeData = NULL;
      ni.fMask = MIIM_STATE;
      GetMenuItemInfo( men, ind, TRUE, &ni );
      ni.fMask = MIIM_STATE;
      switch( check ){
case checked:
  ni.fState = MFS_CHECKED;
  break;
case unchecked:
  ni.fState = MFS_UNCHECKED;
  break;
case disabled:
  ni.fState = MFS_DISABLED;
  break;
      }
      SetMenuItemInfo( men, ind, TRUE, &ni );
      DrawMenuBar( (HWND)w );
    }
  }
}
void osRemoveMenu( void* w, u32 id ){
  HMENU men;
  u32 ind;
  HMENU mmd = GetMenu( (HWND)w );
  if( mmd != NULL && posFindMenu( &men, &ind, mmd, id ) ){
    DeleteMenu( men, ind, MF_BYPOSITION );
    DrawMenuBar( (HWND)w );
  }
}

void* osAddWidget( void* win, u32 x, u32 y, u32 width, u32 height, vlWidgetStyle style ){
  vlState* vltls = osGetTLS();
  if( style == text || style == textNowrap ){
    u32 stl;  
    u32 stlex = WS_EX_CLIENTEDGE;
    HWND hw;
    ATOM ec;
    if( style == text )
      stl = WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL | ES_DISABLENOSCROLL;
    else 
      stl = WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL | ES_DISABLENOSCROLL;

    {
      vlGlobal* vls = vlGetGlobal();
      ec = ( (osGlobal*)( vls->osGlobal ) )->editClass;
      vlReleaseGlobal();
    }
    if( ec == 0 ){
      WNDCLASSEXW wndcls;
      WNDPROC otcb;
      LoadLibraryW( L"Riched20.dll" );
      if( !GetClassInfoEx( NULL, RICHEDIT_CLASS, &wndcls ) )
        vlDie( "Failed to get edit class information." );
      wndcls.cbSize = sizeof( WNDCLASSEX );
      otcb = wndcls.lpfnWndProc;
      wndcls.lpfnWndProc = posTextCallback;
      wndcls.hInstance = GetModuleHandle( NULL );
      wndcls.lpszMenuName = NULL;
      wndcls.lpszClassName = L"ovelText";
      ec = RegisterClassEx( &wndcls );
      if( !ec )
        osDieError( "Failed to register edit widget class!" );
      {
        vlGlobal* vls = vlGetGlobal();
        ( (osGlobal*)( vls->osGlobal ) )->editClass = ec;
        ( (osGlobal*)( vls->osGlobal ) )->oldTextCB = otcb;
        vlReleaseGlobal();
      }
    }

    hw = CreateWindowExW( stlex, L"ovelText", NULL, stl, x, y, width, height, (HWND)win, NULL, GetModuleHandle( NULL ), NULL );
    if( !hw )
      return NULL;
    ShowWindow( hw, SW_SHOW );

    SendMessageW( hw, WM_SETFONT, (WPARAM)vltls->posFont, (LPARAM)TRUE );
    SendMessageW( hw, EM_LIMITTEXT, (WPARAM)maxEditCharacters, 0 );

    return hw;
  } else if( style == statusbar ){
    // LOL init common control sex...
    INITCOMMONCONTROLSEX ics;
    u32 stl = WS_CHILD | WS_VISIBLE;
    HWND hw;

    ics.dwSize = sizeof( INITCOMMONCONTROLSEX );
    ics.dwICC = ICC_BAR_CLASSES;
    if( !InitCommonControlsEx( &ics ) )
      osDieError( "Failed to initialize StatusBar common control!" );

    hw = CreateWindowEx( 0, STATUSCLASSNAMEW, NULL, stl, x, y, width, height, (HWND)win, NULL, GetModuleHandle( NULL ), NULL );
    if( !hw )
      return NULL;
    ShowWindow( hw, SW_SHOW );

    SendMessageW( hw, WM_SETFONT, (WPARAM)vltls->posFont, (LPARAM)TRUE );

    return hw;
  }
  return NULL;
}
void osShowWindow( void* w, int show ){ 
  if( show )
    ShowWindow( (HWND)w, SW_SHOW ); 
  else
    ShowWindow( (HWND)w, SW_HIDE );
}
void osFocusWindow( void* w ){ 
  SetFocus( w );
}
void osRaiseWindow( void* w ){
  SetWindowPos( (HWND)w, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE );
}
void osLoop( void ){  
  vlState* vltls = osGetTLS();
  MSG msg;
  while( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) ){
    vltls->posCurrentMsg = &msg;
    DispatchMessage( &msg );
  }
}
u32 osGetUserData( const u8* key ){
  u32 ku;
  u32 ans;
  HKEY prk;
  HKEY swrk;
  HKEY ork;
  HKEY vlk;
  vlAssess( key != NULL, "Null key in osGetUserData!" );
  ku = vlUTF8ToUTF16( key, strlen( key ) + 1, vlDie, 0, 1 );
  if( RegOpenCurrentUser( KEY_READ, &prk ) != ERROR_SUCCESS )
    vlDie( "Failed to open current user registry!" );
  if( RegOpenKeyExW( prk, L"software", 0, KEY_READ, &swrk ) != ERROR_SUCCESS ){
    vlAutoFree( ku );
    RegCloseKey( prk );
    vlDie( "Failed to open software registry!" );
  }
  if( RegOpenKeyExW( swrk, L"Ovel", 0, KEY_READ, &ork ) != ERROR_SUCCESS ){
    vlAutoFree( ku );
    RegCloseKey( prk );
    RegCloseKey( swrk );
    return 0;
  }
  if( RegOpenKeyEx( ork, vlMem( ku ), 0, KEY_READ, &vlk ) != ERROR_SUCCESS ){
    vlAutoFree( ku );
    RegCloseKey( prk );
    RegCloseKey( swrk );
    RegCloseKey( ork );
    return 0;
  }
  vlAutoFree( ku );
  {
    u32 i = 0;
    u32 k, ku;
    int isc = 1;
    ans = vlAutoMalloc( 0 );
    while( isc ){
      DWORD sans, ssans;
      DWORD tp;
      u8* tbuf;
      LONG res;
      k = vlAutoMalloc( 0 );
      vlAppendString( k, "v" );
      vlAppendInt( k, i );
      ku = vlUTF8ToUTF16( vlMem( k ), vlSize( k ), vlDie, 0, 1 );
      res = RegQueryValueExW( vlk, vlMem( ku ), NULL, &tp, NULL, &sans );
      if( res != ERROR_SUCCESS || tp != REG_BINARY ) 
        isc = 0; 
      tbuf = vlMalloc( sans );
      res = RegQueryValueExW( vlk, vlMem( ku ), NULL, &tp, tbuf, &ssans );
      if( res != ERROR_SUCCESS || sans != ssans )
        isc = 0; 
      if( isc )
        vlAppend( ans, tbuf, sans );
      vlFree( tbuf );
      vlAutoFree( k );
      vlAutoFree( ku );
      ++i;
    }
  }
  RegCloseKey( prk );
  RegCloseKey( swrk );
  RegCloseKey( ork );
  RegCloseKey( vlk );

  return ans;
}
int osSetUserData( const u8* key, const u8* value, u32 sz ){
  int suc = 1;
  u32 ku;
  HKEY prk;
  HKEY swrk;
  HKEY ork;
  HKEY vlk;
  vlAssess( key != NULL && value != NULL, "Null argument in osSetUserData!" );
  ku = vlUTF8ToUTF16( key, strlen( key ) + 1, vlDie, 0, 1 );
  if( sz > maxWindowsRegKeySize ){
    u32 emsg = vlAutoMalloc( 0 );
    vlAppendString( emsg, "An attempt was made to store a value larger than " );
    vlAppendInt( emsg, maxWindowsRegKeySize );
    vlAppendString( emsg, " bytes, which is the largest advisable size for a Windows registry value." );
    vlDie( vlMem( emsg ) );
  }
  if( RegOpenCurrentUser( KEY_WRITE, &prk ) != ERROR_SUCCESS )
    vlDie( "Failed to open current user registry!" );
  if( RegOpenKeyExW( prk, L"software", 0, KEY_WRITE, &swrk ) != ERROR_SUCCESS ){
    RegCloseKey( prk );
    vlAutoFree( ku );
    vlDie( "Failed to open software registry!" );
  }
  if( RegCreateKeyExW( swrk, L"Ovel", 0, NULL, 0, KEY_WRITE, NULL, &ork, NULL ) != ERROR_SUCCESS ){
    RegCloseKey( prk );
    RegCloseKey( swrk );
    vlAutoFree( ku );
    return 0;
  }
  if( RegCreateKeyExW( ork, vlMem( ku ), 0, NULL, 0, KEY_WRITE, NULL, &vlk, NULL ) != ERROR_SUCCESS ){
    RegCloseKey( prk );
    RegCloseKey( swrk );
    RegCloseKey( ork );
    vlAutoFree( ku );
    return 0;
  }
  vlAutoFree( ku );
  {
    int i = 0;
    u32 m = 0, mu = 0;  
    u32 c = 0;
    u32 w = 0;
    while( c < sz ){
      w = maxWindowsRegValueSize;
      if( sz - c < w )
        w = sz - c;
      m = vlAutoMalloc( 0 );
      vlAppendString( m, "v" );
      vlAppendInt( m, i );
      mu = vlUTF8ToUTF16( vlMem( m ), vlSize( m ), vlDie, 0, 1 );
      if( RegSetValueExW( vlk, vlMem( mu ), 0, REG_BINARY, value + c, w ) != ERROR_SUCCESS ){
        suc = 0;
        vlAutoFree( m );
        vlAutoFree( mu );
        break;
      }
      ++i;
      c += w;
      vlAutoFree( m );
      vlAutoFree( mu );
    }
    if( suc ){
      int isc = 1;
      while( isc ){
        m = vlAutoMalloc( 0 );
        vlAppendString( m, "v" );
        vlAppendInt( m, i );
        mu = vlUTF8ToUTF16( vlMem( m ), vlSize( m ), vlDie, 0, 1 );
        if( RegDeleteValueW( vlk, vlMem( mu ) ) != ERROR_SUCCESS )
          isc = 0;
        ++i;
        vlAutoFree( m );
        vlAutoFree( mu );
      }
    }
  }

  RegCloseKey( prk );
  RegCloseKey( swrk );
  RegCloseKey( ork );
  RegCloseKey( vlk );
  return suc;
}
u32 osLoadFile( const u8* name ){
  u32 ans = 0;
  u32 nu = vlUTF8ToUTF16( name, strlen( name ) + 1, vlDie, 0, 1 );
  HANDLE hn = CreateFileW( vlMem( nu ), GENERIC_READ, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL );
  vlAutoFree( nu );
  if( hn == INVALID_HANDLE_VALUE )
    return ans;
  {
    u8* buf = vlMalloc( osFOSize );
    DWORD rd;
    ans = vlAutoMalloc( 0 );
    while( ReadFile( hn, buf, osFOSize, &rd, NULL ) && rd )
      vlAppend( ans, buf, rd );
    vlFree( buf );
  }
  CloseHandle( hn );
  return ans;
}
int osSaveFile( const u8* name, const u8* data, u32 sz, int overwrite ){
  HANDLE hn;
  DWORD wt = 0;
  u32 nu = vlUTF8ToUTF16( name, strlen( name ) + 1, vlDie, 0, 1 );
  if( !overwrite && ( GetFileAttributesW( vlMem( nu ) ) != INVALID_FILE_ATTRIBUTES ) ){
    vlAutoFree( nu );
    return 0;
  }
  hn = CreateFileW( vlMem( nu ), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_FLAG_SEQUENTIAL_SCAN, NULL );
  vlAutoFree( nu );
  if( hn == INVALID_HANDLE_VALUE )
    return 0;
  WriteFile( hn, data, sz, &wt, NULL );
  CloseHandle( hn );
  return wt == sz;
}
u32 osStripPath( const u8* name ){
  u32 ans;
  const u8* p = name;
  const u8* op = NULL;
  while( p - 1 != NULL ){
    op = p;
    p = strpbrk( p, "\\" ) + 1;
  }
  ans = vlAutoMalloc( 0 );
  vlAppendString( ans, op );
  return ans;
}
u32 osGetCWD( void ){
  u8* buf = NULL;
  u32 ans;
  u32 l = GetCurrentDirectoryW( 0, NULL );
  if( !l )
    return 0;
  buf = vlMalloc( l * 2 + 2 );
  l = GetCurrentDirectoryW( l + 1, (LPWSTR)buf );
  if( !l ){
    vlFree( buf );
    return 0;
  }  
  ans = vlUTF16ToUTF8( buf, l * 2, 1, vlDie );
  vlFree( buf );
  return ans;
}
int osSetCWD( const u8* nd ){
  u32 u16;
  int ret;
  vlAssess( nd != NULL, "Null pointer in osSetCWD!" );
  u16 = vlUTF8ToUTF16( nd, strlen( nd ) + 1, vlDie, 0, 1 );  
  ret = SetCurrentDirectoryW( vlMem( u16 ) );
  vlAutoFree( u16 );
  return ret ? 1 : 0;
}
u32 osGetFullPath( const u8* name ){
  u32 ans = 0;
  u32 c;
  u32 nu = vlUTF8ToUTF16( name, strlen( name ) + 1, vlDie, 0, 1 );
  u32 l = vlSize( nu ) / 2;
  u8* buf = vlMalloc( l * 2 + 2 );
  c = GetFullPathNameW( vlMem( nu ), l + 1, (LPWSTR)buf, NULL );
  if( !c ){
    vlAutoFree( nu );
    vlFree( buf );
    return 0;
  }
  if( c > l ){
    vlFree( buf );
    l = c;
    buf = vlMalloc( l * 2 + 2 );
    c = GetFullPathNameW( vlMem( nu ), l + 1, (LPWSTR)buf, NULL );
    if( !c || c > l ){
      vlAutoFree( nu );
      vlFree( buf );
      return 0;
    }
  }
  if( GetFileAttributesW( (LPWSTR)buf ) == INVALID_FILE_ATTRIBUTES ){
    vlAutoFree( nu );
    vlFree( buf );
    return 0;
  }
  ans = vlUTF16ToUTF8( buf, l * 2, 1, vlDie );
  vlAutoFree( nu );
  vlFree( buf );
  return ans;
}
void osConsolePrint( const u8* str, u32 ssz, int err ){
  vlState* vltls = osGetTLS();
  DWORD wrt;
  HANDLE oh = err?vltls->pvlErrorHandle:vltls->pvlOutHandle;
  const u8* sp = str;
  u32 len = ssz;
  while( len ){
    u32 cnt = ( consoleChunkSize > len )?len:consoleChunkSize;
    if( !WriteFile( oh, sp, cnt, &wrt, NULL ) )
      break;
    len -= cnt;
    sp += cnt;
  }
  if( err )
    WriteFile( oh, "\n", 1, &wrt, NULL );
}
u32 osSelfName( void ){
  u8* buf;
  u32 sbuf = 16;
  DWORD sbsz;
  while( 1 ){
    buf = vlMalloc( sbuf * 2 + 2 );
    sbsz = GetModuleFileNameW( GetModuleHandle( NULL ), (LPWSTR)buf, sbuf );
    if( !sbsz ){
      vlFree( buf );
      osDieError( "GetModuleFileName failed!" );
    }
    if( sbsz < ( sbuf - 1 ) ){
      u32 ans = vlUTF16ToUTF8( buf, vlUTF16Len( buf ) + 2, 1, vlDie );
      vlFree( buf );
      return ans;
    }
    vlFree( buf );
    sbuf *= 2;
  }
}
int osIsIdle( void ){
  MSG msg;
  return ( PeekMessageW( &msg, NULL, 0, 0, PM_NOREMOVE ) ? 0 : 1 );
}
int osWindowStatus( void* w ){
  WINDOWINFO p;
  return GetWindowInfo( w, &p );
}
// A helper function to parse the command line.
u8** osGetCLArgs( const u8* cl, int* argcp ){
  u8** argvs;
  u8* argv;
  u32 len;
  u32 argc;
  u8 a;
  u32 i, j;

  int bq;
  int bt;
  int bs;

  len = strlen( cl );
  i = ( ( ( len + 2 ) / 2 ) + 1 ) * sizeof( void* );
  argvs = (u8**)osGlobalMalloc( i + ( len + 2 ) * sizeof( u8 ) );
  argv = (u8*)( ( (u8*)argvs ) + i );
  argc = 0;

  argvs[ 0 ] = argv;
  bq = 0;
  bt = 0;
  bs = 1;
  i = 0;
  j = 0;

  while( 0 != ( a = cl[i] ) ){
    if( bq ){
      if( a == '\"' ){
        bq = 0;
      }else {
        argv[ j ] = a;
        j++;
      }
    }else {
      switch( a ){
        case '\"':
          bq = 1;
          bt = 1;
          if( bs ){
            argvs[ argc ] = argv+j;
            argc++;
          }
          bs = 0;
          break;
        case ' ':
        case '\t':
        case '\n':
        case '\r':
          if( bt ){
            argv[ j ] = '\0';
            j++;
          }
          bt = 0;
          bs = 1;
          break;
        default:
          bt = 1;
          if( bs ){
            argvs[ argc ] = argv + j;
            argc++;
          }
          argv[ j ] = a;
          j++;
          bs = 0;
          break;
      }
    }
    i++;
  }
  argv[ j ] = '\0';
  argvs[ argc ] = NULL;

  *argcp = argc;
  return argvs;
}




// This just calls vlMain.
int WINAPI WinMain( HINSTANCE VISLIB___unused1, HINSTANCE VISLIB___unused2, LPSTR VISLIB___unused3, int VISLIB___unused4 ){ 
  int argc;
  char** argv;
  int ret;
  const u8* cls = (const u8*)GetCommandLineW();
  u32 cl8;
  vlInit();
  cl8 = vlUTF16ToUTF8( cls, vlUTF16Len( cls ) + 2, 1, vlDie );
  argv = osGetCLArgs( vlMem( cl8 ), &argc );
  ret = vlMain( argc, (const char**)argv ); 
  osGlobalFree( argv );
  vlQuit();
}
