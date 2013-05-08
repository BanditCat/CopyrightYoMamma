#include "os.h"
#include "vislib3.h"
#include <stdio.h>
#include <stdlib.h>

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
#include "zlib.h"


// Maximum possible string length for a key description DANGEROUS!!!
#define pvlMaxKeyString 250



#define pvlWindowType 1
#define pvlWindowTypeSize ( sizeof( u32 ) + sizeof( void* ) )
#define pvlWidgetType 2
#define pvlWidgetTypeSize ( sizeof( u32 ) + sizeof( void* ) )

#define pvlSetReg( s, id ) { lua_setfield( s, LUA_REGISTRYINDEX, "vlregdata" VISLIB_VERSION_STRING id ); }
#define pvlGetReg( s, id ) { lua_getfield( s, LUA_REGISTRYINDEX, "vlregdata" VISLIB_VERSION_STRING id ); }

#define pvlIsUserType( s, n, t ) ( lua_isuserdata( s, n ) && *( (u32*)lua_touserdata( s, n ) ) == pvl##t##Type )
#define pvlUserToVoidp( s, n ) ( *( (void**)( ( (u32*)lua_touserdata( s, n ) ) + 1 ) ) )

#define vlLuaAssess( s, t, msg ){ if( !( t ) ) luaL_error( s, "%s", msg ); }

struct pvlWrapperStruct{
  lua_CFunction cfunc;
  const u8* name;
};
typedef struct pvlWrapperStruct pvlWrapper;


#define pvlReadChunk 0x03FFFFFF
static const u8* pvlRead( lua_State* s, const void* data, size_t* size ){
  vlState* vltls = osGetTLS();
  u32 tp = vltls->pvlReadPos;
  if( vltls->pvlReadPos >= vltls->pvlReadSize )
    return NULL;
  if( vltls->pvlReadPos + pvlReadChunk < vltls->pvlReadSize )
    *size = pvlReadChunk;
  else
    *size = vltls->pvlReadSize - vltls->pvlReadPos;
  vltls->pvlReadPos += *size;
  return ( (const u8*)data ) + tp;
}


#include "luafuncs.h"


// Returns NULL on success or returns a error message that is valid until the next call to this function.
const u8* vlLoadString( const u8* src, const u8* srcName ){
  vlState* vltls = osGetTLS();
  lua_State* vlls = vltls->ls;
  int err;
  vlAssess( src != NULL && srcName != NULL, "NULL string pointer in vlLoadString!" );
  vlAssess( vlls != NULL, "Attempt to load a string with a NULL environment!" );
  vltls->pvlReadPos = 0;
  vltls->pvlReadSize = strlen( src );
  err = lua_load( vlls, pvlRead, (void*)src, srcName );
  if( err ){
    vlAutoFree( vltls->pvlErrorBuf );
    vltls->pvlErrorBuf = vlAutoMalloc( 0 );
    switch( err ){ 
      default:
        vlAppendString( vltls->pvlErrorBuf, "Error while loading string from " ); vlAppendString( vltls->pvlErrorBuf, srcName );
        break;
      case LUA_ERRSYNTAX:
        vlAppendString( vltls->pvlErrorBuf, "Syntax error while loading string from " ); vlAppendString( vltls->pvlErrorBuf, srcName );
        break;
      case LUA_ERRMEM:
        vlAppendString( vltls->pvlErrorBuf, "Memory allocation error while loading string from " ); vlAppendString( vltls->pvlErrorBuf, srcName );
        break;
    }
    if( lua_gettop( vlls ) >= 1 && lua_isstring( vlls, -1 ) ){
      vlAppendString( vltls->pvlErrorBuf, ":\n\n" );
      vlAppendString( vltls->pvlErrorBuf, lua_tostring( vlls, -1 ) );
    } else
      vlAppendString( vltls->pvlErrorBuf, "." );
    return vlMem( vltls->pvlErrorBuf );
  }
  return NULL;
}

void vlRunString( const u8* src, const u8* srcName ){
  vlState* vltls = osGetTLS();
  lua_State* vlls = vltls->ls;
  int err;
  vlAssess( src != NULL && srcName != NULL, "NULL string pointer in vlRunString!" );
  vlAssess( vlls != NULL, "Attempt to run a string with a NULL environment!" );
  vltls->pvlReadPos = 0;
  vltls->pvlReadSize = strlen( src );
  err = lua_load( vlls, pvlRead, (void*)src, srcName );
  if( err ){
    u8 srcnm[ 257 ];
    u8 emsg[ 513 ];
    u32 sz = strlen( srcName );
    if( sz >= 256 )
      sz = 255;
    memcpy( srcnm, srcName, sz );
    srcnm[ sz ] = '\0';
    switch( err ){    
      default:
        sprintf( emsg, "Error while loading string from %s.", srcnm );
        vlDie( emsg );
      case LUA_ERRSYNTAX:
        sprintf( emsg, "Syntax error while loading string from %s.", srcnm );
        vlDie( emsg );
      case LUA_ERRMEM:
        sprintf( emsg, "Memory allocation error while loading string from %s.", srcnm );
        vlDie( emsg );
    }
  }
  err = lua_pcall( vlls, 0, 0, 0 );
  if( err ){
    u8 srcnm[ 257 ];
    u8* emsg = NULL;
    u32 sz = strlen( srcName );
    if( sz >= 256 )
      sz = 255;
    memcpy( srcnm, srcName, sz );
    srcnm[ sz ] = '\0';
    if( err == LUA_ERRRUN ){
      if( lua_isstring( vlls, 1 ) ){
        const u8* lmsg = lua_tostring( vlls, 1 );
        sz = strlen( lmsg );
        emsg = vlMem( vlAutoMalloc( 769 + sz ) );
        sprintf( emsg, "While running string from %s the following lua runtime error occured:\n\n" "%s", srcnm, lmsg );
        vlDie( emsg );
      } else{
        emsg = vlMem( vlAutoMalloc( 517 ) );
        sprintf( emsg, "Unknown runtime error while running string from %s.", srcnm );
        vlDie( emsg );
      }
    }else if( err == LUA_ERRMEM ){
      emsg = vlMem( vlAutoMalloc( 517 ) );
      sprintf( emsg, "Memory allocation error while running string from %s.", srcnm );
      vlDie( emsg );
    }else{
      emsg = vlMem( vlAutoMalloc( 517 ) );
      sprintf( emsg, "Unknown error while running string from %s.", srcnm );
      vlDie( emsg );
    }
  }
}
void vlRunStringNonFatal( const u8* src, const u8* srcName ){
   const u8* ans;
   vlState* vltls = osGetTLS();
   lua_State* vlls = vltls->ls;
   ans = vlLoadString( src, srcName );
   if( ans != NULL )
     vlAppendString( vltls->pvlEBuff, ans );
   else
     osLuaCall( vlls, 0, LUA_MULTRET );
}
static void* pvlAlloc( void* ud, void* ptr, size_t osize, size_t nsize ){
  if( !nsize ){
    if( osize )
      vlFree( ptr );
    return NULL;
  }else
    return vlRealloc( ptr, nsize );
}

void pvlLuaInit( void ){
  u32 i;
  vlState* vltls = osGetTLS();
  lua_State* vlls = lua_newstate( pvlAlloc, NULL );
  vltls->ls = vlls;

  vltls->pvlOBuff = vlAutoMalloc( 0 );
  //vlAppendString( vltls->pvlOBuff, "" );
  vltls->pvlEBuff = vlAutoMalloc( 0 );
  //vlAppendString( vltls->pvlEBuff, "" );
  vltls->pvlGlobal = NULL;
  vltls->pvlIdeThread = 0;


  lua_pushcfunction( vlls, luaopen_base ); lua_pushstring( vlls, "" ); lua_call( vlls, 1, 0 );
  lua_pushcfunction( vlls, luaopen_string ); lua_pushstring( vlls, "string" ); lua_call( vlls, 1, 0 );
  lua_pushcfunction( vlls, luaopen_math ); lua_pushstring( vlls, "math" ); lua_call( vlls, 1, 0 );
  lua_pushcfunction( vlls, luaopen_table ); lua_pushstring( vlls, "table" ); lua_call( vlls, 1, 0 );
  lua_pushcfunction( vlls, luaopen_package ); lua_pushstring( vlls, "package" ); lua_call( vlls, 1, 0 );
  lua_pushcfunction( vlls, luaopen_debug ); lua_pushstring( vlls, "debug" ); lua_call( vlls, 1, 0 );

  lua_newtable( vlls );
  for( i = 0; i < sizeof( pvlvlFuncs ) / sizeof( pvlvlFuncs[ 0 ] ); ++i ){
    lua_pushstring( vlls, pvlvlFuncs[ i ].name );
    lua_pushcfunction( vlls, pvlvlFuncs[ i ].cfunc );
    lua_settable( vlls, 1 );
  }
  lua_setglobal( vlls, "vl" );

  lua_newtable( vlls );
  for( i = 0; i < sizeof( pvlWindowFuncs ) / sizeof( pvlWindowFuncs[ 0 ] ); ++i ){
    lua_pushstring( vlls, pvlWindowFuncs[ i ].name );
    lua_pushcfunction( vlls, pvlWindowFuncs[ i ].cfunc );
    lua_settable( vlls, 1 );
  }
  lua_setglobal( vlls, "window" );

  lua_newtable( vlls );
  for( i = 0; i < sizeof( pvlWidgetFuncs ) / sizeof( pvlWidgetFuncs[ 0 ] ); ++i ){
    lua_pushstring( vlls, pvlWidgetFuncs[ i ].name );
    lua_pushcfunction( vlls, pvlWidgetFuncs[ i ].cfunc );
    lua_settable( vlls, 1 );
  }
  lua_setglobal( vlls, "widget" );

  lua_newtable( vlls );
  lua_setfield( vlls, LUA_REGISTRYINDEX, "windows" );

  lua_newtable( vlls );
  lua_setfield( vlls, LUA_REGISTRYINDEX, "widgets" );

  // Load prelude if present.
  {
    lua_State* vlgl = vlGetGlobal()->luaGlobal;
    lua_checkstack( vlgl, 5 );
    lua_getfield( vlgl, LUA_REGISTRYINDEX, "vlglobals" );
    if( lua_isnil( vlgl, -1 ) ){
      lua_pop( vlgl, 1 );
      vlReleaseGlobal();
      return;
    }
    lua_pushstring( vlgl, "prelude" ); 
    lua_gettable( vlgl, -2 );
    if( lua_isstring( vlgl, -1 ) )
      lua_pushstring( vlls, lua_tostring( vlgl, -1 ) );
    lua_pop( vlgl, 2 );
    vlReleaseGlobal();
  }
  // Run it.
  if( lua_gettop( vlls ) == 1 && lua_isstring( vlls, 1 ) ){
    int ret;
    const u8* src = lua_tostring( vlls, 1 );
    vltls->pvlReadPos = 0; vltls->pvlReadSize = strlen( src );
    if( 0 != ( ret = lua_load( vlls, pvlRead, (void*)src, "<prelude>" ) ) ){
      if( ret == LUA_ERRMEM )
        vlDie( "Memory error while loading prelude!" );
      else if( ret == LUA_ERRSYNTAX ){
        u32 em = vlAutoMalloc( 0 );
        vlAppendString( em, "Syntax error while loading prelude:\n" );
        vlAppendString( em, lua_tostring( vlls, -1 ) );
        vlInformError( vlMem( em ), "Error while loading prelude!" );
        vlAutoFree( em );
        vlQuit();
      } else
        vlDie( "Unknown error while loading prelude!" );
    }
    osLuaCall( vlls, 0, 0 );
    if( vlSize( vltls->pvlEBuff ) ){
      u32 tb = vlAutoMalloc( 0 );
      vlAppend( tb, vlMem( vltls->pvlEBuff ), vlSize( vltls->pvlEBuff ) );
      vlAppendString( tb, "" );
      osInformError( vlMem( tb ), "Error while running prelude!"  );
      vlAutoFree( tb );
      vlQuit();
    }
    lua_settop( vlls, 0 );
  }
}
void pvlLuaCleanup( void ){
  vlState* vltls = osGetTLS();
  lua_State* vlls = vltls->ls;
  if( vlls != NULL ){
    lua_settop( vlls, 0 );
    // Load the finale if present.
    {
      lua_State* vlgl = vlGetGlobal()->luaGlobal;
      lua_checkstack( vlgl, 3 );
      lua_getfield( vlgl, LUA_REGISTRYINDEX, "vlglobals" );
      if( lua_isnil( vlgl, -1 ) ){
        lua_pop( vlgl, 1 );
        vlReleaseGlobal();
        return;
      }
      lua_pushstring( vlgl, "finale" ); 
      lua_gettable( vlgl, -2 );
      if( lua_isstring( vlgl, -1 ) )
        lua_pushstring( vlls, lua_tostring( vlgl, -1 ) );
      lua_pop( vlgl, 2 );
      vlReleaseGlobal();
    }
    // Run it.
    if( lua_gettop( vlls ) == 1 && lua_isstring( vlls, 1 ) ){
      const u8* src = lua_tostring( vlls, 1 );
      vltls->pvlReadPos = 0; vltls->pvlReadSize = strlen( src );
      if( lua_load( vlls, pvlRead, (void*)src, "<finale>" ) )
        osDieError( "Error while loading finale!" );
      {
        u32 ol = vlSize( vltls->pvlEBuff );
        osLuaCall( vlls, 0, 0 );
        if( ol != vlSize( vltls->pvlEBuff ) ){
          u32 tb = vlAutoMalloc( 0 );
          vlAppend( tb, vlMem( vltls->pvlEBuff ), vlSize( vltls->pvlEBuff ) / 2 );
          vlAppendString( tb, "" );
          osInformError( vlMem( tb ), "Error while running finale!"  );
          vlAutoFree( tb );
          osDie( 0 );
        }
        lua_settop( vlls, 0 );
      }
    }   

    lua_close( vlls );
  }
}

static void pvlCleanup( void ){
  vlState* vltls = osGetTLS();
  u32 i;
  pvlLuaCleanup();
  vltls->active = 0;
  if( vltls->pvlAllocs != NULL ){
    for( i = 0; i < vltls->pvlAllocCount; ++i )
      if( vltls->pvlAllocs[ i ] != NULL )
        vlFree( vltls->pvlAllocs[ i ] );
    vlFree( vltls->pvlAllocs );
    vltls->pvlAllocs = NULL;
    vltls->pvlAllocCount = 0;
    vltls->pvlAllocBuffSize = 0;
    if( vltls->pvlAllocCounts != NULL )
      vlFree( vltls->pvlAllocCounts );
    vltls->pvlAllocCounts = NULL;
    if( vltls->pvlAllocBuffSizes != NULL )
      vlFree( vltls->pvlAllocBuffSizes );
    if( vltls->pvlFrees != NULL )
      vlFree( vltls->pvlFrees );
    vltls->pvlAllocBuffSizes = NULL;
  }
#ifdef VLDEBUG
  if( vltls->pvlMallocCount ){
    if( vltls->tbuf != NULL )
      osGlobalFree( vltls->tbuf );
    vltls->tbuf = osGlobalMalloc( 1024 );
    sprintf( vltls->tbuf, "Count: %i", (int)vltls->pvlMallocCount );
    vlInform( vltls->tbuf, "Memory Allocation Count" );
  }
#endif
  if( vltls->tbuf != NULL ){
    osGlobalFree( vltls->tbuf );
    vltls->tbuf = NULL;
  }
  osCleanup();
}
const u8* vlKeyString( const vlKey* key ){
  u8* buf = ( osGetTLS() )->pvlKeyBuf;
  u32 pos = 0;
  static const u8* const shift = "Shift+";
  static const u32 shiftLen = 6;
  static const u8* const ctrl = "Ctrl+";
  static const u32 ctrlLen = 5;
  static const u8* const alt = "Alt+";
  static const u32 altLen = 4;
  static const u8* const sys = "Sys+";
  static const u32 sysLen = 4;
  if( key->modifiers & vlCtrl ){
    memcpy( buf + pos, ctrl, ctrlLen );
    pos += ctrlLen;
  }
  if( key->modifiers & vlShift ){
    memcpy( buf + pos, shift, shiftLen );
    pos += shiftLen;
  }
  if( key->modifiers & vlAlt ){
    memcpy( buf + pos, alt, altLen );
    pos += altLen;
  }
  if( key->modifiers & vlSystem ){
    memcpy( buf + pos, sys, sysLen );
    pos += sysLen;
  }
  {
    u32 knl = strlen( vlKeyNames[ key->code ] );
    memcpy( buf + pos, vlKeyNames[ key->code ], knl );
    pos += knl;
  }
  buf[ pos ] = '\0';
  return buf;
}
int vlStringKey( const u8* name, vlKey* key ){
  u32 mods = 0;
  const u8* pos = name;
  u32 i;
  vlAssess( name != NULL, "NULL name in vlStringKey!" );
  while( strchr( pos, '+' ) != NULL ){
    if( strstr( pos, "Shift+" ) == pos ){
      pos += strlen( "Shift+" );
      mods |= vlShift;
      continue;
    } else if( strstr( pos, "Ctrl+" ) == pos ){
      pos += strlen( "Ctrl+" );
      mods |= vlCtrl;
      continue;
    } else if( strstr( pos, "Alt+" ) == pos ){
      pos += strlen( "Alt+" );
      mods |= vlAlt;
      continue;
    } else if( strstr( pos, "Sys+" ) == pos ){
      pos += strlen( "Sys+" );
      mods |= vlSystem;
      continue;
    } else
      return 0;
  }
  for( i = 0; i < 256; ++i ){
    if( !strcmp( pos, vlKeyNames[ i ] ) ){
      key->modifiers = (u8)mods;
      key->code = (u8)i;
      return 1;
    }
  }
  return 0;
}
vlGlobal* vlGetGlobal( void ){ return osGetGlobal(); }
void vlReleaseGlobal( void ){ osReleaseGlobal(); }
void vlInit( void ){
  vlState* vltls;
  osInitTLS();
  vltls = osGetTLS();
  vltls->active = 1;

#ifdef VLDEBUG
  vltls->pvlMallocCount = 0;
#endif

  vltls->tbuf = NULL;
  vltls->pvlogName = 0;
  vltls->pvlAllocs = vlMalloc( sizeof( void* ) );
  vltls->pvlAllocCounts = vlMalloc( sizeof( u32 ) );
  vltls->pvlAllocBuffSizes = vlMalloc( sizeof( u32 ) );
  vltls->pvlFrees = vlMalloc( sizeof( u32 ) );
  vltls->pvlNumFrees = 0;
  vltls->pvlAllocCount = 0;
  vltls->pvlAllocBuffSize = 1;

  // Allocate "static" buffers
  vltls->pvlKeyBuf = vlMem( vlAutoMalloc( pvlMaxKeyString ) ); 
  vltls->pvlAppendIntBuf = vlMem( vlAutoMalloc( 256 ) );
  vltls->pvlAppendIntFub = vlMem( vlAutoMalloc( 256 ) );
  vltls->pvlErrorBuf = vlAutoMalloc( 0 );

  osInit();
  pvlLuaInit();
}
void vlQuit( void ){ pvlCleanup(); osDie( 0 ); }
void vlDie( const u8* msg ){
  vlState* vltls = osGetTLS();
  if( vltls->active ){
    vltls->active = 0;
    vlAppendString( vltls->pvlEBuff, "\n" );
    vlAppendString( vltls->pvlEBuff, msg );
    osInformError( msg, "Error" );
    pvlCleanup();
    osDie( 0 ); 
  }
}
void vlWaitMessage( void ){ osWaitMessage(); }
u32 vlGetThreadId( void ){ return osGetThreadId(); }
void vlInform( const u8* msg, const u8* title ){ osInform( msg, title ); }
int vlQuery( const u8* msg, const u8* title ){ return osQuery( msg, title ); }
void vlInformError( const u8* msg, const u8* title ){ osInformError( msg, title ); }

///////////////////////////////////////////////////////////////////////////////////////////////////
// UTF functions
u32 vlUTF32ToUTF16( u32 cp ){
  if( cp <= 0x0000FFFF )
    return cp;  
  else
    return ( ( ( ( cp - 0x10000 ) >> 10 ) + 0xD800 ) << 16 ) + ( cp & 0x03FF ) + 0xDC00;
}
u32 vlUTF32ToUTF8( u32 cp ){
  if( cp <= 0x7F )
    return cp;
  else if( cp <= 0x7FF )
    return ( ( 192 + ( ( cp >> 6 ) & 0x1F ) ) << 8 ) + 128 + ( cp & 0x3F );
  else if( cp <= 0x0FFFF )
    return ( ( 224 + ( ( cp >> 12 ) & 0xF ) ) << 16 ) + ( ( 128 + ( ( cp >> 6 ) & 0x3F ) ) << 8 ) + 128 + ( cp & 0x3f );
  else
    return ( ( 240 + ( ( cp >> 18 ) & 0x7 ) ) << 24 ) + ( ( 128 + ( ( cp >> 12 ) & 0x3F ) ) << 16 ) + ( ( 128 + ( ( cp >> 6 ) & 0x3F ) ) << 8 ) + 128 + ( cp & 0x3f );
}
// optionally swaps byte order in a u16 or u32
u16 vlSwab16( u16 val, int swab ){
  if( swab )
    return ( val >> 8 ) + ( ( val & 0xFF ) << 8 );
  else 
    return val;
}
u32 vlSwab32( u32 val, int swab ){
  if( swab )
    return ( val >> 24 ) + ( ( val & 0x000000FF ) << 24 ) + ( ( ( val & 0x0000FF00 ) >> 8 ) << 16 ) + ( ( ( val & 0x00FF0000 ) >> 16 ) << 8 );
  else 
    return val;
}
void vlUTF16Meta( const u8* src, u32 sz, u32 ans, int defaultToLittleEndian, void (*func)( u32, u32 ), void (*errfunc)( const u8* estr ) ){
  u16* s16 = (u16*)src;
  u32 sz16 = sz / 2;
  int swab = !defaultToLittleEndian;
  u32 pos = 0;
  u32 cp = 0;
  if( sz % 2 )
    errfunc( "vlUTF16Meta called with a odd number of bytes." );
  if( s16[ 0 ] == 0xFEFF || s16[ 0 ] == 0xFFFE ){
    if( s16[ 0 ] == 0xFFFE )
      swab = 1;
    else
      swab = 0;
    pos += 1;
  }
  while( pos < sz16 ){
    int err = 0;
    if( vlSwab16( s16[ pos ], swab ) >= 0xD800 && vlSwab16( s16[ pos ], swab ) <= 0xDFFF ){
      if( vlSwab16( s16[ pos ], swab ) >= 0xDC00 ){
        err = 1;
        errfunc( "Invalid first word in surrogate pair in vlUTF16Meta greater than 0xDBFF." );
      }else if( pos + 1 >= sz16 ){
        err = 1;
        errfunc( "Invalid first word in surrogate pair in vlUTF16Meta with no second word." );
      }else if( vlSwab16( s16[ pos + 1 ], swab ) < 0xDC00 || vlSwab16( s16[ pos + 1 ], swab ) > 0xDFFF  ){
        err = 1;
        errfunc( "Invalid second word in surrogate pair in vlUTF16Meta out of range 0xDC00-0xDFFF." );
      }else
        cp = 0x10000 + ( ( vlSwab16( s16[ pos ], swab ) & 0x03FF ) << 10 ) + ( vlSwab16( s16[ pos + 1 ], swab ) & 0x03FF );
      pos += 2;
    } else
      cp = vlSwab16( s16[ pos++ ], swab );
    if( !err ){
     if( cp >= 0xD800 && cp <= 0xDBFF )
       errfunc( "Invalid unicode code point in vlUTF16Meta in the range 0xD800–0xDFFF." );
     else if( cp > 0x0010FFFF )
       errfunc( "Invalid unicode code point in vlUTF16Meta greater than 0x0010FFFF." );
     else
       func( cp, ans );
    }
  }
}
void vlUTF8Meta( const u8* src, u32 sz, u32 ans, void (*func)( u32, u32 ), void (*errfunc)( const u8* estr ) ){
  u32 pos = 0;
  u32 cp = 0;

  while( pos != sz ){
    int err = 0;
    if( src[ pos ] < 128 ){
      cp = src[ pos ];
      ++pos;
    } else if( src[ pos ] < 0xC0 ){
      err = 1;
      errfunc( "Invalid utf-8 multi-byte character in vlUTF8Meta." );
      ++pos;
    } else if( src[ pos ] < 0xC2 ){
      err = 1;
      errfunc( "Invalid utf-8 overlong encoding in vlUTF8Meta." );
      ++pos;
    } else if( src[ pos ] < 0xE0 ){
      cp = ( src[ pos ] & 0x1F ) << 6;
      if( pos + 1 >= sz || src[ pos + 1 ] < 0x80 || src[ pos + 1 ] > 0xBF ){ 
        err = 1;
        ++pos;       
        errfunc( "Invalid second byte in two byte encoding in vlUTF8Meta." );
      } else{
        cp += src[ pos + 1 ] & 0x3F;
        pos += 2;
      }
    } else if( src[ pos ] < 0xF0 ){
      cp = ( src[ pos ] & 0x0F ) << 12;
      if( pos + 1 >= sz || src[ pos + 1 ] < 0x80 || src[ pos + 1 ] > 0xBF ){
        err = 1;
        ++pos;
        errfunc( "Invalid second byte in three byte encoding in vlUTF8Meta." );
      }else {
        cp += ( src[ pos + 1 ] & 0x3F ) << 6;
        if( pos + 2 >= sz || src[ pos + 2 ] < 0x80 || src[ pos + 2 ] > 0xBF ){
          err = 1;
          pos += 2;
          errfunc( "Invalid third byte in three byte encoding in vlUTF8Meta." );
        } else{
          cp += src[ pos + 2 ] & 0x3F;
          pos += 3;
        }
      }
    } else if( src[ pos ] < 0xF5 ){
      cp = ( src[ pos ] & 0x07 ) << 18;
      if( pos + 1 >= sz || src[ pos + 1 ] < 0x80 || src[ pos + 1 ] > 0xBF ){
        err = 1;
        ++pos;
        errfunc( "Invalid second byte in four byte encoding in vlUTF8Meta." );
      } else{
        cp += ( src[ pos + 1 ] & 0x3F ) << 12;
        if( pos + 2 >= sz || src[ pos + 2 ] < 0x80 || src[ pos + 2 ] > 0xBF ){
          err = 1;
          pos += 2;
          errfunc( "Invalid third byte in four byte encoding in vlUTF8Meta." );
        } else{
          cp += ( src[ pos + 2 ] & 0x3F ) << 6;
          if( pos + 3 >= sz || src[ pos + 3 ] < 0x80 || src[ pos + 3 ] > 0xBF ){
            err = 1;
            pos += 3;
            errfunc( "Invalid fourth byte in four byte encoding in vlUTF8Meta." );
          } else{
            cp += src[ pos + 3 ] & 0x3F;
            pos += 4;
          }
        }
      }
    } else{ 
      err = 1;
      ++pos;
      errfunc( "Invalid byte sequence in vlUTF8Meta." );
    }
    if( !err ){
     if( cp >= 0xD800 && cp <= 0xDBFF )
       errfunc( "Invalid unicode code point in vlUTF8Meta in the range 0xD800–0xDFFF." );
     else if( cp > 0x0010FFFF )
       errfunc( "Invalid unicode code point in vlUTF8Meta greater than 0x0010FFFF." );
     else
       func( cp, ans );
    }
  }
}
static void pvl8to16leHelper( u32 cp, u32 ans ){
  u32 cpp = vlUTF32ToUTF16( cp );
  if( cpp & 0xFFFF0000 )
    vlAppend( ans, &cpp, 4 );
  else
    vlAppend( ans, &cpp, 2 );
}
static void pvl8to16beHelper( u32 cp, u32 ans ){
  u32 cpp = vlUTF32ToUTF16( cp );
  cpp = ( vlSwab16( ( cpp >> 16 ), 1 ) << 16 ) + vlSwab16( cpp & 0x0000FFFF, 1 );
  if( cpp & 0xFFFF0000 )
    vlAppend( ans, &cpp, 4 );
  else
    vlAppend( ans, &cpp, 2 );
}
static void pvl16to8helper( u32 cp, u32 ans ){
  u32 cpp = vlUTF32ToUTF8( cp );
  u32 mask = (u32)0x000000FF << 24;
  u32 shft = 24;
  u32 i;
  u8 c;
  int wrt = 0;
  for( i = 0; i < 4; ++i, mask >>= 8, shft -= 8 ){
    c = (u8)( ( cpp & mask ) >> shft );
    if( c || i == 3 )
      wrt = 1;
    if( wrt )
      vlAppend( ans, &c, 1 );
  }
}
static void pvlto32leHelper( u32 cp, u32 ans ){
  vlAppend( ans, &cp, 4 );
}
static void pvlto32beHelper( u32 cp, u32 ans ){
  cp = vlSwab32( cp, 1 );
  vlAppend( ans, &cp, 4 );
}
u32 vlUTF8ToUTF16( const u8* str, u32 sz, void (*errfunc)( const u8* estr ), int bom, int le ){
  u32 ans = vlAutoMalloc( 0 );
  if( bom ){
    if( le )
      vlAppend( ans, "\xFF\xFE", 2 );
    else
      vlAppend( ans, "\xFE\xFF", 2 );
  }
  vlUTF8Meta( str, sz, ans, le ? pvl8to16leHelper : pvl8to16beHelper, errfunc ); 
  return ans;
}
u32 vlUTF16ToUTF8( const u8* str, u32 sz, int defaultToLittleEndian, void (*errfunc)( const u8* estr ) ){
  u32 ans = vlAutoMalloc( 0 );
  vlUTF16Meta( str, sz, ans, defaultToLittleEndian, pvl16to8helper, errfunc );
  return ans;
}
u32 vlUTF16Len( const u8* str ){
  u32 sz = 0;
  while( ( (const u16*)str )[ sz ] )
    ++sz;
  return sz * 2;
}
u32 vlUTF32Len( const u8* str ){
  u32 sz = 0;
  while( ( (const u32*)str )[ sz ] )
    ++sz;
  return sz * 4;
}
u32 vlUTF8BytePos( const u8* src, u32 sz, u32 cp ){
  u32 pos = 0;
  while( pos < sz && cp ){
    if( src[ pos ] < 128 )
      ++pos;
    else if( src[ pos ] < 0xE0 )
      pos += 2;  
    else if( src[ pos ] < 0xF0 )
      pos += 3;
    else if( src[ pos ] < 0xF5 )
      pos += 4;
    else
      ++pos;
    --cp;
  }
  if( pos >= sz )
    return sz;
  else
    return pos;
}
u32 vlUTF8CharPos( const u8* src, u32 sz, u32 bp ){
  u32 pos = 0;
  u32 cp = 0;
  if( bp > sz )
    bp = sz;
  while( pos < bp ){
    if( src[ pos ] < 128 )
      ++pos;
    else if( src[ pos ] < 0xE0 )
      pos += 2;  
    else if( src[ pos ] < 0xF0 )
      pos += 3;
    else if( src[ pos ] < 0xF5 )
      pos += 4;
    else
      ++pos;
    ++cp;
  }
  return cp;
}

void* vlMalloc( u32 sz ){ 
#ifdef VLDEBUG
  ++( osGetTLS() )->pvlMallocCount;
#endif
  return osHeapMalloc( sz ); 
}
void vlFree( void* d ){ 
  if( d != NULL ){
#ifdef VLDEBUG
    --( osGetTLS() )->pvlMallocCount;
#endif
    osHeapFree( d ); 
  }
}
void* vlRealloc( void* d, u32 sz ){ 
  if( d == NULL )
    return vlMalloc( sz );
  if( !sz ){
    vlFree( d );
    return NULL;
  }
  return osHeapRealloc( d, sz ); 
}
void* vlGlobalMalloc( u32 sz ){ return osGlobalMalloc( sz ); }
void vlGlobalFree( void* data ){ osGlobalFree( data ); }


u32 vlAutoMalloc( u32 sz ){
  vlState* vltls = osGetTLS();
  vlAssess( vltls->pvlAllocs != NULL, "Bad vlAutoMalloc!" );
  if( vltls->pvlNumFrees ){
    u32 ti = vltls->pvlFrees[ --vltls->pvlNumFrees ];
    vlAssess( vltls->pvlAllocs[ ti ] == NULL, "Memory corruption!" );
    vltls->pvlAllocs[ ti ] = vlMalloc( sz + 1 );
    vltls->pvlAllocCounts[ ti ] = sz;
    vltls->pvlAllocBuffSizes[ ti ] = sz + 1;
    return ti + 1;
  } else{
    while( vltls->pvlAllocCount >= vltls->pvlAllocBuffSize ){
      void* nb = vlMalloc( sizeof( void* ) * vltls->pvlAllocBuffSize * 2 );
      vltls->pvlAllocBuffSize *= 2;
      memcpy( nb, vltls->pvlAllocs, sizeof( void* ) * vltls->pvlAllocCount );
      vlFree( vltls->pvlAllocs ); vltls->pvlAllocs = nb;
      nb = vlMalloc( sizeof( u32 ) * vltls->pvlAllocBuffSize );
      memcpy( nb, vltls->pvlAllocCounts, sizeof( u32 ) * vltls->pvlAllocCount );
      vlFree( vltls->pvlAllocCounts ); vltls->pvlAllocCounts = nb;
      nb = vlMalloc( sizeof( u32 ) * vltls->pvlAllocBuffSize );
      memcpy( nb, vltls->pvlAllocBuffSizes, sizeof( u32 ) * vltls->pvlAllocCount );
      vlFree( vltls->pvlAllocBuffSizes ); vltls->pvlAllocBuffSizes = nb;
      nb = vlMalloc( sizeof( u32 ) * vltls->pvlAllocBuffSize );
      memcpy( nb, vltls->pvlFrees, sizeof( u32 ) * vltls->pvlNumFrees );
      vlFree( vltls->pvlFrees ); vltls->pvlFrees = nb;
    }
    vltls->pvlAllocs[ vltls->pvlAllocCount ] = vlMalloc( sz + 1 );
    vltls->pvlAllocCounts[ vltls->pvlAllocCount ] = sz;
    vltls->pvlAllocBuffSizes[ vltls->pvlAllocCount ] = sz + 1;
    return ++vltls->pvlAllocCount;
  }
}
void vlAutoFree( u32 i ){
  vlState* vltls = osGetTLS();
  if( vltls->pvlAllocs == NULL )
    osDieError( "vlAutoFree called before initializtion!" );
  vlAssess( !( !i || ( i > vltls->pvlAllocCount ) ), "Invalid pointer reference in vlAutoFree!" );
  vlAssess( vltls->pvlAllocs[ i - 1 ] != NULL, "Double vlAutoFree!" );
  vltls->pvlFrees[ vltls->pvlNumFrees++ ] = i - 1;
  vlFree( vlMem( i ) );
  vltls->pvlAllocs[ i - 1 ] = NULL;
}
void* vlMem( u32 i ){
  vlState* vltls = osGetTLS();
  vlAssess( !( !i || ( i > vltls->pvlAllocCount ) ), "Invalid pointer reference in vlMem!" );
  return vltls->pvlAllocs[ i - 1 ];
}
u32 vlSize( u32 i ){
  vlState* vltls = osGetTLS();
  vlAssess( !( !i || ( i > vltls->pvlAllocCount ) ), "Invalid pointer reference in vlSize!" );
  return vltls->pvlAllocCounts[ i - 1 ];
}
void vlRemove( u32 i, u32 pos, u32 sz ){
  vlState* vltls = osGetTLS();
  u32 m = i - 1;
  u8* ts,* te,* tp;
  vlAssess( !( !i || ( i > vltls->pvlAllocCount ) ), "Invalid pointer reference in vlRemove!" );
  vlAssess( pos + sz <= vltls->pvlAllocCounts[ m ], "An attempt was made to remove more memory than exists in vlRemove!" );
  ts = ( (u8*)vlMem( i ) ) + pos;
  te = ts + sz;
  tp = ( (u8*)vlMem( i ) ) + vltls->pvlAllocCounts[ m ];
  while( te < tp )
    *(ts++) = *(te++);
  vltls->pvlAllocCounts[ m ] -= sz;
}
void vlPopNul( u32 i ){
  vlState* vltls = osGetTLS();
  u32 m = i - 1;
  vlAssess( !( !i || ( i > vltls->pvlAllocCount ) ), "Invalid pointer reference in vlPopNul!" );
  if( vlSize( i ) && !( ( (u8*)vlMem( i ) )[ vlSize( i ) - 1 ] ) )
    --vltls->pvlAllocCounts[ m ];  
}
void vlAppend( u32 i, const void* nm, u32 sz ){
  vlState* vltls = osGetTLS();
  u32 m = i - 1;
  vlAssess( !( !i || ( i > vltls->pvlAllocCount ) ), "Invalid pointer reference in vlAppend!" );
  if( vltls->pvlAllocCounts[ m ] + sz >= vltls->pvlAllocBuffSizes[ m ] ){
    void* nb;
    u32 nsz = vltls->pvlAllocBuffSizes[ m ] * 2;
    while( nsz < vltls->pvlAllocCounts[ m ] + sz )
      nsz *= 2;
    nb = vlMalloc( nsz );
    vltls->pvlAllocBuffSizes[ m ] = nsz;
    memcpy( nb, vltls->pvlAllocs[ m ], vltls->pvlAllocCounts[ m ] );
    vlFree( vltls->pvlAllocs[ m ] );
    vltls->pvlAllocs[ m ] = nb;
  }
  memcpy( (u8*)vltls->pvlAllocs[ m ] + vltls->pvlAllocCounts[ m ], nm, sz );
  vltls->pvlAllocCounts[ m ] += sz;
}
void vlGrow( u32 i, u32 sz ){
  vlState* vltls = osGetTLS();
  u32 m = i - 1;
  vlAssess( !( !i || ( i > vltls->pvlAllocCount ) ), "Invalid pointer reference in vlGrow!" );
  if( vltls->pvlAllocCounts[ m ] + sz >= vltls->pvlAllocBuffSizes[ m ] ){
    void* nb;
    u32 nsz = vltls->pvlAllocBuffSizes[ m ] * 2;
    while( nsz < vltls->pvlAllocCounts[ m ] + sz )
      nsz *= 2;
    nb = vlMalloc( nsz );
    vltls->pvlAllocBuffSizes[ m ] = nsz;
    memcpy( nb, vltls->pvlAllocs[ m ], vltls->pvlAllocCounts[ m ] );
    vlFree( vltls->pvlAllocs[ m ] );
    vltls->pvlAllocs[ m ] = nb;
  }
  {
    u8* mp = (u8*)vltls->pvlAllocs[ m ] + vltls->pvlAllocCounts[ m ];
    u32 k;
    for( k = 0; k < sz; ++k )
      mp[ k ] = '\0';
  }
  vltls->pvlAllocCounts[ m ] += sz;
}
void vlAppendString( u32 i, const u8* msg ){
  vlState* vltls = osGetTLS();
  if( msg != NULL ){
    u32 m = i - 1;
    u32 sz = 0;
    while( msg[ sz ] )
      ++sz;
    vlAssess( !( !i || ( i > vltls->pvlAllocCount ) ), "Invalid pointer reference in vlAppend!" );
    if( sz ){
      if( vltls->pvlAllocCounts[ m ] && !( (u8**)vltls->pvlAllocs )[ m ][ vltls->pvlAllocCounts[ m ] - 1 ] ){
        ( (u8**)vltls->pvlAllocs )[ m ][ vltls->pvlAllocCounts[ m ] - 1 ] = *msg;
        vlAppend( i, msg + 1, sz - 1 );
      }else
        vlAppend( i, msg, sz );
    }
    if( ( vltls->pvlAllocCounts[ m ] && ( (u8**)vltls->pvlAllocs )[ m ][ vltls->pvlAllocCounts[ m ] - 1 ] ) || !vltls->pvlAllocCounts[ m ] )
      vlAppend( i, "\0", 1 );
  }
}
void vlAppendInt( u32 i, s64 msg ){
  vlState* vltls = osGetTLS();
  u8* buf = vltls->pvlAppendIntBuf;
  u8* fub = vltls->pvlAppendIntFub;
  if( msg ){
    u32 j = 0, k;
    int neg = 0;
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
      fub[ k ] = buf[ j - k - 1 ];
    fub[ j ] = '\0';
  } else{
    fub[ 0 ] = '0';
    fub[ 1 ] = '\0';
  }
  vlAppendString( i, fub );
}
u32 vlLoadFile( const u8* name ){
  return osLoadFile( name );
}
int vlSaveFile( const u8* name, const u8* data, u32 sz, int overwrite ){
  return osSaveFile( name, data, sz, overwrite );
}
u32 vlNumThreads( void ){
  u32 ans = vlGetGlobal()->numThreads;
  vlReleaseGlobal();
  return ans;
}

typedef struct {
  u8* parentId;
  u8* src;
  u8* inheritance;
  u8* srcName;
} pvlNewThreadStruct;
static void pvlNewThreadFunc( void* d ){
  pvlNewThreadStruct* p = d;
  vlInit();
  if( p->inheritance != NULL && p->parentId != NULL ){
    u32 i;
    u32 exe = vlAutoMalloc( 0 );
    u32 ine = vlGetNumQuoteEquals( p->inheritance );
    vlAppendString( exe, "ovel.parentThreadId = " );
    vlAppendString( exe, p->parentId );
    vlAppendString( exe, "\novel.inheritance = [" );
    for( i = 0; i < ine; ++i ) vlAppendString( exe, "=" );
    vlAppendString( exe, "[\n" );
    vlAppendString( exe, p->inheritance );
    vlAppendString( exe, "]" );
    for( i = 0; i < ine; ++i ) vlAppendString( exe, "=" );
    vlAppendString( exe, "]" );
    vlRunString( vlMem( exe ), "<new thread setup>" );
    vlAutoFree( exe );
    vlRunStringNonFatal( p->inheritance, "<inheritance>" );
  }
  {
    u32 exe = vlAutoMalloc( 0 );
    vlAppendString( exe, "ovel.threadId = " );
    vlAppendInt( exe, osGetThreadId() );
    vlAppendString( exe, "\n" );
    vlRunString( vlMem( exe ), "<set tid>" );
    vlAutoFree( exe );
  }
  vlRunStringNonFatal( p->src, p->srcName );
  vlQuit();
  vlGlobalFree( p->src );
  vlGlobalFree( p->parentId );
  vlGlobalFree( p->inheritance );
  vlGlobalFree( p->srcName );
  vlGlobalFree( d );
}
void vlNewThread( const u8* src, const u8* pid, const u8* inh, const u8* srcName ){
  pvlNewThreadStruct* pts = vlGlobalMalloc( sizeof( pvlNewThreadStruct ) );
  u32 esz;
  u32 ssz;
#ifndef VLDEBUG
  if( src == NULL ) osDieError( "Null pointer in vlNewThread!" );
  if( pid != NULL ){
    u32 i;
    u32 sz = strlen( pid );
    for( i = 0; i < sz; ++i )
      if( pid[ i ] > '9' || pid[ i ] < '0' )
        osDieError( "Non-numeric parent id in vlNewThread!" );
  }
#endif
  esz = strlen( src ) + 1;
  if( srcName == NULL )
    srcName = "<new thread>";
  ssz = strlen( srcName ) + 1;
  if( pid != NULL && inh != NULL ){
    u32 isz = strlen( inh ) + 1;
    u32 psz = strlen( inh ) + 1;
    pts->inheritance = vlGlobalMalloc( isz ); memcpy( pts->inheritance, inh, isz );
    pts->parentId = vlGlobalMalloc( psz ); memcpy( pts->parentId, pid, psz );
  } else
    pts->inheritance = pts->parentId = NULL;
  pts->src = vlGlobalMalloc( esz ); memcpy( pts->src, src, esz );
  pts->srcName = vlGlobalMalloc( ssz ); memcpy( pts->srcName, srcName, ssz );
  osCreateThread( pvlNewThreadFunc, pts );
}
u32 vlGetNumQuoteEquals ( const u8* str ){
  u32 ne = 0; 
  u32 mh, i;
  while( 1 ){
    mh = vlAutoMalloc( 0 );
    vlAppendString( mh, "]" );
    for( i = 0; i < ne; ++i ) 
      vlAppendString( mh, "=" );
    vlAppendString( mh, "]" );
    if( strstr( str, vlMem( mh ) ) == NULL ){
      vlAutoFree( mh );
      return ne;
    } else{
      vlAutoFree( mh );
      ++ne;
    }
  }
}
u32 vlStripPath( const u8* name ){
  return osStripPath( name );
}
void vlConsolePrint( const u8* str, u32 ssz, int err ){
  osConsolePrint( str, ssz, err );
}
u32 vlSelfFullName( void ){
  return osSelfName();
}
u32 vlSelfName( void ){
  u32 tb = osSelfName();  
  u32 ans = osStripPath( vlMem( tb ) );
  vlAutoFree( tb );
  return ans;
}
int vlLoadResource( u32 handle, void** mem, u32* sz, const u8* type ){
  return osLoadResource( handle, mem, sz, type );
}
u32 vlUncompress( const u8* mem, u32 sz ){
  u32 ssz = sz;
  u32 dsz;
  int stat;
  uLongf tdsz;
  const u8* src = mem;
  u32 dst;
  
  if( sz < 4 )
    return 0;
  dsz = *( (u32*)src );
  tdsz = dsz;
  src += 4;
  ssz -= 4;
  dst = vlAutoMalloc( dsz );
  stat = uncompress( vlMem( dst ), &tdsz, src, ssz );
 

  if( stat != Z_OK || tdsz != dsz ){
    vlAutoFree( dst );
    return 0;
  }
  return dst;
}
void vlSleep( u32 msec ){
  osSleep( msec );
}
int vlIsHandleAttached( int err ){
  vlState* vltls = osGetTLS();
  return ( err?vltls->pvlErrorHandle:vltls->pvlOutHandle ) != 0;
}
int vlIsIdle( void ){ return osIsIdle(); }
void vlItcPost( u32 thread, const u8* msg, u32 msz ){
 lua_State* vlgl = vlGetGlobal()->luaGlobal;
 lua_checkstack( vlgl, 8 );
 lua_getfield( vlgl, LUA_REGISTRYINDEX, "vlitc" ); 
  if( !lua_istable( vlgl, -1 ) ){
    lua_pop( vlgl, 1 );
    lua_createtable( vlgl, 0, 0 );
    lua_pushvalue( vlgl, -1 );
    lua_setfield( vlgl, LUA_REGISTRYINDEX, "vlitc" );
  } // vlitc
  lua_pushnumber( vlgl, (lua_Number)thread );
  lua_gettable( vlgl, -2 );  
  if( !lua_istable( vlgl, -1 ) ){
    lua_pop( vlgl, 1 ); // vlitc
    lua_newtable( vlgl ); // vlitc, nt
    lua_pushstring( vlgl, "f" );
    lua_pushnumber( vlgl, 1 );
    lua_settable( vlgl, -3 );
    lua_pushstring( vlgl, "l" );
    lua_pushnumber( vlgl, 0 );
    lua_settable( vlgl, -3 );
    lua_pushnumber( vlgl, (lua_Number)thread ); // vlitc, nt, tid
    lua_pushvalue( vlgl, -2 ); // vlitc, nt, tid, nt
    lua_settable( vlgl, -4 ); 
  } // vlitc, vlitc[ tid ] 
  lua_getfield( vlgl, -1, "l" ); // vlitc, vlitc[ tid ], vlitc[ tid ].l
  {
    lua_Number last = lua_tonumber( vlgl, -1 ) + 1;
    lua_pop( vlgl, 1 ); // vlitc, vlitc[ tid ]
    lua_pushnumber( vlgl, last ); // vlitc, vlitc[ tid ], vlitc[ tid ].l + 1
    lua_setfield( vlgl, -2, "l" ); // vlitc, vlitc[ tid ]
    lua_pushnumber( vlgl, last );  
    lua_pushlstring( vlgl, msg, msz ); // vlitc, vlitc[ tid ], vlitc[ tid ].l + 1, msg
    lua_settable( vlgl, -3 ); // vlitc, vlitc[ tid ]
  }
  lua_pop( vlgl, 2 ); //

  vlReleaseGlobal();
}
u32 vlItcRetrieve( u32 thread ){
  lua_State* vlgl = vlGetGlobal()->luaGlobal;
  u32 c = lua_gettop( vlgl );
  u32 ans;

  lua_checkstack( vlgl, 8 );
  lua_getfield( vlgl, LUA_REGISTRYINDEX, "vlitc" );
  if( !lua_istable( vlgl, -1 ) ){
    lua_settop( vlgl, c );
    vlReleaseGlobal();
    return 0;
  } // vlitc
  lua_pushnumber( vlgl, thread );
  lua_gettable( vlgl, -2 );
  if( !lua_istable( vlgl, -1 ) ){
    lua_settop( vlgl, c );
    vlReleaseGlobal();
    return 0;
  } // vlitc, vlitc[ tid ]
  lua_getfield( vlgl, -1, "f" ); // vlitc, vlitc[ tid ], vlitc[ tid ].f
  lua_getfield( vlgl, -2, "l" ); // vlitc, vlitc[ tid ], vlitc[ tid ].f, vlitc[ tid ].l
  {
    lua_Number first = lua_tonumber( vlgl, -2 );
    lua_Number last = lua_tonumber( vlgl, -1 );
    if( first > last ){
      lua_settop( vlgl, c );
      vlReleaseGlobal();
      return 0;
    }
    lua_pop( vlgl, 2 ); // vlitc, vlitc[ tid ]
    lua_pushnumber( vlgl, first + 1 ); // vlitc, vlitc[ tid ], vlitc[ tid ].f + 1
    lua_setfield( vlgl, -2, "f" ); // vlitc, vlitc[ tid ]
    lua_pushnumber( vlgl, first ); // vlitc, vlitc[ tid ], vlitc[ tid ].f
    lua_gettable( vlgl, -2 ); // vlitc, vlitc[ tid ], vlitc[ tid ][ vlitc[ tid ].f ]
    {
      const u8* str;
      u32 ssz;
      str = lua_tolstring( vlgl, -1, &ssz );
      ans = vlAutoMalloc( ssz );
      memcpy( vlMem( ans ), str, ssz );
    }
    lua_pop( vlgl, 1 ); // vlitc, vlitc[ tid ]
    lua_pushnumber( vlgl, first ); // vlitc, vlitc[ tid ], vlitc[ tid ].f
    lua_pushnil( vlgl ); // vlitc, vlitc[ tid ], vlitc[ tid ].f, nil
    lua_settable( vlgl, -3 ); // vlitc, vlitc[ tid ]
  }
  lua_pop( vlgl, 2 ); // 
  vlReleaseGlobal();
  
  return ans;
} 
u32 vlIdeThread( void ){
  vlState* vltls = osGetTLS();
  lua_State* vlls = vltls->ls;
  if( vltls->pvlIdeThread )
    return ( osIsThreadRunning( vltls->pvlIdeThread ) )?vltls->pvlIdeThread:0;
  lua_checkstack( vlls, 5 );
  lua_getglobal( vlls, "ovel" );
  if( lua_isnil( vlls, -1 ) ){
    lua_pop( vlls, 1 );
    return 0;
  }
  lua_getfield( vlls, -1, "ideThread" );
  if( !lua_isnumber( vlls, -1 ) ){
    lua_pop( vlls, 2 );
    return 0;
  }
  vltls->pvlIdeThread = (u32)lua_tonumber( vlls, -1 );
  lua_pop( vlls, 2 );
  return ( osIsThreadRunning( vltls->pvlIdeThread ) )?vltls->pvlIdeThread:0;
}
void vlPrint( const u8* str, u32 ssz, int err ){
  u32 ide = vlIdeThread();
  if( ide ){
    u8* msg = vlMalloc( ssz + 5 );
    if( err ){
      u32 ta = vlAutoMalloc( 0 );
      vlAppendString( ta, "wError in thread " ); vlAppendInt( ta, osGetThreadId() ); vlAppendString( ta, ":" ); vlPopNul( ta );
      vlItcPost( ide, vlMem( ta ), vlSize( ta ) );
      vlAutoFree( ta );
    } 
    if( err )
      *msg = 'i';
    else
      *msg = 'n';
      
    memcpy( msg + 1, str, ssz );
    if( err ){
      memcpy( msg + ssz + 1, "\n\n", 2 );
      vlItcPost( ide, msg, ssz + 3 );
    } else
      vlItcPost( ide, msg, ssz + 1 );
    vlFree( msg );
    if( vlIsHandleAttached( err ) )
      vlConsolePrint( str, ssz, err );
  } else{
    if( vlIsHandleAttached( err ) )
      vlConsolePrint( str, ssz, err );
    else{
      vlState* vltls = osGetTLS();
      if( err ){
        vlAppend( vltls->pvlEBuff, str, ssz );
        vlAppend( vltls->pvlEBuff, "\n", 1 );
      }else
        vlAppend( vltls->pvlOBuff, str, ssz );
    }
  }
}
