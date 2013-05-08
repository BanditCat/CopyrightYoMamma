// Helpers
#include "expat.h"
// Returns a string with the type of the argument at index i.  MUST BE DEALLOCATED WITH vlFree;
u8* ppvlGetType( lua_State* s, int i ){
  u8* ans;
  const u8* tsp;
  int t = lua_type( s, i );
  int tp = lua_gettop( s );
  if( t == LUA_TUSERDATA || t == LUA_TTABLE ){
    lua_checkstack( s, 10 );
    if( lua_getmetatable( s, i ) ){
      lua_getfield( s, -1, "__type" );
      if( lua_isfunction( s, -1 ) ){
        lua_call( s, 0, -1 );
        if( lua_isstring( s, -1 ) ){
          tsp = lua_tostring( s, -1 );
          ans = vlMalloc( strlen( tsp ) + 1 );
          strcpy( ans, tsp );
          lua_settop( s, tp );
          return ans;
        }
      } else if( lua_isstring( s, -1 ) ){
        tsp = lua_tostring( s, -1 );
        ans = vlMalloc( strlen( tsp ) + 1 );
        strcpy( ans, tsp );
        lua_settop( s, tp );
        return ans;
      }
    }
  }
  tsp =  lua_typename( s, t );
  ans = vlMalloc( strlen( tsp ) + 1 );
  strcpy( ans, tsp );
  return ans;
}

static int pvlInformError( lua_State* s ){
  u32 n = lua_gettop( s );
  switch( n ){
    case 1:
      if( lua_isstring( s, 1 ) ) vlInformError( lua_tostring( s, 1 ), "Error" ); break;
    case 2:
      if( lua_isstring( s, 1 ) && lua_isstring( s, 2 ) ) vlInformError( lua_tostring( s, 1 ), lua_tostring( s, 2 ) ); break;
  }
  return 0;
}
static int pvlInform( lua_State* s ){
  u32 n = lua_gettop( s );
  vlLuaAssess( s, n && lua_isstring( s, 1 ) && ( ( n == 1 ) || ( n == 2 && lua_isstring( s, 2 ) ) ), "Bad arguments to pvlInform." );
  switch( n ){
    case 1:
      vlInform( lua_tostring( s, 1 ), "Information" ); 
      break;
    case 2:
      vlInform( lua_tostring( s, 1 ), lua_tostring( s, 2 ) );
      break;
  }
  return 0;
}
static int pvlQuery( lua_State* s ){
  u32 n = lua_gettop( s );
  vlLuaAssess( s, n && lua_isstring( s, 1 ) && ( ( n == 1 ) || ( n == 2 && lua_isstring( s, 2 ) ) ), "Bad arguments to pvlInform." );
  switch( n ){
    case 1:
      lua_pushboolean( s, vlQuery( lua_tostring( s, 1 ), "Query" ) ); 
      break;
    case 2:
      lua_pushboolean( s, vlQuery( lua_tostring( s, 1 ), lua_tostring( s, 2 ) ) ); 
      break;
  }
  return 1;
}
static int pvlDie( lua_State* s ){
  if( ( lua_gettop( s ) == 1 ) && lua_isstring( s, 1 ) ) 
    vlDie( lua_tostring( s, 1 ) );
  else
    vlDie( "Unknown Error" );
  return 0;
}
static int pvlQuit( lua_State* s ){
  vlQuit();
  return 0;
}
static int pvlSysInfoVislibVersion( lua_State* s ){
  lua_pushstring( s, VISLIB_VERSION_STRING );
  return 1;
}
static int pvlSysInfoOsVersion( lua_State* s ){
  lua_pushstring( s, osGetVersion() );
  return 1;
}
static int pvlSysInfoLuaVersion( lua_State* s ){
  lua_pushstring( s, LUA_RELEASE );
  return 1;
}
static int pvlSysInfoDesktop( lua_State* s ){
  lua_newtable( s );
  lua_pushnumber( s, osDesktopX() ); lua_setfield( s, -2, "x" );
  lua_pushnumber( s, osDesktopY() ); lua_setfield( s, -2, "y" );
  lua_pushnumber( s, osDesktopWidth() ); lua_setfield( s, -2, "width" );
  lua_pushnumber( s, osDesktopHeight() ); lua_setfield( s, -2, "height" );
  return 1;
}
static int pvlSysInfoVirtualDisplay( lua_State* s ){
  lua_newtable( s );
  lua_pushnumber( s, osVirtualDisplayWidth() ); lua_setfield( s, -2, "width" );
  lua_pushnumber( s, osVirtualDisplayHeight() ); lua_setfield( s, -2, "height" );
  return 1;
}
static int pvlSysInfoDisplays( lua_State* s ){
  u32 t = 0;
  u32 i;
  u32 dispm = osGetDisplays();
  u32 cnt = *( (u32*)vlMem( dispm ) );
  osDisplay* disps = (osDisplay*)( (u8*)vlMem( dispm ) + 4 );
  lua_newtable( s );
  for( i = 0; i < cnt; ++i ){
    lua_checkstack( s, 5 );
    lua_pushnumber( s, ++t );
    lua_newtable( s );
    lua_pushnumber( s, disps[ i ].bpp ); lua_setfield( s, -2, "bpp" );
    lua_pushlstring( s, vlMem( disps[ i ].display ), vlSize( disps[ i ].display ) ); lua_setfield( s, -2, "display" );
    lua_pushlstring( s, vlMem( disps[ i ].identifier ), vlSize( disps[ i ].identifier ) ); lua_setfield( s, -2, "identifier" );
    lua_pushnumber( s, disps[ i ].frequency ); lua_setfield( s, -2, "frequency" );
    lua_pushnumber( s, disps[ i ].width ); lua_setfield( s, -2, "width" );
    lua_pushnumber( s, disps[ i ].height ); lua_setfield( s, -2, "height" );
    lua_settable( s, -3 );
    vlAutoFree( disps[ i ].display );
    vlAutoFree( disps[ i ].identifier );
  }
  vlAutoFree( dispm );
  return 1;
}
static int pvlGetUserData( lua_State* s ){
  u32 ans;
  vlLuaAssess( s, lua_gettop( s ) == 1 && lua_isstring( s, 1 ), "Bad arguments to pvlGetUserData (should be 1 string)." );
  if( 0 == ( ans = osGetUserData( lua_tostring( s, 1 ) ) ) )
    lua_pushnil( s );
  else{
    lua_pushlstring( s, vlMem( ans ), vlSize( ans ) );
    vlAutoFree( ans );
  }
  return 1;
}
static int pvlSetUserData( lua_State* s ){
  const u8* v;
  u32 sz;
  vlLuaAssess( s, lua_gettop( s ) == 2 && lua_isstring( s, 1 ) && lua_isstring( s, 2 ), "Bad arguments to pvlSetUserData (should be 2 strings)." );
  v = lua_tolstring( s, 2, &sz );
  if( !osSetUserData( lua_tostring( s, 1 ), v, sz ) )
    luaL_error( s, "Failed to set user data." );
  return 0;
}
static int pvlSetCWD( lua_State* s ){
  vlLuaAssess( s, lua_gettop( s ) == 1 && lua_isstring( s, 1 ), "Bad arguments to pvlSetCWD (should be 1 string)." );
  lua_pushboolean( s, osSetCWD( lua_tostring( s, 1 ) ) );
  return 1;
}
static int pvlGetCWD( lua_State* s ){
  u32 sp;
  vlLuaAssess( s, lua_gettop( s ) == 0, "Bad arguments to pvlGetCWD (should be no argumets)." );
  sp = osGetCWD();
  if( !sp )
    lua_pushnil( s );
  else
    lua_pushlstring( s, vlMem( sp ), vlSize( sp ) );
  if( sp )
    vlAutoFree( sp );
  return 1;
}
static int pvlStripPath( lua_State* s ){
  u32 sp;
  vlLuaAssess( s, lua_gettop( s ) == 1 && lua_isstring( s, 1 ), "Bad arguments to pvlStripPath (should be 1 string)." );
  sp = osStripPath( lua_tostring( s, 1 ) );
  if( !sp )
    lua_pushnil( s );
  else{
    vlPopNul( sp );
    lua_pushlstring( s, vlMem( sp ), vlSize( sp ) );
    vlAutoFree( sp );
  }
  return 1;
}
static int pvlGetFullPath( lua_State* s ){
  u32 sp;
  vlLuaAssess( s, lua_gettop( s ) == 1 && lua_isstring( s, 1 ), "Bad arguments to pvlGetFullPath (should be 1 string)." );
  sp = osGetFullPath( lua_tostring( s, 1 ) );
  if( !sp )
    lua_pushnil( s );
  else
    lua_pushlstring( s, vlMem( sp ), vlSize( sp ) );
  if( sp )
    vlAutoFree( sp );
  return 1;
}
static int pvlSaveFile( lua_State* s ){
  u32 sz;
  const u8* dt;
  int ow = 0;
  vlLuaAssess( s, ( lua_gettop( s ) / 2 ) == 1 && lua_isstring( s, 1 ) && lua_isstring( s, 2 ) && ( lua_gettop( s ) == 2 || lua_isboolean( s, 3 ) ), "Bad arguments to pvlSaveFile (should be 2 strings)." );
  dt = lua_tolstring( s, 2, &sz );
  if( lua_gettop( s ) == 3 )
    ow = lua_toboolean( s, 3 );
  lua_pushboolean( s, osSaveFile( lua_tostring( s, 1 ), dt, sz, ow ) );
  return 1;
}
static int pvlLoadFile( lua_State* s ){
  u32 lf;
  vlLuaAssess( s, lua_gettop( s ) == 1 && lua_isstring( s, 1 ), "Bad arguments to pvlLoadFile (should be 1 string)." );
  lf = osLoadFile( lua_tostring( s, 1 ) );
  if( !lf )
    lua_pushnil( s );
  else
    lua_pushlstring( s, vlMem( lf ), vlSize( lf ) );
  if( lf )
    vlAutoFree( lf );
  return 1;
}
static int pvlShowWindow( lua_State* s ){
  vlLuaAssess( s, lua_gettop( s ) == 1 && pvlIsUserType( s, 1, Window ), "Bad arguments to pvlShowWindow." );
  osShowWindow( pvlUserToVoidp( s, 1 ), 1 );
  return 0;
}
static int pvlShowWidget( lua_State* s ){
  vlLuaAssess( s, lua_gettop( s ) == 1 && pvlIsUserType( s, 1, Widget ), "Bad arguments to pvlShowWidget." );
  osShowWindow( pvlUserToVoidp( s, 1 ), 1 );
  return 0;
}
static int pvlHideWindow( lua_State* s ){
  vlLuaAssess( s, lua_gettop( s ) == 1 && pvlIsUserType( s, 1, Window ), "Bad arguments to pvlHideWindow." );
  osShowWindow( pvlUserToVoidp( s, 1 ), 0 );
  return 0;
}
static int pvlHideWidget( lua_State* s ){
  vlLuaAssess( s, lua_gettop( s ) == 1 && pvlIsUserType( s, 1, Widget ), "Bad arguments to pvlHideWidget." );
  osShowWindow( pvlUserToVoidp( s, 1 ), 0 );
  return 0;
}
static int pvlFocusWindow( lua_State* s ){
  vlLuaAssess( s, lua_gettop( s ) == 1 && pvlIsUserType( s, 1, Window ), "Bad arguments to pvlFocusWindow." );
  osFocusWindow( pvlUserToVoidp( s, 1 ) );
  return 0;
}
static int pvlFocusWidget( lua_State* s ){
  vlLuaAssess( s, lua_gettop( s ) == 1 && pvlIsUserType( s, 1, Widget ), "Bad arguments to pvlFocusWidget." );
  osFocusWindow( pvlUserToVoidp( s, 1 ) );
  return 0;
}
static int pvlGetTitleWindow( lua_State* s ){
  vlLuaAssess( s, lua_gettop( s ) == 1 && pvlIsUserType( s, 1, Window ), "Bad arguments to pvlGetTitleWindow." );
  {
    u8* ans = osGetTextWindow( pvlUserToVoidp( s, 1 ) );
    lua_pushstring( s, ans );
    vlFree( ans );
  }
  return 1;
}
static int pvlGetTextWidget( lua_State* s ){
  vlLuaAssess( s, lua_gettop( s ) == 1 && pvlIsUserType( s, 1, Widget ), "Bad arguments to pvlGetTextWidget." );
  {
    u8* ans = osGetTextWidget( pvlUserToVoidp( s, 1 ) );
    lua_pushstring( s, ans );
    vlFree( ans );
  }
  return 1;
}
static int pvlGetModify( lua_State* s ){
  vlLuaAssess( s, lua_gettop( s ) == 1 && pvlIsUserType( s, 1, Widget ), "Bad arguments to pvlGetModify." );
  lua_pushboolean( s, osGetModifyWidget( pvlUserToVoidp( s, 1 ) ) );
  return 1;
}
static int pvlSetModify( lua_State* s ){
  vlLuaAssess( s, lua_gettop( s ) == 2 && pvlIsUserType( s, 1, Widget ) && lua_isboolean( s, 2 ), "Bad arguments to pvlSetModify." );
  osSetModifyWidget( pvlUserToVoidp( s, 1 ), lua_toboolean( s, 2 ) );
  return 0;
}
static int pvlSetTitleWindow( lua_State* s ){
  vlLuaAssess( s, lua_gettop( s ) == 2 && pvlIsUserType( s, 1, Window ) && lua_isstring( s, 2 ), "Bad arguments to pvlSetTitleWindow." );
  osSetTitleWindow( pvlUserToVoidp( s, 1 ), lua_tostring( s, 2 ) );
  return 0;
}
static int pvlSetTextWidget( lua_State* s ){
  vlLuaAssess( s, lua_gettop( s ) == 2 && pvlIsUserType( s, 1, Widget ) && lua_isstring( s, 2 ), "Bad arguments to pvlSetTextWidget." );
  osSetTitleWindow( pvlUserToVoidp( s, 1 ), lua_tostring( s, 2 ) );
  return 0;
}
static int pvlSelectTextWidget( lua_State* s ){
  vlLuaAssess( s, lua_gettop( s ) == 3 && pvlIsUserType( s, 1, Widget ) && lua_isnumber( s, 2 ) && lua_isnumber( s, 3 ), "Bad arguments to pvlSelectTextWidget." );
  osSelectTextWidget( pvlUserToVoidp( s, 1 ), (u32)lua_tonumber( s, 2 ), (u32)lua_tonumber( s, 3 ) );
  return 0;
}
static int pvlGetSelectionTextWidget( lua_State* s ){
  u32 b, e;
  vlLuaAssess( s, lua_gettop( s ) == 1 && pvlIsUserType( s, 1, Widget ), "Bad arguments to pvlGetSelectionTextWidget (should be a text widget)." );
  osGetSelectionTextWidget( pvlUserToVoidp( s, 1 ), &b, &e );
  lua_pushnumber( s, b );
  lua_pushnumber( s, e );
  return 2;
}
static int pvlGetSelectionTextTextWidget( lua_State* s ){
  u32 ans;
  vlLuaAssess( s, lua_gettop( s ) == 1 && pvlIsUserType( s, 1, Widget ), "Bad arguments to pvlGetSelectionTextTextWidget (should be a text widget)." );
  ans = osGetSelectionTextTextWidget( pvlUserToVoidp( s, 1 ) );
  lua_pushlstring( s, vlMem( ans ), vlSize( ans ) );
  return 1;
}
static int pvlUndoTextWidget( lua_State* s ){
  vlLuaAssess( s, lua_gettop( s ) == 1 && pvlIsUserType( s, 1, Widget ), "Bad arguments to pvlUndoTextWidget (should be a text widget)." );
  osUndoTextWidget( pvlUserToVoidp( s, 1 ) );
  return 0;
}
static int pvlRedoTextWidget( lua_State* s ){
  vlLuaAssess( s, lua_gettop( s ) == 1 && pvlIsUserType( s, 1, Widget ), "Bad arguments to pvlRedoTextWidget (should be a text widget)." );
  osRedoTextWidget( pvlUserToVoidp( s, 1 ) );
  return 0;
}
static int pvlScrollWidget( lua_State* s ){
  vlLuaAssess( s, lua_gettop( s ) == 2 && pvlIsUserType( s, 1, Widget ) && lua_isnumber( s, 2 ), "Bad arguments given to pvlScrollWidget (should be a widget and a percentage)." );
  osScrollWidget( pvlUserToVoidp( s, 1 ), lua_tonumber( s, 2 ) );
  return 0;
}
static int pvlRelativeScrollWidget( lua_State* s ){
  vlLuaAssess( s, lua_gettop( s ) == 2 && pvlIsUserType( s, 1, Widget ) && lua_isnumber( s, 2 ), "Bad arguments given to pvlRelativeScrollWidget (should be a widget and a percentage)." );
  osRelativeScrollWidget( pvlUserToVoidp( s, 1 ), (s64)lua_tonumber( s, 2 ) );
  return 0;
}
static int pvlGetStyleWidget( lua_State* s ){
  vlWidgetStyle vwt;
  vlLuaAssess( s, lua_gettop( s ) == 1 && pvlIsUserType( s, 1, Widget ), "Bad arguments given to pvlGetStyleWidget (should be 1 widget)." );
  vwt = osGetStyleWidget( pvlUserToVoidp( s, 1 ) );
  if( vwt == text )
    lua_pushstring( s, "text" );
  else if( vwt == textNowrap )
    lua_pushstring( s, "textNowrap" );
  else if( vwt == statusbar )
    lua_pushstring( s, "statusbar" );
  else
    osDieError( "UNIMPLEMTED!! BORK BORK BORK!!!" );
  return 1;
}
static int pvlGetStyleWindow( lua_State* s ){
  vlWindowStyle vwt;
  vlLuaAssess( s, lua_gettop( s ) == 1 && pvlIsUserType( s, 1, Window ), "Bad arguments given to pvlGetStyleWindow (should be 1 window)." );
  vwt = osGetStyleWindow( pvlUserToVoidp( s, 1 ) );
  if( vwt == toolbox )
    lua_pushstring( s, "toolbox" );
  else if( vwt == app )
    lua_pushstring( s, "app" );
  else if( vwt == borderless )
    lua_pushstring( s, "borderless" );
  else
    osDieError( "UNIMPLEMTED!! BORK BORK BORK!!!" );
  return 1;
}
static int pvlSetPartTextWidget( lua_State* s ){
  vlLuaAssess( s, lua_gettop( s ) == 3 && pvlIsUserType( s, 1, Widget ) && lua_isnumber( s, 2 ) && lua_isstring( s, 3 ), "Bad arguments to pvlSetPartsWidget (should be a widget, a integer, and a string)." );
  osSetPartTextWidget( pvlUserToVoidp( s, 1 ), (u32)lua_tonumber( s, 2 ), lua_tostring( s, 3 ) );
  return 0;
}
static int pvlSetPartsWidget( lua_State* s ){
  u32 parts = vlAutoMalloc( 0 );
  u32 txts = vlAutoMalloc( 0 );
  vlLuaAssess( s, lua_gettop( s ) == 2 && pvlIsUserType( s, 1, Widget ) && lua_istable( s, 2 ), "Bad arguments to pvlSetPartsWidget (should be a widget and a table)." );
  lua_pushnil( s );
  while( lua_next( s, 2 ) ){
    if( lua_type( s, -1 ) == LUA_TSTRING && lua_type( s, -2 ) == LUA_TNUMBER ){
      int pt = (int)lua_tonumber( s, -2 );
      size_t l;
      u8* txt;
      const u8* st = lua_tolstring( s, -1, &l );
      txt = vlMalloc( l + 1 );
      memcpy( txt, st, l );
      txt[ l ] = '\0';
      vlAppend( parts, &pt, sizeof( int ) );
      vlAppend( txts, &txt, sizeof( u8* ) );
    }
    lua_pop( s, 1 );
  }
  osSetPartsWidget( pvlUserToVoidp( s, 1 ), vlMem( parts ), vlMem( txts ), vlSize( parts ) / sizeof( int ) );
  {
    u32 i, max = vlSize( txts ) / sizeof( u8* );
    for( i = 0; i < max; ++i )
      vlFree( ( (u8**)( vlMem( txts ) ) )[ i ] );
    vlAutoFree( txts );
    vlAutoFree( parts );
  }
  return 0;
}
static int pvlGetLineCountWidget( lua_State* s ){
  vlLuaAssess( s, lua_gettop( s ) == 1 && pvlIsUserType( s, 1, Widget ), "Bad arguments to pvlGetLineCountWidget (should be a widget)." );
  lua_pushnumber( s, osGetLineCountWidget( pvlUserToVoidp( s, 1 ) ) );
  return 1;
}
static int pvlLineFromCharWidget( lua_State* s ){
  vlLuaAssess( s, lua_gettop( s ) == 2 && pvlIsUserType( s, 1, Widget ) && lua_isnumber( s, 2 ), "Bad arguments to pvlLineFromCharWidget (should be a widget and a number)." );
  lua_pushnumber( s, osLineFromCharWidget( pvlUserToVoidp( s, 1 ), (u32)lua_tonumber( s, 2 ) ) );
  return 1;
}
static int pvlCharFromLineWidget( lua_State* s ){
  u32 ans;
  vlLuaAssess( s, lua_gettop( s ) == 2 && pvlIsUserType( s, 1, Widget ) && lua_isnumber( s, 2 ), "Bad arguments to pvlCharFromLineWidget (should be a widget and a number)." );
  ans = osCharFromLineWidget( pvlUserToVoidp( s, 1 ), (u32)lua_tonumber( s, 2 ) );
  if( ans == (u32)-1 )
    lua_pushnumber( s, -1 );
  else
    lua_pushnumber( s, (lua_Number)ans );
  return 1;
}
static int ppvlReplaceFormatedTextWidgetHelper( lua_State* s, osChangeType ct ){
  u32 fmt;
  u8* tp;
  const u8* fname;
  const u8* msg;
  int bullet = 0;
  u32 l;
  lua_Number r, g, b, size, firstLine, left, right;
  int bold, italic, underline;
  const u8* emsg = "Bad arguments to ppvlReplaceFormatedTextWidgetHelper (should be a widget, a fontStyle and a string).";
  vlLuaAssess( s, lua_gettop( s ) == 3 && pvlIsUserType( s, 1, Widget ) && lua_isstring( s, 3 ), emsg );
  msg = lua_tolstring( s, 3, &l );
  tp = ppvlGetType( s, 2 );
  if( strcmp( tp, "fontStyle" ) ){
    vlFree( tp );
    luaL_error( s, "%s", emsg );
  }
  vlFree( tp );
  lua_getfield( s, 2, "r" ); r = lua_tonumber( s, -1 ); lua_pop( s, 1 );
  lua_getfield( s, 2, "g" ); g = lua_tonumber( s, -1 ); lua_pop( s, 1 );
  lua_getfield( s, 2, "b" ); b = lua_tonumber( s, -1 ); lua_pop( s, 1 );
  lua_getfield( s, 2, "size" ); size = lua_tonumber( s, -1 ); lua_pop( s, 1 );
  lua_getfield( s, 2, "bold" ); bold = lua_toboolean( s, -1 ); lua_pop( s, 1 );
  lua_getfield( s, 2, "italic" ); italic = lua_toboolean( s, -1 ); lua_pop( s, 1 );
  lua_getfield( s, 2, "underline" ); underline = lua_toboolean( s, -1 ); lua_pop( s, 1 );
  {
    const u8* ts;
    lua_getfield( s, 2, "bullet" ); 
    ts = lua_tostring( s, -1 ); 
    if( !strcmp( ts, "*" ) )
      bullet = -1;
    else{
      if( !strcspn( ts, "0123456789" ) )
        sscanf( ts, "%i", &bullet );
    }
    lua_pop( s, 1 );
  }
  fmt = ( bold ? vlBold : 0 ) | ( italic ? vlItalic : 0 ) | ( underline ? vlUnderline : 0 );
  lua_getfield( s, 2, "font" ); fname = lua_tostring( s, -1 );
  lua_getfield( s, 2, "margin" );
  lua_getfield( s, -1, "firstLine" ); firstLine = lua_tonumber( s, -1 ); lua_pop( s, 1 );
  lua_getfield( s, -1, "left" ); left = lua_tonumber( s, -1 ); lua_pop( s, 1 );
  lua_getfield( s, -1, "right" ); right = lua_tonumber( s, -1 ); lua_pop( s, 1 );
  osChangeFormatedTextWidget( pvlUserToVoidp( s, 1 ), size, r, g, b, fmt, fname, msg, l, firstLine, left, right, ct, bullet );
  lua_pop( s, 2 );
  return 1;
}
static int pvlReplaceSelectionFormatedTextWidget( lua_State* s ){ return ppvlReplaceFormatedTextWidgetHelper( s, replaceSelection ); }
static int pvlReplaceAllFormatedTextWidget( lua_State* s ){ return ppvlReplaceFormatedTextWidgetHelper( s, replaceAll ); }
static int pvlReplaceSelectionWithHR( lua_State* s ){
  lua_Number r, g, b, size;
  u8* tp;
  const u8* emsg = "Bad arguments to pvlReplaceSelectionWithHR (should be a widget, a color and a number).";
  vlLuaAssess( s, lua_gettop( s ) == 3 && pvlIsUserType( s, 1, Widget ) && lua_isnumber( s, 3 ), emsg );
  tp = ppvlGetType( s, 2 );
  if( strcmp( tp, "color" ) ){
    vlFree( tp );
    luaL_error( s, "%s", emsg );
  }
  size = lua_tonumber( s, 3 );
  vlFree( tp );
  lua_getfield( s, 2, "r" ); r = lua_tonumber( s, -1 ); lua_pop( s, 1 );
  lua_getfield( s, 2, "g" ); g = lua_tonumber( s, -1 ); lua_pop( s, 1 );
  lua_getfield( s, 2, "b" ); b = lua_tonumber( s, -1 ); lua_pop( s, 1 );
  osReplaceSelectionWithHRTextWidget( pvlUserToVoidp( s, 1 ), size, r, g, b );
  lua_pop( s, 1 );
  return 1;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Expat interface.  Builds a table from some XML, returns true and the table or false and
// a error message.

#define ExpatBuffSize 8192
typedef struct{ 
  int reading;
  u8* rbuf;
  u32 wbuf;
  u32 lvl;
  lua_State* s;
} ExpatParseStruct;
static void hpvlExpatStartHandler( void *data, const char *el, const char **attr ){
  ExpatParseStruct* ep = data;
  const char** ap = attr;
  lua_checkstack( ep->s, 10 );
  if( ep->reading ){
    lua_pushnumber( ep->s, lua_objlen( ep->s, -1 ) + 1 );
    lua_pushlstring( ep->s, vlMem( ep->wbuf ), vlSize( ep->wbuf ) );
    lua_settable( ep->s, -3 );
    vlAutoFree( ep->wbuf );
    ep->wbuf = vlAutoMalloc( 0 );
  }
  ep->reading = 0;
  lua_newtable( ep->s );
  lua_pushstring( ep->s, "element" );
  lua_setfield( ep->s, -2, "type" );
  lua_pushstring( ep->s, el );
  lua_setfield( ep->s, -2, "element" );
  if( *ap != NULL ){
    lua_pushstring( ep->s, "attributes" );
    lua_newtable( ep->s );
    while( *ap != NULL ){
      lua_pushstring( ep->s, *( ap++ ) );
      lua_pushstring( ep->s, *( ap++ ) );
      lua_settable( ep->s, -3 );
    }
    lua_settable( ep->s, -3 );
  }
}

static void hpvlExpatEndHandler( void *data, const char *el ){
  ExpatParseStruct* ep = data;
  lua_checkstack( ep->s, 10 );
  if( ep->reading ){
    lua_pushnumber( ep->s, lua_objlen( ep->s, -1 ) + 1 );
    lua_pushlstring( ep->s, vlMem( ep->wbuf ), vlSize( ep->wbuf ) );
    lua_settable( ep->s, -3 );
    vlAutoFree( ep->wbuf );
    ep->wbuf = vlAutoMalloc( 0 );
  }
  ep->reading = 0;
  lua_pushnumber( ep->s, lua_objlen( ep->s, -2 ) + 1 );
  lua_insert( ep->s, -2 );
  lua_settable( ep->s, -3 );
}

static void hpvlExpatCharacterHandler( void *data, const char *txt, int sz ){
  ExpatParseStruct* ep = data;
  ep->reading = 1;
  vlAppend( ep->wbuf, txt, sz );
}
static void hpvlExpatEntityHandler( void *data, const char *ent, int ispar ){
  ExpatParseStruct* ep = data;
  lua_checkstack( ep->s, 10 );
  if( ep->reading ){
    lua_pushnumber( ep->s, lua_objlen( ep->s, -1 ) + 1 );
    lua_pushlstring( ep->s, vlMem( ep->wbuf ), vlSize( ep->wbuf ) );
    lua_settable( ep->s, -3 );
    vlAutoFree( ep->wbuf );
    ep->wbuf = vlAutoMalloc( 0 );
  }
  ep->reading = 0;
  lua_pushnumber( ep->s, lua_objlen( ep->s, -1 ) + 1 );
  lua_createtable( ep->s, 0, 2 );
  lua_pushstring( ep->s, "entity" );
  lua_setfield( ep->s, -2, "type" );
  lua_pushstring( ep->s, ent );
  lua_setfield( ep->s, -2, "entity" );
  lua_settable( ep->s, -3 );
}
static int pvlExpatParseXML( lua_State* s ){
  int suc = 1;
  const u8* str;
  u32 ssz;
  u32 spos = 0;
  ExpatParseStruct ep;
  XML_Memory_Handling_Suite mh;
  XML_Parser p;
  mh.malloc_fcn = vlMalloc;
  mh.free_fcn = vlFree;
  mh.realloc_fcn = vlRealloc;
  vlLuaAssess( s, lua_gettop( s ) == 1 && lua_isstring( s, 1 ), "Bad arguments to pvlExpatParseXML (should be 1 string)." );
  p = XML_ParserCreate_MM( NULL, &mh, NULL );
  vlLuaAssess( s, p != NULL, "Failed to create Expat XML parser object in pvlExpatParseXML." );
  ep.rbuf = vlMalloc( ExpatBuffSize + 5 );
  ep.wbuf = vlAutoMalloc( 0 );
  ep.s = s;
  ep.lvl = 2;
  ep.reading = 0;
  str = lua_tolstring( s, 1, &ssz );
  XML_SetElementHandler( p, hpvlExpatStartHandler, hpvlExpatEndHandler );
  XML_SetCharacterDataHandler( p, hpvlExpatCharacterHandler );
  XML_SetSkippedEntityHandler( p, hpvlExpatEntityHandler );
  XML_SetUserData( p, &ep );
  lua_checkstack( s, 5 );
  lua_newtable( s );  

  while( spos < ssz ){
    const u8* rpos = str + spos;
    u32 rsz = ( ( ssz - spos ) < ExpatBuffSize ) ? ( ssz - spos ) : ExpatBuffSize;
    spos += rsz;
    if( XML_Parse( p, rpos, rsz, spos == ssz ) == XML_STATUS_ERROR ){
      u32 abuf = vlAutoMalloc( 0 );
      vlAppendString( abuf, "XML parsing error at line " ); vlAppendInt( abuf, XML_GetCurrentLineNumber( p ) ); 
      vlAppendString( abuf, ":\n" ); vlAppendString( abuf, XML_ErrorString( XML_GetErrorCode( p ) ) ); vlAppendString( abuf, "\n" );
      suc = 0;
      lua_settop( s, 1 );
      lua_pushboolean( s, 0 );
      vlPopNul( abuf );
      lua_pushlstring( s, vlMem( abuf ), vlSize( abuf ) );
      vlAutoFree( abuf );
    }
  }

  if( suc ){
    lua_pushnumber( s, 1 );
    lua_gettable( s, -2 );
    lua_remove( s, -2 );
    lua_pushboolean( s, 1 );
    lua_insert( s, 2 );
  }

  lua_settop( s, 3 );
  XML_ParserFree( p );
  vlFree( ep.rbuf );
  vlAutoFree( ep.wbuf );
  return 2;
}















static int pvlAddMenuWindow( lua_State* s ){
  vlLuaAssess( s, lua_gettop( s ) == 4 && pvlIsUserType( s, 1, Window ) && lua_isstring( s, 2 ) && lua_isnumber( s, 3 ) && lua_isnumber( s, 4 ), "Bad arguments to pvlAddMenuWindow." );
  osAddMenu( pvlUserToVoidp( s, 1 ), lua_tostring( s, 2 ), (u32)lua_tonumber( s, 3 ), (u32)lua_tonumber( s, 4 ) );
  return 0;
}
static int pvlRemoveMenuWindow( lua_State* s ){
  vlLuaAssess( s, lua_gettop( s ) == 2 && pvlIsUserType( s, 1, Window ) && lua_isnumber( s, 2 ), "Bad arguments to pvlRemoveMenuWindow." );
  osRemoveMenu( pvlUserToVoidp( s, 1 ), (u32)lua_tonumber( s, 2 ) );
  return 0;
}
static int pvlAddMenuBreakWindow( lua_State* s ){
  vlLuaAssess( s, lua_gettop( s ) == 3 && pvlIsUserType( s, 1, Window ) && lua_isnumber( s, 2 ) && lua_isnumber( s, 3 ), "Bad arguments to pvlAddMenuBreakWindow." );
  osAddMenu( pvlUserToVoidp( s, 1 ), "", (u32)lua_tonumber( s, 2 ), (u32)lua_tonumber( s, 3 ) );
  return 0;
}
static int pvlSetMenuItemState( lua_State* s ){
  vlMenuItemState mis = 0;
  const u8* sn;
  vlLuaAssess( s, lua_gettop( s ) == 3 && pvlIsUserType( s, 1, Window ) && lua_isnumber( s, 2 ) && lua_isstring( s, 3 ), "Bad arguments to pvlSetMenuItemStateWindow." );
  sn = lua_tostring( s, 3 );
  if( !strcmp( sn, "checked" ) )
    mis = checked;
  else if( !strcmp( sn, "unchecked" ) )
    mis = unchecked;
  else if( !strcmp( sn, "disabled" ) )
    mis = disabled;
  else
    luaL_error( s, "Unrecognized menu item state specified in pvlSetMenuItemState." );
  osSetMenuItemState( pvlUserToVoidp( s, 1 ), (u32)lua_tonumber( s, 2 ), mis );
  return 0;
}
static int pvlRaiseWindow( lua_State* s ){
  vlLuaAssess( s, lua_gettop( s ) == 1 && pvlIsUserType( s, 1, Window ), "Bad arguments to pvlRaiseWindow." );
  osRaiseWindow( pvlUserToVoidp( s, 1 ) );
  return 0;
}
static int pvlNewWindow( lua_State* s ){
  u32* dt, x, y, width, height;
  u8* name;
  const u8* wintype;
  void* ans;
  vlWindowStyle vs = app;
  vlLuaAssess( s, lua_gettop( s ) == 7 && lua_isnumber( s, 1 ) && lua_isnumber( s, 2 ) && lua_isnumber( s, 3 ) && lua_isnumber( s, 4 ) && lua_isstring( s, 5 ) && lua_isstring( s, 6 ) && lua_isfunction( s, 7 ), "Bad arguments in pvlNewWindow." );
  wintype = lua_tostring( s, 6 );
  if( !strcmp( wintype, "app" ) )
    vs = app;
  else if( !strcmp( wintype, "borderless" ) )
    vs = borderless;
  else if( !strcmp( wintype, "toolbox" ) )
    vs = toolbox;
  else
    vlDie( "Unrecognized window type in pvlNewWindow." );

  x = (u32)lua_tonumber( s, 1 ); y = (u32)lua_tonumber( s, 2 ); width = (u32)lua_tonumber( s, 3 ); height = (u32)lua_tonumber( s, 4 );
  {
    const u8* tnp = lua_tostring( s, 5 );
    u32 sz = strlen( tnp );
    name = vlMalloc( sz + 1 );
    memcpy( name, tnp, sz );
    name[ sz ] = '\0';
  }

  lua_pushvalue( s, 7 );
  pvlSetReg( s, "nw" );

  lua_settop( s, 0 );
  lua_newuserdata( s, pvlWindowTypeSize );
  lua_newtable( s );
  dt = (u32*)lua_touserdata( s, 1 );
  dt[ 0 ] = pvlWindowType;
  ans = osCreateWindow( x, y, width, height, name, vs );
  if( ans == NULL )
    vlDie( "Failed to create window in pvlNewWindow." );
  memcpy( dt + 1, &ans, sizeof( void* ) );
  vlFree( name );
  lua_pushvalue( s, 1 );
  lua_setfield( s, 2, "window" );

  pvlGetReg( s, "nw" );
  lua_setfield( s, 2, "callback" );

  lua_getfield( s, LUA_REGISTRYINDEX, "windows" );
  lua_pushlightuserdata( s, ans );
  lua_pushvalue( s, 2 );
  lua_settable( s, -3 );
  lua_settop( s, 1 );

  return 1;
}
static int pvlLoadResource( lua_State* s ){
  u32 sz;
  void* mem;
  vlLuaAssess( s, lua_gettop( s ) == 2 && lua_isnumber( s, 1 ) && lua_isstring( s, 2 ), "Bad arguments given to pvlLoadResource (should be a number and a string)" );
  if( osLoadResource( (u32)lua_tonumber( s, 1 ), &mem, &sz, lua_tostring( s, 2 ) ) )
    lua_pushlstring( s, mem, sz );
  else
    lua_pushnil( s );
  return 1;
}
static int pvlNewWidget( lua_State* s ){
  u32* dt, x, y, width, height;
  const u8* widtype;
  void* ans,* par;
  vlWidgetStyle vs = text;
  vlLuaAssess( s, lua_gettop( s ) == 7 && pvlIsUserType( s, 1, Window ) && lua_isnumber( s, 2 ) && lua_isnumber( s, 3 ) && lua_isnumber( s, 4 ) && lua_isnumber( s, 5 ) && lua_isstring( s, 6 ) && lua_isfunction( s, 7 ), "Bad arguments in pvlNewWidget." );
  par = pvlUserToVoidp( s, 1 );
  widtype = lua_tostring( s, 6 );
  if( !strcmp( widtype, "text" ) )
    vs = text;
  else if( !strcmp( widtype, "textNowrap" ) )
    vs = textNowrap;
  else if( !strcmp( widtype, "statusbar" ) )
    vs = statusbar;
  else
    vlDie( "Unrecognized widget type in pvlNewWidget." );

  x = (u32)lua_tonumber( s, 2 ); y = (u32)lua_tonumber( s, 3 ); width = (u32)lua_tonumber( s, 4 ); height = (u32)lua_tonumber( s, 5 );

  lua_pushvalue( s, 7 );
  pvlSetReg( s, "nw" );
  lua_settop( s, 0 );
  lua_newuserdata( s, pvlWidgetTypeSize );
  lua_newtable( s );
  dt = (u32*)lua_touserdata( s, 1 );
  dt[ 0 ] = pvlWidgetType;
  ans = osAddWidget( par, x, y, width, height, vs );
  if( ans == NULL )
    vlDie( "Failed to create widget in pvlNewWidget." );
  memcpy( dt + 1, &ans, sizeof( void* ) );
  lua_pushvalue( s, 1 );
  lua_setfield( s, 2, "widget" );

  pvlGetReg( s, "nw" );
  lua_setfield( s, 2, "callback" );

  lua_getfield( s, LUA_REGISTRYINDEX, "widgets" );
  lua_pushlightuserdata( s, ans );
  lua_pushvalue( s, 2 );
  lua_settable( s, -3 );
  lua_settop( s, 1 );

  return 1;
}
static int pvlIsVisibleWindow( lua_State* s ){
  vlLuaAssess( s, lua_gettop( s ) == 1 && pvlIsUserType( s, 1, Window ), "Bad arguments to pvlIsVisibleWindow." );
  lua_pushboolean( s, osIsVisibleWindow( pvlUserToVoidp( s, 1 ) ) );
  return 1;
}
static int pvlIsVisibleWidget( lua_State* s ){
  vlLuaAssess( s, lua_gettop( s ) == 1 && pvlIsUserType( s, 1, Widget ), "Bad arguments to pvlIsVisibleWidget." );
  lua_pushboolean( s, osIsVisibleWindow( pvlUserToVoidp( s, 1 ) ) );
  return 1;
}
static int pvlGetPosWindow( lua_State* s ){
  s32 x, y, w, h;
  vlLuaAssess( s, lua_gettop( s ) == 2 && pvlIsUserType( s, 1, Window ) && lua_isboolean( s, 2 ), "Bad arguments to pvlGetPosWindow." );
  osGetPosWindow( pvlUserToVoidp( s, 1 ), &x, &y, &w, &h, lua_toboolean( s, 2 ) );
  lua_pushnumber( s, x ); lua_pushnumber( s, y ); lua_pushnumber( s, w ); lua_pushnumber( s, h );
  return 4;
}
static int pvlGetPosWidget( lua_State* s ){
  s32 x, y, w, h;
  vlLuaAssess( s, lua_gettop( s ) == 1 && pvlIsUserType( s, 1, Widget ), "Bad arguments to pvlGetPosWidget." );
  osGetPosWidget( pvlUserToVoidp( s, 1 ), &x, &y, &w, &h );
  lua_pushnumber( s, x ); lua_pushnumber( s, y ); lua_pushnumber( s, w ); lua_pushnumber( s, h );
  return 4;
}
static int pvlSetPosWindow( lua_State* s ){
  vlLuaAssess( s, lua_gettop( s ) == 6 && pvlIsUserType( s, 1, Window ) && lua_isnumber( s, 2 ) && lua_isnumber( s, 3 ) && lua_isnumber( s, 4 ) && lua_isnumber( s, 5 ) && lua_isboolean( s, 6 ), "Bad arguments to pvlSetPosWindow." );
  osResizeWindow( pvlUserToVoidp( s, 1 ), (s32)lua_tonumber( s, 2 ), (s32)lua_tonumber( s, 3 ), (s32)lua_tonumber( s, 4 ), (s32)lua_tonumber( s, 5 ), lua_toboolean( s, 6 ) );
  return 0;
}
static int pvlSetPosWidget( lua_State* s ){
  vlLuaAssess( s, lua_gettop( s ) == 5 && pvlIsUserType( s, 1, Widget ) && lua_isnumber( s, 2 ) && lua_isnumber( s, 3 ) && lua_isnumber( s, 4 ) && lua_isnumber( s, 5 ), "Bad arguments to pvlSetPosWidget." );
  osResizeWindow( pvlUserToVoidp( s, 1 ), (s32)lua_tonumber( s, 2 ), (s32)lua_tonumber( s, 3 ), (s32)lua_tonumber( s, 4 ), (s32)lua_tonumber( s, 5 ), 0 );
  return 0;
}
static int pvlDestroyWindow( lua_State* s ){
  void* ovp = pvlUserToVoidp( s, 1 );
  vlAssess( lua_gettop( s ) == 1 && pvlIsUserType( s, 1, Window ), "Bad data in pvlDestroyWindow." );
  osDestroyWindow( pvlUserToVoidp( s, 1 ) );
  lua_getfield( s, LUA_REGISTRYINDEX, "windows" );
  lua_pushlightuserdata( s, ovp );
  lua_pushnil( s );
  lua_settable( s, -3 );
  return 0;
}
static int pvlDestroyWidget( lua_State* s ){
  void* ovp = pvlUserToVoidp( s, 1 );
  vlAssess( lua_gettop( s ) == 1 && pvlIsUserType( s, 1, Widget ), "Bad data in pvlDestroyWidget." );
  osDestroyWindow( pvlUserToVoidp( s, 1 ) );
  lua_getfield( s, LUA_REGISTRYINDEX, "widgets" );
  lua_pushlightuserdata( s, ovp );
  lua_pushnil( s );
  lua_settable( s, -3 );
  return 0;
}
static int pvlType( lua_State* s ){
  vlLuaAssess( s, lua_gettop( s ) <= 1, "Wrong number of arguments given to pvlType (should be 0 or 1)." );
  if( !lua_gettop( s ) )
    lua_pushstring( s, "none" );
  else{
    u8* tp = ppvlGetType( s, 1 );
    lua_pushstring( s, tp );
    vlFree( tp );
  }
  return 1;
}
static int pvlSetmetatable( lua_State* s ){
  vlLuaAssess( s, lua_gettop( s ) == 2 && ( lua_isuserdata( s, 1 ) || lua_istable( s, 1 ) ) && ( lua_istable( s, 2 ) || lua_isnil( s, 2 ) ), "Incorrect arguments to pvlSetmetatable." );
  lua_setmetatable( s, 1 );
  return 0;
}
static int pvlWindowStatus( lua_State* s ){
  lua_pushboolean( s, osWindowStatus( lua_touserdata( s, 1 ) ) ); 
  return 1;
}
static int pvlLoop( lua_State* s ){
  osLoop();
  return 0;
}
static int pvlWaitMessage( lua_State* s ){
  vlWaitMessage();
  return 0;
}
static int pvlIsIdle( lua_State* s ){
  lua_pushboolean( s, vlIsIdle() );
  return 1;
}

static int pvlThread( lua_State* s ){
  u32 nfo = vlAutoMalloc( 0 );
  vlLuaAssess( s, lua_gettop( s ) == 1 && lua_isstring( s, 1 ), "Incorrect arguments to pvlThread (should be 1 string)" ); 
  lua_checkstack( s, 10 );
  vlAppendInt( nfo, osGetThreadId() );
  {
    u32 inh = 0;
    lua_getglobal( s, "ovel" );
    if( lua_istable( s, -1 ) ){
      lua_getfield( s, -1, "inheritance" );
      if( lua_isstring( s, -1 ) ){
        inh = vlAutoMalloc( 0 );
        vlAppendString( inh, lua_tostring( s, -1 ) );
      } 
      lua_settop( s, 1 );
    }
    if( inh ){
      vlAppend( nfo, vlMem( inh ), vlSize( inh ) );
      vlAutoFree( inh );
    } else
      vlAppend( nfo, "", 1 );
  }
  {
    const u8* str = lua_tostring( s, 1 );
    vlAppend( nfo, str, strlen( str ) + 1 );
  }
  {
    vlGlobal* vlg = vlGetGlobal();
    vlReleaseGlobal();
    vlItcPost( vlg->dispatchThread, vlMem( nfo ), vlSize( nfo ) );
  }
  vlAutoFree( nfo );
  return 0;
} 
static int pvlNumThreads( lua_State* s ){
  // Subtract one because we don't want to know about the dispatch thread.
  lua_pushnumber( s, vlNumThreads() - 1 );
  return 1;
}
static int pvlIsThreadRunning( lua_State* s ){
  vlLuaAssess( s, lua_gettop( s ) == 1 && lua_isnumber( s, 1 ), "Wrong argmuents to pvlIsThreadRunning (should be 1 number)." );
  lua_pushboolean( s, osIsThreadRunning( (u32)lua_tonumber( s, 1 ) ) );
  return 1;
}
static int pvlSetGlobal( lua_State* s ){
  u32 i;
  vlLuaAssess( s, lua_gettop( s ) == 2, "Wrong number of arguments to pvlSetGlobal (should be 2)." );
  vlLuaAssess( s, lua_isstring( s, 1 ) && ( lua_isstring( s, 2 ) || lua_isnil( s, 2 ) || lua_islightuserdata( s, 2 ) ), "Wrong type of arguments to pvlSetGlobal (the key should be a string or number and the value should be a string or number or light user data or nil)." );
  {
    lua_State* vlgl = vlGetGlobal()->luaGlobal;
    lua_checkstack( vlgl, 3 );
    lua_getfield( vlgl, LUA_REGISTRYINDEX, "vlglobals" );
    if( lua_isnil( vlgl, -1 ) ){
      lua_pop( vlgl, 1 );
      lua_createtable( vlgl, 0, 0 );
    }
    for( i = 1; i <= 2; ++i ){
      if( lua_isnil( s, i ) )
        lua_pushnil( vlgl );
      else if( lua_type( s, i ) == LUA_TNUMBER )
        lua_pushnumber( vlgl, lua_tonumber( s, i ) );
      else if( lua_type( s, i ) == LUA_TLIGHTUSERDATA )
        lua_pushlightuserdata( vlgl, lua_touserdata( s, i ) );
      else{
        size_t sz;
        const u8* buf = lua_tolstring( s, i, &sz );
        lua_pushlstring( vlgl, buf, sz ); 
      }
    }
    lua_settable( vlgl, -3 );
    lua_setfield( vlgl, LUA_REGISTRYINDEX, "vlglobals" );
    vlReleaseGlobal();
  }
  return 0;
}
static int pvlGetGlobal( lua_State* s ){
  vlLuaAssess( s, lua_gettop( s ) == 1 && lua_isstring( s, 1 ), "Bad arguments to pvlGetGlobal (should be 1 number or string)." );
  {
    lua_State* vlgl = vlGetGlobal()->luaGlobal;
    lua_checkstack( vlgl, 2 );
    lua_getfield( vlgl, LUA_REGISTRYINDEX, "vlglobals" );
    if( lua_isnil( vlgl, -1 ) ){
      lua_pop( vlgl, 1 );
      lua_pushnil( s );
      vlReleaseGlobal();
      return 1;
    }
    if( lua_type( s, 1 ) == LUA_TNUMBER )
      lua_pushnumber( vlgl, lua_tonumber( s, 1 ) );
    else{
      size_t sz;
      const u8* buf = lua_tolstring( s, 1, &sz );
      lua_pushlstring( vlgl, buf, sz ); 
    }
    lua_gettable( vlgl, -2 );
    if( lua_isnil( vlgl, -1 ) )
      lua_pushnil( s );
    else if( lua_type( vlgl, -1 ) == LUA_TNUMBER )
      lua_pushnumber( s, lua_tonumber( vlgl, -1 ) );
    else if( lua_type( vlgl, -1 ) == LUA_TLIGHTUSERDATA )
      lua_pushlightuserdata( s, lua_touserdata( vlgl, -1 ) );
    else{
      size_t sz;
      const u8* buf = lua_tolstring( vlgl, -1, &sz );
      lua_pushlstring( s, buf, sz ); 
    } 
    lua_pop( vlgl, 2 );
    vlReleaseGlobal();
  }
  return 1;
}
static int pvlGetAllGlobals( lua_State* s ){
  vlLuaAssess( s, lua_gettop( s ) == 0, "Bad arguments to pvlGetAllGlobals (should be no arguments)." );
  {
    lua_State* vlgl = vlGetGlobal()->luaGlobal;
    lua_checkstack( vlgl, 6 );
    lua_getfield( vlgl, LUA_REGISTRYINDEX, "vlglobals" );
    lua_newtable( s );
    if( lua_isnil( vlgl, -1 ) ){
      lua_pop( vlgl, 1 );
      vlReleaseGlobal();
      return 1;
    }

    lua_pushnil( vlgl );  /* first key */
    while( lua_next( vlgl, -2 ) ){
      /* uses 'key' (at index -2) and 'value' (at index -1) */
      if( lua_isnil( vlgl, -2 ) )
        lua_pushnil( s );
      else if( lua_type( vlgl, -2 ) == LUA_TNUMBER )
        lua_pushnumber( s, lua_tonumber( vlgl, -2 ) );
      else if( lua_type( vlgl, -2 ) == LUA_TLIGHTUSERDATA )
        lua_pushlightuserdata( s, lua_touserdata( vlgl, -2 ) );
      else{
        size_t sz;
        const u8* buf = lua_tolstring( vlgl, -2, &sz );
        lua_pushlstring( s, buf, sz ); 
      }
      if( lua_isnil( vlgl, -1 ) )
        lua_pushnil( s );
      else if( lua_type( vlgl, -1 ) == LUA_TNUMBER )
        lua_pushnumber( s, lua_tonumber( vlgl, -1 ) );
      else if( lua_type( vlgl, -1 ) == LUA_TLIGHTUSERDATA )
        lua_pushlightuserdata( s, lua_touserdata( vlgl, -1 ) );
      else{
        size_t sz;
        const u8* buf = lua_tolstring( vlgl, -1, &sz );
        lua_pushlstring( s, buf, sz ); 
      }
      lua_settable( s, -3 );
      lua_pop( vlgl, 1 );
    } 
    lua_pop( vlgl, 1 );
    vlReleaseGlobal();
  }
  return 1;
}
// BCJBCJ YOU MUST CALL THIS BEFORE USING pvl[SG]etGlobalNonAtomic TO PREVENT A DEADLOCK!!!!!
static int pvlLockGlobal( lua_State* s ){
  osGetTLS()->pvlGlobal = vlGetGlobal();
  return 0;
}
static int pvlSetGlobalNonAtomic( lua_State* s ){
  u32 i;
  vlLuaAssess( s, lua_gettop( s ) == 2, "Wrong number of arguments to pvlSetGlobal (should be 2)." );
  vlLuaAssess( s, lua_isstring( s, 1 ) && ( lua_isstring( s, 2 ) || lua_isnil( s, 2 ) || lua_islightuserdata( s, 2 ) ), "Wrong type of arguments to pvlSetGlobal (the key should be a string or number and the value should be a string or number or light user data or nil)." );
  {
    vlGlobal* vlg = osGetTLS()->pvlGlobal;
    lua_State* vlgl;
    vlLuaAssess( s, vlg != NULL, "NULL global pointer in pvlSetGlobalNonAtomic. This is probably because pvlLockGlobal was not called." );
    vlgl = vlg->luaGlobal;
    lua_checkstack( vlgl, 3 );
    lua_getfield( vlgl, LUA_REGISTRYINDEX, "vlglobals" );
    if( lua_isnil( vlgl, -1 ) ){
      lua_pop( vlgl, 1 );
      lua_createtable( vlgl, 0, 0 );
    }
    for( i = 1; i <= 2; ++i ){
      if( lua_isnil( s, i ) )
        lua_pushnil( vlgl );
      else if( lua_type( s, i ) == LUA_TNUMBER )
        lua_pushnumber( vlgl, lua_tonumber( s, i ) );
      else if( lua_type( s, i ) == LUA_TLIGHTUSERDATA )
        lua_pushlightuserdata( vlgl, lua_touserdata( s, i ) );
      else{
        size_t sz;
        const u8* buf = lua_tolstring( s, i, &sz );
        lua_pushlstring( vlgl, buf, sz ); 
      }
    }
    lua_settable( vlgl, -3 );
    lua_setfield( vlgl, LUA_REGISTRYINDEX, "vlglobals" );
    //vlReleaseGlobal();
  }
  return 0;
}
static int pvlGetGlobalNonAtomic( lua_State* s ){
  vlLuaAssess( s, lua_gettop( s ) == 1 && lua_isstring( s, 1 ), "Bad arguments to pvlGetGlobal (should be 1 number or string)" );
  {
    vlGlobal* vlg = osGetTLS()->pvlGlobal;
    lua_State* vlgl;
    vlLuaAssess( s, vlg != NULL, "NULL global pointer in pvlGetGlobalNonAtomic. This is probably because pvlLockGlobal was not called." );
    vlgl = vlg->luaGlobal;
    lua_checkstack( vlgl, 2 );
    lua_getfield( vlgl, LUA_REGISTRYINDEX, "vlglobals" );
    if( lua_isnil( vlgl, -1 ) ){
      lua_pop( vlgl, 1 );
      lua_pushnil( s );
      //vlReleaseGlobal();
      return 1;
    }
    if( lua_type( s, 1 ) == LUA_TNUMBER )
      lua_pushnumber( vlgl, lua_tonumber( s, 1 ) );
    else{
      size_t sz;
      const u8* buf = lua_tolstring( s, 1, &sz );
      lua_pushlstring( vlgl, buf, sz ); 
    }
    lua_gettable( vlgl, -2 );
    if( lua_isnil( vlgl, -1 ) )
      lua_pushnil( s );
    else if( lua_type( vlgl, -1 ) == LUA_TNUMBER )
      lua_pushnumber( s, lua_tonumber( vlgl, -1 ) );
    else if( lua_type( vlgl, -1 ) == LUA_TLIGHTUSERDATA )
      lua_pushlightuserdata( s, lua_touserdata( vlgl, -1 ) );
    else{
      size_t sz;
      const u8* buf = lua_tolstring( vlgl, -1, &sz );
      lua_pushlstring( s, buf, sz ); 
    } 
    lua_pop( vlgl, 2 );
    //vlReleaseGlobal();
  }
  return 1;
}
// BCJBCJ YOU MUST CALL THIS AFTER USING pvl[SG]etGlobalNonAtomic TO PREVENT A DEADLOCK!!!!!
static int pvlReleaseGlobal( lua_State* s ){
  osGetTLS()->pvlGlobal = NULL;
  vlReleaseGlobal();
  return 0;
}
static void pvlVlPrintHelper( lua_State* s, int err ){
  u32 i;
  u32 top = lua_gettop( s );
  u32 ssz;
  const u8* str;
  for( i = 1; i <= top; ++i ){
    lua_getglobal( s, "tostring" );
    lua_pushvalue( s, i );
    lua_call( s, 1, 1 );
    if( i != 1 )
      vlPrint( "\t", 1, err );
    str = lua_tolstring( s, -1, &ssz );
    vlPrint( str, ssz, err );
    lua_pop( s, 1 );
  }
}
static int pvlPrint( lua_State* s ){
  pvlVlPrintHelper( s, 0 );
  return 0;
}
static int pvlGetFont( lua_State* s ){
  u8* f;
  vlLuaAssess( s, lua_gettop( s ) == 1 && lua_isstring( s, 1 ), "Bad arguments to pvlGetFont(should be 1 string)." )
    f = osGetFont( lua_tostring( s, 1 ) );
  if( f != NULL ){
    lua_pushstring( s, f );
    vlFree( f );
  }else
    lua_pushnil( s );
  return 1;
}
static int pvlErrorPrint( lua_State* s ){
  pvlVlPrintHelper( s, 1 );
  return 0;
}
static int pvlPrintBuffer( lua_State* s ){
  vlState* vltls = osGetTLS();
  lua_pushlstring( s, vlMem( vltls->pvlOBuff ), vlSize( vltls->pvlOBuff ) );
  return 1;
}
static int pvlErrorPrintBuffer( lua_State* s ){
  vlState* vltls = osGetTLS();
  u32 sz = vlSize( vltls->pvlEBuff );
  if( sz && ( (u8*)vlMem( vltls->pvlEBuff ) )[ vlSize( vltls->pvlEBuff ) - 1 ] == 0 )
    --sz;
  lua_pushlstring( s, vlMem( vltls->pvlEBuff ), sz );
  return 1;
}
static int pvlSleep( lua_State* s ){
  if( lua_gettop( s ) >= 1 && lua_isnumber( s, 1 ) )
    vlSleep( (u32)lua_tonumber( s, 1 ) );
  return 0;
}
static int pvlItcPost( lua_State* s ){
  u32 msz;
  const u8* msg;
  vlLuaAssess( s, lua_gettop( s ) == 2, "Wrong number of arguments to pvlItcPost (should be 2)." );
  vlLuaAssess( s, lua_isstring( s, 1 ) && lua_isnumber( s, 2 ), "Wrong type of arguments to pvlItcPost (should be a string and a number)." );
  msg = lua_tolstring( s, 1, &msz );
  vlItcPost( (u32)lua_tonumber( s, 2 ), msg, msz );
  return 0;
}
static int pvlItcRetrieve( lua_State* s ){
  u32 t = osGetThreadId();
  u32 ans;
  vlLuaAssess( s, lua_gettop( s ) <= 1, "Wrong number of arguments to pvlItcRetrieve (should be 0 or 1)." );
  if( lua_gettop( s ) ){
    vlLuaAssess( s, lua_isnumber( s, 1 ), "Wrong type of argument to pvlItcRetrieve (should be a number)." );
    t = (u32)lua_tonumber( s, 1 );
  }
  ans = vlItcRetrieve( t );
  if( ans )
    lua_pushlstring( s, vlMem( ans ), vlSize( ans ) );
  else
    lua_pushnil( s );
  return 1;
} 
static int pvlFileDialog( lua_State* s ){
  vlFileDialogOptions opts = 0;
  u32 fn;
  vlLuaAssess( s, lua_gettop( s ) <= 1, "Wrong number of arguments to pvlFileDialog (should be 0 or 1)." );
  if( lua_gettop( s ) == 1 ){
    vlLuaAssess( s, lua_istable( s, 1 ), "Wrong type of argument to pvlFileDialog (should be a table)." );  
    lua_getfield( s, 1, "save" );
    if( lua_toboolean( s, -1 ) )
      opts += save;
    lua_pop( s, 1 );
  }
  fn = osFileDialog( opts );
  if( fn ){
    lua_pushlstring( s, vlMem( fn ), vlSize( fn ) );
    vlAutoFree( fn );
  }else
    lua_pushnil( s );
  return 1;
}
static int pvlCompress( lua_State* s ){
  u64 dsz64;
  uLongf dsz;
  u32 ssz;
  const u8* src;
  u8* dst;
  u32 q = 9;
  vlLuaAssess( s, ( ( lua_gettop( s ) + 1 ) / 2 == 1 && lua_isstring( s, 1 ) ) && ( lua_gettop( s ) == 1 || lua_isnumber( s, 2 ) ), "Bad arguments to pvlCompress (should be a string and an optional number between 1 and 9 inclusive)." ); 
  if( lua_gettop( s ) == 2 ){
    q = (u32)lua_tonumber( s, 2 );
    vlLuaAssess( s, q >= 1 && q <= 9, "Bad quality level given to pvlCompress (must between 1 and 9 inclusive)." );
  }
  src = lua_tolstring( s, 1, &ssz );
  dsz64 = ( ( ( (u64)ssz ) * 1002 ) / 1000 ) + 12;
  vlLuaAssess( s, dsz64 + 4 <= 4294967295, "Attempt to compress to large a string." );
  dsz = (u32)dsz64;
  dst = vlMalloc( dsz + 4 );
  *( (u32*)dst ) = ssz;
  if( compress2( dst + 4, &dsz, src, ssz, q ) != Z_OK ){
    vlFree( dst );
    vlDie( "Compression failed." );
  }
  lua_pushlstring( s, dst, dsz + 4 );
  vlFree( dst );
  return 1;
}
static int pvlUncompress( lua_State* s ){
  u32 ssz;
  const u8* src;
  u32 ans;
  vlLuaAssess( s, lua_gettop( s ) == 1 && lua_isstring( s, 1 ), "Bad arguments to pvlUncompress (should be 1 string)." ); 
  src = lua_tolstring( s, 1, &ssz );
  ans = vlUncompress( src, ssz );
  vlLuaAssess( s, ans, "Uncompression failed." );
  lua_pushlstring( s, vlMem( ans ), vlSize( ans ) );
  vlAutoFree( ans );
  return 1;
}
typedef struct{
  u32 dst;
  const u8* str;
  u32 ssz;
} ppvlLevStruct;
static int ppvlLevSort( const void* i1, const void* i2 ){
  return ( (ppvlLevStruct*)i1 )->dst - ( (ppvlLevStruct*)i2 )->dst;
}
static void ppvlLevHelp( const u8* s1, u32 sz, ppvlLevStruct* ls, u32 num ){
  u32** abuf;
  u32* buf;
  u32 i, msz = 0, szp, mszp;
  for( i = 0; i < num; ++i )
    if( ls[ i ].ssz > msz )
      msz = ls[ i ].ssz;
  szp = sz + 1;
  mszp = msz + 1;
  buf = vlMalloc( sizeof( u32 ) * szp * mszp );
  abuf = vlMalloc( sizeof( u32* ) * szp );
  for( i = 0; i < szp; ++i )
    abuf[ i ] = buf + ( i * mszp );
  for( i = 0; i < num; ++i ){
    u32 cszp = ls[ i ].ssz + 1;
    u32 j, k, t;
    for( j = 0; j < szp; ++j )
      abuf[ j ][ 0 ] = j;
    for( j = 0; j < cszp; ++j )
      abuf[ 0 ][ j ] = j;
    for( j = 1; j < szp; ++j )
      for( k = 1; k < cszp; ++k ){
        abuf[ j ][ k ] = abuf[ j - 1 ][ k ] + 1;
        t = abuf[ j ][ k - 1 ] + 1;
        if( t < abuf[ j ][ k ] )
          abuf[ j ][ k ] = t;
        t = abuf[ j - 1 ][ k - 1 ] + ( ( s1[ j - 1 ] == ( ls[ i ].str )[ k - 1 ] )? 0 : 1 );
        if( t < abuf[ j ][ k ] )
          abuf[ j ][ k ] = t;
      }
      ls[ i ].dst = abuf[ sz ][ ls[ i ].ssz ];
  }
  vlFree( buf );
  vlFree( abuf ); 
}
static int pvlLevenshtein( lua_State* s ){
  ppvlLevStruct ls;
  const u8* s1;
  u32 sz1;
  vlLuaAssess( s, lua_gettop( s ) == 2 && lua_isstring( s, 1 ) && lua_isstring( s, 2 ), "Bad arguments to pvlLevenshtein (should be two strings." ); 
  s1 = lua_tolstring( s, 1, &sz1 );
  ls.str = lua_tolstring( s, 2, &ls.ssz );
  ppvlLevHelp( s1, sz1, &ls, 1 );
  lua_pushnumber( s, (lua_Number)( ls.dst ) );
  return 1;
}
static int pvlClosestStrings( lua_State* s ){
  const u8* s1;
  u32 sz1;
  u32 top = lua_gettop( s );
  vlLuaAssess( s, top / 2 == 1 && lua_isstring( s, 1 ) && lua_istable( s, 2 ) && lua_isnumber( s, 3 ), "Bad arguments to pvlClosestStrings (should be a string, a set of strings and optionally a number)." ); 
  s1 = lua_tolstring( s, 1, &sz1 );
  {
    u32 asz, num = 0, i;
    ppvlLevStruct ls;
    u32 lss = vlAutoMalloc( 0 );
    lua_checkstack( s, 6 );

    lua_pushnil( s );
    while( lua_next( s, 2 ) ){
      if( lua_type( s, -2 ) == LUA_TSTRING ){
        ls.str = lua_tolstring( s, -2, &ls.ssz );
        vlAppend( lss, &ls, sizeof( ppvlLevStruct ) );
        ++num;
      }
      lua_pop( s, 1 );
    }
    ppvlLevHelp( s1, sz1, (ppvlLevStruct*)vlMem( lss ), num );
    qsort( vlMem( lss ), num, sizeof( ppvlLevStruct ), ppvlLevSort );
    asz = ( top == 3 ) ? (u32)lua_tonumber( s, 3 ) : num;
    if( num < asz )
      asz = num;
    lua_newtable( s );
    for( i = 0; i < asz; ++i ){
      ppvlLevStruct* lsp = (ppvlLevStruct*)vlMem( lss ) + i;
      lua_pushnumber( s, (lua_Number)( i + 1 ) );
      lua_pushlstring( s, lsp->str, lsp->ssz );
      lua_settable( s, -3 );
    }
    vlAutoFree( lss );
  }
  return 1;
}
static int pvlIsConsoleOutAttached( lua_State* s ){
  vlLuaAssess( s, !lua_gettop( s ), "Bad arguments to pvlIsConsoleOutAttached (should be no argumnents)." );
  lua_pushboolean( s, vlIsHandleAttached( 0 ) );
  return 1;
}
static int pvlIsConsoleErrorAttached( lua_State* s ){
  vlLuaAssess( s, !lua_gettop( s ), "Bad arguments to pvlIsConsoleErrorAttached (should be no argumnents)." );
  lua_pushboolean( s, vlIsHandleAttached( 1 ) );
  return 1;
}
static int pvlGetPasteBuffer( lua_State* s ){
  u32 b = osGetPasteBuffer();
  if( b ){
    lua_pushlstring( s, vlMem( b ), vlSize( b ) );
    vlAutoFree( b );
  }
  else
    lua_pushstring( s, "" );
  return 1;
}
static int pvlSetPasteBuffer( lua_State* s ){
  const u8* str;
  u32 sz;
  vlLuaAssess( s, lua_gettop( s ) == 1 && lua_isstring( s, 1 ), "Bad arguments to pvlSetPasteBuffer (should be 1 string)." );
  str = lua_tolstring( s, 1, &sz );
  osSetPasteBuffer( str, sz );
  return 0;
}
void ppvlUTFErrorHandler( const u8* emsg ){
  lua_State* s = osGetTLS()->ls;
  int err = 0;
  u32 tp = lua_gettop( s );
  lua_checkstack( s, 10 );
  lua_getglobal( s, "ovel" );
  if( lua_istable( s, -1 ) ){
    lua_getfield( s, -1, "UTFErrorHandler" );
    if( lua_isfunction( s, -1 ) ){
      lua_pushstring( s, emsg );
      lua_call( s, 1, 0 );
    } else
      err = 1;
  } else
    err = 1;
  lua_settop( s, tp );
  if( err )
    vlDie( emsg );
}
static int pvlUTF8To16( lua_State* s ){
  int bom = 0, le = 0;
  u32 ssz;
  const u8* str;
  u32 ans;
  vlLuaAssess( s, lua_gettop( s ) >= 1 && lua_isstring( s, 1 ) && ( lua_gettop( s ) == 1 || ( lua_gettop( s ) == 2 && lua_isboolean( s, 2 ) ) ||  ( lua_gettop( s ) == 3 && lua_isboolean( s, 2 ) && lua_isboolean( s, 3 ) ) ) , "Bad arguments to pvlUTF8To16 (should be 1 string and up to two booleans)." );
  if( lua_gettop( s ) >= 2 )
    le = lua_toboolean( s, 2 );
  if( lua_gettop( s ) == 3 )
    bom = lua_toboolean( s, 3 );
  str = lua_tolstring( s, 1, &ssz );
  ans = vlUTF8ToUTF16( str, ssz, ppvlUTFErrorHandler, bom, le );
  lua_pushlstring( s, vlMem( ans ), vlSize( ans ) );
  vlAutoFree( ans );
  return 1;
}
void ppvlUTFTo32helper( u32 cp, u32 data ){
  lua_State* s = *( (lua_State**)( vlMem( data ) ) );
  lua_pushnumber( s, lua_objlen( s, -1 ) + 1 );
  lua_pushnumber( s, (lua_Number)cp );
  lua_settable( s, -3 );
}
static int pvlUTF8To32( lua_State* s ){
  u32 ssz;
  const u8* str;
  vlLuaAssess( s, lua_gettop( s ) == 1 && lua_isstring( s, 1 ), "Bad arguments to pvlUTF8To32 (should be a string)." );
  str = lua_tolstring( s, 1, &ssz );
  { 
    u32 ans = vlAutoMalloc( sizeof( lua_State* ) );
    lua_checkstack( s, 20 );
    *( (lua_State**)( vlMem( ans ) ) ) = s;
    lua_newtable( s );
    vlUTF8Meta( str, ssz, ans, ppvlUTFTo32helper, ppvlUTFErrorHandler );
    vlAutoFree( ans );
  }
  return 1;
}
static int pvlUTF16To8( lua_State* s ){
  int dle = 0;
  u32 ssz;
  const u8* str;
  u32 ans;
  vlLuaAssess( s, lua_gettop( s ) >= 1 && lua_isstring( s, 1 ) && ( lua_gettop( s ) == 1 || ( lua_gettop( s ) == 2 && lua_isboolean( s, 2 ) ) ) , "Bad arguments to pvlUTF16To8 (should be 1 string and an optional boolean)." );
  if( lua_gettop( s ) == 2 )
    dle = lua_toboolean( s, 2 );
  str = lua_tolstring( s, 1, &ssz );
  ans = vlUTF16ToUTF8( str, ssz, dle, ppvlUTFErrorHandler );
  lua_pushlstring( s, vlMem( ans ), vlSize( ans ) );
  vlAutoFree( ans );
  return 1;
}
static int pvlUTF16To32( lua_State* s ){
  int dle = 0;
  u32 ssz;
  const u8* str;
  vlLuaAssess( s, lua_gettop( s ) >= 1 && lua_isstring( s, 1 ) && ( lua_gettop( s ) == 1 || ( lua_gettop( s ) == 2 && lua_isboolean( s, 2 ) ) ) , "Bad arguments to pvlUTF16To32 (should be 1 string and an optional boolean)." );
  if( lua_gettop( s ) == 2 )
    dle = lua_toboolean( s, 2 );
  str = lua_tolstring( s, 1, &ssz );
  { 
    u32 ans = vlAutoMalloc( sizeof( lua_State* ) );
    lua_checkstack( s, 20 );
    *( (lua_State**)( vlMem( ans ) ) ) = s;
    lua_newtable( s );
    vlUTF16Meta( str, ssz, ans, dle, ppvlUTFTo32helper, ppvlUTFErrorHandler );
    vlAutoFree( ans );
  }
  return 1;
}
static int pvlUTF32To8( lua_State* s ){
  u32 cp;
  lua_checkstack( s, 10 );
  vlLuaAssess( s, lua_gettop( s ) == 1 && lua_isnumber( s, 1 ), "Bad arguments to pvlUTF32To8 (should be 1 number)." );
  cp = (u32)lua_tonumber( s, 1 );
  if( cp >= 0xD800 && cp <= 0xDBFF ){
    ppvlUTFErrorHandler( "Invalid unicode code point in pvlUTF32To8 in the range 0xD800-0xDFFF." );
    lua_pushlstring( s, "", 0 );
    return 1;
  }else if( cp > 0x0010FFFF ){
    ppvlUTFErrorHandler( "Invalid unicode code point in pvlUTF32To8 greater than 0x0010FFFF." );
    lua_pushlstring( s, "", 0 );
    return 1;
  }
  {
    u32 cpp = vlUTF32ToUTF8( cp );
    u32 ans = vlAutoMalloc( 0 );
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
    lua_pushlstring( s, vlMem( ans ), vlSize( ans ) );
    vlAutoFree( ans );
  }  
  return 1;
}
static int pvlUTF32To16( lua_State* s ){
  u32 cp;
  u32 le = 0;
  lua_checkstack( s, 10 );
  vlLuaAssess( s, lua_gettop( s ) >= 1 && lua_isnumber( s, 1 ) && ( lua_gettop( s ) == 1 || ( lua_gettop( s ) == 2 && lua_isboolean( s, 2 ) ) ), "Bad arguments to pvlUTF32To16 (should be 1 number)." );
  cp = (u32)lua_tonumber( s, 1 );
  if( lua_gettop( s ) == 2 )
    le = lua_toboolean( s, 2 );
  if( cp >= 0xD800 && cp <= 0xDBFF ){
    ppvlUTFErrorHandler( "Invalid unicode code point in pvlUTF32To16 in the range 0xD800-0xDFFF." );
    lua_pushlstring( s, "", 0 );
    return 1;
  }else if( cp > 0x0010FFFF ){
    ppvlUTFErrorHandler( "Invalid unicode code point in pvlUTF32To16 greater than 0x0010FFFF." );
    lua_pushlstring( s, "", 0 );
    return 1;
  }
  {
    u32 ans = vlAutoMalloc( 0 );
    u32 cpp = vlUTF32ToUTF16( cp );
    cpp = ( vlSwab16( ( cpp >> 16 ), !le ) << 16 ) + vlSwab16( cpp & 0x0000FFFF, !le );
    if( cpp & 0xFFFF0000 )
      vlAppend( ans, &cpp, 4 );
    else
      vlAppend( ans, &cpp, 2 );
    lua_pushlstring( s, vlMem( ans ), vlSize( ans ) );
    vlAutoFree( ans );
  }  
  return 1;
}
static int pvlUTF8BytePosition( lua_State* s ){
  u32 cp;
  u32 ssz;
  const u8* str;
  vlLuaAssess( s, lua_gettop( s ) == 2 && lua_isstring( s, 1 ) && lua_isnumber( s, 2 ), "Bad arguments to pvlUTF8BytePosition (should be a string and a number)." );
  str = lua_tolstring( s, 1, &ssz );
  cp = (u32)lua_tonumber( s, 2 );
  lua_pushnumber( s, vlUTF8BytePos( str, ssz, cp ) );
  return 1;
}
static int pvlUTF8CharPosition( lua_State* s ){
  u32 bp;
  u32 ssz;
  const u8* str;
  vlLuaAssess( s, lua_gettop( s ) == 2 && lua_isstring( s, 1 ) && lua_isnumber( s, 2 ), "Bad arguments to pvlUTF8CharPosition (should be a string and a number)." );
  str = lua_tolstring( s, 1, &ssz );
  bp = (u32)lua_tonumber( s, 2 );
  lua_pushnumber( s, vlUTF8CharPos( str, ssz, bp ) );
  return 1;
}
static int pvlAsciiEncode( lua_State* s ){
  static u8 pvlAsciiEncodeCodes[] = { 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 49, 50, 51, 52, 53, 54, 55, 56, 57, 48, 33, 64, 35, 36, 37, 94, 38, 42, 40, 41, 45, 95, 61, 43, 91, 93, 123, 125, 124, 59, 39, 58, 44, 46, 60, 62, 63, 96, 126 };
  u32 sz;
  const u8* str;
  vlLuaAssess( s, lua_gettop( s ) == 1 && lua_isstring( s, 1 ), "Bad arguments to pvlAsciiEncode (should be a string)." );
  str = lua_tolstring( s, 1, &sz );
  if( ( ( (u64)sz ) * 5 + 20 ) / 4 > ( (u64)4294967295 ) )
    vlDie( "A string requiring a representation over 4 gigabytes(!) was supplied to pvlAsciiEncode." );
  if( !sz ){
    lua_pushstring( s, "" );
    return 1;
  } else {
    u8* buf;
    u32 cnt = ( sz + 3 ) / 4;
    u8 mod = 3 - ( (u8)( ( sz + 3 ) % 4 ) );
    u8* tp;
    u32* sp;
    u8* dp;
    buf = vlMalloc( ( cnt + 1 ) * 5 + 2 );
    tp = vlMalloc( sz + 5 );
    memset( tp + sz, 0, 5 );
    memcpy( tp, str, sz );
    sp = (u32*)tp;
    dp = buf;

    while( cnt ){
      u32 v = *sp++;
      u8 i;
      for( i = 0; i < 5; ++i ){
        *dp++ = pvlAsciiEncodeCodes[ v % 85 ];
        v /= 85;
      }
      --cnt;
    }
    *( dp - mod ) = '\0';

    lua_pushstring( s, buf );
    vlFree( buf );
    vlFree( tp );
  }

  return 1;
}
static int pvlHexAsciiEncode( lua_State* s ){
  u32 sz;
  const u8* str;
  vlLuaAssess( s, lua_gettop( s ) == 1 && lua_isstring( s, 1 ), "Bad arguments to pvlHexAsciiEncode (should be a string)." );
  str = lua_tolstring( s, 1, &sz );
  if( ( (u64)sz ) * 2 > ( (u64)4294967295 ) )
    vlDie( "A string requiring a representation over 4 gigabytes(!) was supplied to pvlHexAsciiEncode." );
  if( !sz ){
    lua_pushstring( s, "" );
    return 1;
  } else {
    u8* buf;
    const u8* sp = str;
    u8* dp;
    buf = vlMalloc( sz * 2 + 1);
    buf[ sz * 2 ] = '\0';
    dp = buf;

    while( sz ){
      u8 ln = ( *sp & 15 );
      u8 hn = ( *( sp++ ) >> 4 );
      *( dp++ ) = ( hn < 10 )?( hn + '0' ):( hn + ( 'A' - 10 ) );
      *( dp++ ) = ( ln < 10 )?( ln + '0' ):( ln + ( 'A' - 10 ) );
      --sz;
    }

    lua_pushstring( s, buf );
    vlFree( buf );
  }

  return 1;
}
static int pvlLongBracket( lua_State* s ){
  u32 ne, i, sz;
  const u8* str;
  u32 ans = vlAutoMalloc( 0 );
  vlLuaAssess( s, lua_gettop( s ) == 1 && lua_isstring( s, 1 ), "Bad arguments to pvlCompactStringEncode (should be one string)." );
  ne = vlGetNumQuoteEquals( lua_tostring( s, 1 ) );
  str = lua_tolstring( s, 1, &sz );
  vlAppendString( ans, "[" ); for( i = 0; i < ne; ++i ) vlAppendString( ans, "=" ); vlAppendString( ans, "[\n" );  
  vlPopNul( ans );
  vlAppend( ans, str, sz );
  vlAppendString( ans, "]" ); for( i = 0; i < ne; ++i ) vlAppendString( ans, "=" ); vlAppendString( ans, "]" );  
  vlPopNul( ans );
  lua_pushlstring( s, vlMem( ans ), vlSize( ans ) - ( ( ( (u8*)vlMem( ans ) )[ vlSize( ans ) - 1 ] )?0:1 ) );
  vlAutoFree( ans );
  return 1;
}
static int pvlAsciiDecode( lua_State* s ){
  static u8 pvlAsciiDecodeCodes[] = { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 56, 255, 58, 59, 60, 62, 76, 64, 65, 63, 69, 78, 66, 79, 255, 55, 46, 47, 48, 49, 50, 51, 52, 53, 54, 77, 75, 80, 68, 81, 82, 57, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 70, 255, 71, 61, 67, 83, 255, 255, 255, 255, 255, 255, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 72, 74, 73, 84, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 };
  u32 sz;
  const u8* str;

  vlLuaAssess( s, lua_gettop( s ) == 1 && lua_isstring( s, 1 ), "Bad arguments to pvlAsciiDecode (should be a string)." );
  lua_pushnil( s );
  str = lua_tolstring( s, 1, &sz );
  if( sz % 5 == 1 )
    return 1;
  {
    u8* buf;
    u32 cnt = ( sz + 4 ) / 5;
    u32 mod = 4 - ( sz + 4 ) % 5;
    const u8* sp = str;
    const u8* ep = str + sz;
    u32 dsz = cnt * 4 - mod;
    u32* dp;
    buf = vlMalloc( cnt * 4 + 1 );
    dp = (u32*)buf;

    while( cnt ){
      u64 vm = 1;
      u64 v = 0;
      u32 i;
      for( i = 0; i < 5; ++i ){
        u8 sv = ( sp >= ep ) ? 0 : pvlAsciiDecodeCodes[ *sp++ ];
        if( sv == 255 ){
          vlFree( buf );
          return 1;  
        }
        v += vm * sv;
        vm *= 85;
      }
      --cnt;
      if( v > 4294967295 ){
        vlFree( buf );
        return 1;  
      }
      *dp++ = (u32)v;
    }
    lua_pop( s, 1 );
    lua_pushlstring( s, buf, dsz );
    vlFree( buf );
  }

  return 1;
}
static int pvlHexAsciiDecode( lua_State* s ){
  u32 sz;
  const u8* str;

  vlLuaAssess( s, lua_gettop( s ) == 1 && lua_isstring( s, 1 ), "Bad arguments to pvlHexAsciiDecode (should be a string)." );
  str = lua_tolstring( s, 1, &sz );
  vlLuaAssess( s, !( sz % 2 ), "String passed to pvlHexAsciiDecode with an odd number of characters." );
  vlLuaAssess( s, strspn( str, "0123456789ABCDEF" ) == sz, "String passed to pvlHexAsciiDecode with non-hexidecimal characters." );
  {
    u8* buf;
    u32 cnt = sz / 2;
    u32 dsz = cnt;
    const u8* sp = str;
    u8* dp;
    buf = vlMalloc( dsz );
    dp = buf;
    while( cnt ){
      *dp = ( ( ( *sp >= '0' ) && ( *sp <= '9' ) ) ? ( *sp - '0' ) : ( *sp - ( 'A' - 10 ) ) ) << 4;
      ++sp;
      *dp++ += ( ( ( *sp >= '0' ) && ( *sp <= '9' ) ) ? ( *sp - '0' ) : ( *sp - ( 'A' - 10 ) ) );
      ++sp;
      --cnt;
    }
    lua_pushlstring( s, buf, dsz );
    vlFree( buf );
  }

  return 1;
}
static int pvlClock( lua_State* s ){
  vlLuaAssess( s, lua_gettop( s ) == 0, "Wrong number of arguments given to pvlClock (should be 0)." );
  lua_pushnumber( s, (f64)osClock() / (f64)osClockFrequency() );
  return 1;
}
static int pvlUnixTime( lua_State* s ){
  vlLuaAssess( s, lua_gettop( s ) == 0, "Wrong number of arguments given to pvlUnixTime (should be 0)." );
  lua_pushnumber( s, osUnixTime() );
  return 1;
}
static const pvlWrapper pvlvlFuncs[] = {
  // UTF
  { pvlUTF8To16, "UTF8To16" },
  { pvlUTF8To32, "UTF8To32" },
  { pvlUTF16To8, "UTF16To8" },
  { pvlUTF16To32, "UTF16To32" },
  { pvlUTF32To8, "UTF32To8" },
  { pvlUTF32To16, "UTF32To16" },
  { pvlUTF8BytePosition, "UTF8BytePosition" },
  { pvlUTF8CharPosition, "UTF8CharacterPosition" },
  

  // String
  { pvlAsciiEncode, "asciiEncode" },
  { pvlHexAsciiEncode, "hexAsciiEncode" },
  { pvlLongBracket, "longBracket" },
  { pvlAsciiDecode, "asciiDecode" },
  { pvlHexAsciiDecode, "hexAsciiDecode" },
  { pvlCompress, "compress" },
  { pvlUncompress, "uncompress" },
  { pvlLevenshtein, "levenshtein" },
  { pvlClosestStrings, "closestStrings" },
  { pvlExpatParseXML, "expatParseXML" },

  // System
  { pvlSleep, "sleep" },
  { pvlClock, "clock" },
  { pvlUnixTime, "unixTime" },
  { pvlInform, "inform" },
  { pvlQuery, "query" },
  { pvlInformError, "informError" },
  { pvlQuit, "quit" },
  { pvlDie, "die" },
  { pvlLoop, "loop" },
  { pvlWaitMessage, "wait" },
  { pvlIsIdle, "isIdle" },
  { pvlLoadResource, "loadResource" },
  { pvlGetUserData, "getUserData" },
  { pvlSetUserData, "setUserData" },
  { pvlSetCWD, "setCWD" },
  { pvlGetCWD, "getCWD" },
  { pvlStripPath, "stripPath" },
  { pvlGetFullPath, "getFullPath" },
  { pvlSaveFile, "saveFile" },
  { pvlLoadFile, "loadFile" },
  { pvlGetPasteBuffer, "getPasteBuffer" },
  { pvlSetPasteBuffer, "setPasteBuffer" },
  { pvlFileDialog, "fileDialog" },

  // Threads
  { pvlThread, "thread" },
  { pvlNumThreads, "numThreads" },
  { pvlIsThreadRunning, "isThreadRunning" },
  { pvlItcPost, "itcPost" },
  { pvlItcRetrieve, "itcRetrieve" },

  // Globals
  { pvlSetGlobal, "setGlobal" },
  { pvlGetGlobal, "getGlobal" },
  { pvlGetAllGlobals, "getAllGlobals" },
  { pvlLockGlobal, "lockGlobal" },
  { pvlSetGlobalNonAtomic, "setGlobalNonAtomic" },
  { pvlGetGlobalNonAtomic, "getGlobalNonAtomic" },
  { pvlReleaseGlobal, "releaseGlobal" },

  // IO
  { pvlPrint, "print" },
  { pvlErrorPrint, "errorPrint" },
  { pvlPrintBuffer, "getPrintBuffer" },
  { pvlErrorPrintBuffer, "getErrorPrintBuffer" },
  { pvlIsConsoleOutAttached, "isConsoleOutAttached" },
  { pvlIsConsoleErrorAttached, "isConsoleErrorAttached" },
  { pvlGetFont, "getFont" },

  // Sysinfo
  { pvlSysInfoVislibVersion, "sysInfoVislibVersion" },
  { pvlSysInfoOsVersion, "sysInfoOsVersion" },
  { pvlSysInfoLuaVersion, "sysInfoLuaVersion" },
  { pvlSysInfoDesktop, "sysInfoDesktop" },
  { pvlSysInfoVirtualDisplay, "sysInfoVirtualDisplay" },
  { pvlSysInfoDisplays, "sysInfoDisplays" },

  // Misc
  { pvlType, "type" },
  { pvlSetmetatable, "setmetatable" },
  { pvlWindowStatus, "windowStatus" },


};
static const pvlWrapper pvlWindowFuncs[] = {
  { pvlNewWindow, "new" },
  { pvlIsVisibleWindow, "visible" },
  { pvlGetPosWindow, "getPos" },
  { pvlSetPosWindow, "setPos" },
  { pvlShowWindow, "show" },
  { pvlHideWindow, "hide" },
  { pvlRaiseWindow, "raise" },
  { pvlFocusWindow, "focus" },
  { pvlGetTitleWindow, "getTitle" },
  { pvlSetTitleWindow, "setTitle" },
  { pvlAddMenuWindow, "addMenu" },
  { pvlRemoveMenuWindow, "removeMenu" },
  { pvlAddMenuBreakWindow, "addMenuBreak" },
  { pvlSetMenuItemState, "setMenuItemState" },
  { pvlGetStyleWindow, "getStyle" },
  { pvlDestroyWindow, "destroy" },
};
static const pvlWrapper pvlWidgetFuncs[] = {
  { pvlNewWidget, "new" },
  { pvlIsVisibleWidget, "visible" },
  { pvlGetPosWidget, "getPos" },
  { pvlSetPosWidget, "setPos" },
  { pvlShowWidget, "show" },
  { pvlHideWidget, "hide" },
  { pvlFocusWidget, "focus" },
  { pvlGetModify, "getModify" },
  { pvlSetModify, "setModify" },
  { pvlGetTextWidget, "getText" },
  { pvlDestroyWidget, "destroy" },
  { pvlSelectTextWidget, "select" },
  { pvlGetSelectionTextWidget, "getSelection" },
  { pvlGetSelectionTextTextWidget, "getSelectionText" },
  { pvlReplaceSelectionFormatedTextWidget, "replaceSelectionFormated" },
  { pvlReplaceAllFormatedTextWidget, "replaceAllFormated" },
  { pvlReplaceSelectionWithHR, "replaceSelectionWithHR" },
  { pvlUndoTextWidget, "undo" },
  { pvlRedoTextWidget, "redo" },
  { pvlScrollWidget, "scroll" },
  { pvlRelativeScrollWidget, "relativeScroll" },
  { pvlGetStyleWidget, "getStyle" },
  { pvlSetPartTextWidget, "setPartText" },

  { pvlSetPartsWidget, "setParts" },
  { pvlGetLineCountWidget, "lineCount" },
  { pvlLineFromCharWidget, "lineFromChar" },
  { pvlCharFromLineWidget, "charFromLine" },
};