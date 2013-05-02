#include "vislib.h"
#include "rc.h"

#pragma warning( push )
#pragma warning( disable: 4668 4820 4255 )
#include <richedit.h>
#pragma warning( pop )

extern int _fltused = 0;

#include "glprot.c"

extern surface vscreenTex = { 0 };
void vgetScreenTex( u8* data );
void osGetScreenBytes( u8* data );

extern int vkeys[ 256 ] = { 0 };
extern int vkeyReleases[ 256 ] = { 0 };
extern int vkeyPresses[ 256 ] = { 0 };
extern int vmouseButtons[ 5 ] = { 0 };
extern u32 vxpos = 0;
extern u32 vypos = 0;
extern u32 vwidth = 1;
extern u32 vheight = 1;
extern u32 vscreenWidth = 0;
extern u32 vscreenHeight = 0;
extern int vactive = TRUE;
extern int vfullscreen = TRUE;

static LARGE_INTEGER vhpFrequency;
static LARGE_INTEGER vclock;
static LARGE_INTEGER vstartClock;

static f32 vmouseXDelta = 0;
static f32 vmouseYDelta = 0;
static f32 vmouseWheelDelta = 0;


HINSTANCE vinstance = NULL;
HWND vwindowHandle = NULL;
HWND vlogWindowParentHandle = NULL;
HWND vlogWindowHandle = NULL;
HDC vdc = NULL;
HGLRC vrc = NULL;

static u32 vallocCount = 0;
static u32 vallocBuffSize = 0;
static void** vallocs = NULL;
static u32* vallocCounts = NULL;
static u32* vallocBuffSizes = NULL;

static u32 vsysInfoName = 0;
static u32 vextensionsName = 0;
static u32 vlogName = 0;

static u32 vpixelFormatsName = 0;
static u32 vpixelFormatsLogName = 0;

void* vsmalloc( u32 sz ){
  void* nb = GlobalAlloc( GMEM_FIXED, sz );
  if( nb == NULL ) 
    vdie( "Unable to allocate memory!." );
  return nb;
}

void pvnquit( void );

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
const u8* vgetResourceOrDie( u32 handle, u32* sz, const u8* type ){
  const u8* tp;
  u8* ans = NULL;
  u32 ts;
  HRSRC hnd = FindResource( NULL, MAKEINTRESOURCE( handle ), type );
  HGLOBAL hg;
  if( hnd == NULL ) vdie( "FindResource failed." );
  ts = SizeofResource( NULL, hnd );
  if( !ts ) vdie( "Zero sized resource!" );
  if( sz != NULL )
    *sz = ts;
  hg = LoadResource( NULL, hnd );
  if( hg == NULL ) vdie( "Resource handle load failed with NULL." );
  tp = LockResource( hg );
  if( tp == NULL ) vdie( "Resource handle load failed with NULL in LockResource." );
  ans = vsmalloc( ts + 1 );
  vmemcpy( ans, tp, ts );
  ans[ ts ] = '\0';
  return ans;
}

const u8* vsysInfo( void ){
  if( !vsysInfoName ){
    u32 i;
    const u8* inf;
    vsysInfoName = vmalloc( 0 );
    ( (u8*)vmem( vsysInfoName ) )[ 0 ] = 0;
    vappendString( vsysInfoName, "OS: Windows v" );
    vappendInt( vsysInfoName, LOBYTE(LOWORD(GetVersion())), 0 );  
    vappendString( vsysInfoName, "." );  
    vappendInt( vsysInfoName, HIBYTE(LOWORD(GetVersion())), 0 );  
    vappendString( vsysInfoName, "\n" );

    vappendString( vsysInfoName, "Vislib version: " VISLIB_VERSION "\n" );
    vappendString( vsysInfoName, "OpenGL version: " );
    inf = (const u8*)glGetString( GL_VERSION ); if( inf == NULL ) vdie( "glGetString failed!" );
    vappendString( vsysInfoName, inf ); vappendString( vsysInfoName, "\n" );
    vappendString( vsysInfoName, "OpenGL vendor: " );
    inf = (const u8*)glGetString( GL_VENDOR ); if( inf == NULL ) vdie( "glGetString failed!" );
    vappendString( vsysInfoName, inf ); vappendString( vsysInfoName, "\n" );
    vappendString( vsysInfoName, "OpenGL renderer: " );
    inf = (const u8*)glGetString( GL_RENDERER ); if( inf == NULL ) vdie( "glGetString failed!" );
    vappendString( vsysInfoName, inf ); vappendString( vsysInfoName, "\n" );

    {
      //POINT SIZE MIN f
      //POINT SIZE MAX f
      //MAX TEXTURE STACK DEPTH i
      //MAX PROJECTION STACK DEPTH i 
      //MAX MODELVIEW STACK DEPTH i
      //MAX COLOR MATRIX STACK DEPTH i
      //MAX LIGHTS i
      //MAX CLIP PLANES i
      //SUBPIXEL BITS i
      //MAX 3D TEXTURE SIZE i
      //MAX TEXTURE SIZE i
      //MAX ARRAY TEXTURE LAYERS i
      //MAX TEXTURE LOD BIAS i
      //MAX CUBE MAP TEXTURE SIZE i
      //MAX TEXTURE UNITS i
      //MAX VERTEX ATTRIBS i
      //MAX VERTEX UNIFORM COMPONENTS i
      //MAX VARYING COMPONENTS i
      //MAX COMBINED TEXTURE IMAGE UNITS i
      //MAX VERTEX TEXTURE IMAGE UNITS i
      //MAX TEXTURE IMAGE UNITS i
      //MAX TEXTURE COORDS i
      //MAX FRAGMENT UNIFORM COMPONENTS i
      //AUX BUFFERS i
      //MAX DRAW BUFFERS i
      //RGBA MODE b
      //INDEX MODE b
      //DOUBLEBUFFER b
      //STEREO b
      //SAMPLE BUFFERS i
      //SAMPLES i
      //MAX COLOR ATTACHMENTS i
      //MAX SAMPLES i
      //RED BIT i
      //GREEN BITS i
      //BLUE BITS i
      //ALPHA BITS i
      //INDEX BITS i
      //DEPTH BITS i
      //STENCIL BITS i
      //ACCUM RED BITS i
      //ACCUM GREEN BITS i
      //ACCUM BLUE BITS i
      GLenum enms[] = {
        GL_POINT_SIZE_MIN,
        GL_POINT_SIZE_MAX,
        GL_MAX_TEXTURE_STACK_DEPTH,
        GL_MAX_PROJECTION_STACK_DEPTH ,
        GL_MAX_MODELVIEW_STACK_DEPTH,
        GL_MAX_COLOR_MATRIX_STACK_DEPTH,
        GL_MAX_LIGHTS,
        GL_MAX_CLIP_PLANES,
        GL_SUBPIXEL_BITS,
        GL_MAX_3D_TEXTURE_SIZE,
        GL_MAX_TEXTURE_SIZE,
        GL_MAX_ARRAY_TEXTURE_LAYERS_EXT,
        GL_MAX_TEXTURE_LOD_BIAS,
        GL_MAX_CUBE_MAP_TEXTURE_SIZE,
        GL_MAX_TEXTURE_UNITS,
        GL_MAX_VERTEX_ATTRIBS,
        GL_MAX_VERTEX_UNIFORM_COMPONENTS,
        GL_MAX_VARYING_COMPONENTS_EXT,
        GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS,
        GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS,
        GL_MAX_TEXTURE_IMAGE_UNITS,
        GL_MAX_TEXTURE_COORDS,
        GL_MAX_FRAGMENT_UNIFORM_COMPONENTS,
        GL_AUX_BUFFERS,
        GL_MAX_DRAW_BUFFERS,
        GL_RGBA_MODE,
        GL_INDEX_MODE,
        GL_DOUBLEBUFFER,
        GL_STEREO,
        GL_SAMPLE_BUFFERS,
        GL_SAMPLES,
        GL_MAX_COLOR_ATTACHMENTS_EXT,
        GL_MAX_SAMPLES_EXT,
        GL_RED_BITS,
        GL_GREEN_BITS,
        GL_BLUE_BITS,
        GL_ALPHA_BITS,
        GL_INDEX_BITS,
        GL_DEPTH_BITS,
        GL_STENCIL_BITS,
        GL_ACCUM_RED_BITS,
        GL_ACCUM_GREEN_BITS,
        GL_ACCUM_BLUE_BITS,
      };
      const u8* strs[] = {
        "GL_POINT_SIZE_MIN",
        "GL_POINT_SIZE_MAX",
        "GL_MAX_TEXTURE_STACK_DEPTH",
        "GL_MAX_PROJECTION_STACK_DEPTH ",
        "GL_MAX_MODELVIEW_STACK_DEPTH",
        "GL_MAX_COLOR_MATRIX_STACK_DEPTH",
        "GL_MAX_LIGHTS",
        "GL_MAX_CLIP_PLANES",
        "GL_SUBPIXEL_BITS",
        "GL_MAX 3D_TEXTURE_SIZE",
        "GL_MAX_TEXTURE_SIZE",
        "GL_MAX_ARRAY_TEXTURE_LAYERS",
        "GL_MAX_TEXTURE_LOD_BIAS",
        "GL_MAX_CUBE_MAP_TEXTURE_SIZE",
        "GL_MAX_TEXTURE_UNITS",
        "GL_MAX_VERTEX_ATTRIBS",
        "GL_MAX_VERTEX_UNIFORM_COMPONENTS",
        "GL_MAX_VARYING_COMPONENTS",
        "GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS",
        "GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS",
        "GL_MAX_TEXTURE_IMAGE_UNITS",
        "GL_MAX_TEXTURE_COORDS",
        "GL_MAX_FRAGMENT_UNIFORM_COMPONENTS",
        "GL_AUX_BUFFERS",
        "GL_MAX_DRAW_BUFFERS",
        "GL_RGBA_MODE",
        "GL_INDEX_MODE",
        "GL_DOUBLEBUFFER",
        "GL_STEREO",
        "GL_SAMPLE_BUFFERS",
        "GL_SAMPLES",
        "GL_MAX_COLOR_ATTACHMENTS",
        "GL_MAX_SAMPLES",
        "GL_RED_BIT",
        "GL_GREEN_BITS",
        "GL_BLUE_BITS",
        "GL_ALPHA_BITS",
        "GL_INDEX_BITS",
        "GL_DEPTH_BITS",
        "GL_STENCIL_BITS",
        "GL_ACCUM_RED_BITS",
        "GL_ACCUM_GREEN_BITS",
        "GL_ACCUM_BLUE_BITS",
      };
      int types[] = {
        2,
        2,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        3,
        3,
        3,
        3,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
      };
      vappendString( vsysInfoName, "\n\nOpenGL information:\n" );
      for( i = 0; i < ( sizeof( enms ) / sizeof( enms[ 0 ] ) ); ++i ){
        if( types[ i ] == 1 ){
          GLint iinf;
          glGetIntegerv( enms[ i ], &iinf ); 
          vappendString( vsysInfoName, strs[ i ] ); vappendString( vsysInfoName, ": " ); 
          vappendInt( vsysInfoName, iinf, 0 ); 
          vappendString( vsysInfoName, "\n" );
        }else if( types[ i ] == 2 ){
          GLfloat finf;
          glGetFloatv( enms[ i ], &finf ); 
          vappendString( vsysInfoName, strs[ i ] ); vappendString( vsysInfoName, ": " ); 
          vappendInt( vsysInfoName, (u32)finf, 0 ); 
          vappendString( vsysInfoName, "\n" );
        }else if( types[ i ] == 3 ){
          GLboolean binf;
          glGetBooleanv( enms[ i ], &binf ); 
          vappendString( vsysInfoName, strs[ i ] ); vappendString( vsysInfoName, ": " ); 
          vappendInt( vsysInfoName, (u32)binf, 0 ); 
          vappendString( vsysInfoName, "\n" );
        }
      }
    }
  }
  return vmem( vsysInfoName );
}
const u8* vglExtensions( void ){
  const u8* inf;
  if( !vextensionsName ){
    vextensionsName = vmalloc( 1 );
    ( (u8*)vmem( vextensionsName ) )[ 0 ] = 0;
    inf = (const u8*)glGetString( GL_EXTENSIONS ); if( inf == NULL ) vdie( "glGetString failed!" );
    vappendString( vextensionsName, inf );
    vappendString( vextensionsName, wglGetExtensionsString( GetDC( NULL ) ) );
  } 
  return vmem( vextensionsName );
}
int vglExtensionAvailable( const u8* ext ){
  assess( ext != NULL, "NULL pointer in vglExtensionAvailable!" );
  if( vstrstr( vglExtensions(), ext ) == NULL )
    return 0;
  else
    return 1;
}
u32 vglNumPixelFormats( void ){
  return vsize( vpixelFormatsName ) / sizeof( pixelFormat );
} 
void pvlInitPixelFormats( void ){
  int args[] = {
    WGL_DRAW_TO_PBUFFER_ARB,     //0
    WGL_ACCELERATION_ARB,        //1
    WGL_SWAP_METHOD_ARB,         //2
    WGL_NUMBER_OVERLAYS_ARB,     //3
    WGL_NUMBER_UNDERLAYS_ARB,    //4
    WGL_TRANSPARENT_ARB,         //5
    WGL_SUPPORT_OPENGL_ARB,      //6
    WGL_DOUBLE_BUFFER_ARB,       //7
    WGL_STEREO_ARB,              //8
    WGL_PIXEL_TYPE_ARB,          //9
    WGL_RED_BITS_ARB,            //10
    WGL_RED_SHIFT_ARB,           //11
    WGL_GREEN_BITS_ARB,          //12
    WGL_GREEN_SHIFT_ARB,         //13
    WGL_BLUE_BITS_ARB,           //14
    WGL_BLUE_SHIFT_ARB,          //15
    WGL_ALPHA_BITS_ARB,          //16
    WGL_ALPHA_SHIFT_ARB,         //17
    WGL_ACCUM_BITS_ARB,          //18
    WGL_DEPTH_BITS_ARB,          //19
    WGL_STENCIL_BITS_ARB,        //20
    WGL_AUX_BUFFERS_ARB,         //21
    WGL_BIND_TO_TEXTURE_RGB_ARB, //22
    WGL_BIND_TO_TEXTURE_RGBA_ARB,//23
  };
  int rets[ sizeof( args ) / sizeof( args[ 0 ] ) ];
  int numf;
  u32 i;
  int numfarg = WGL_NUMBER_PIXEL_FORMATS_ARB;
  vpixelFormatsLogName = vmalloc( 1 );
  vpixelFormatsName = vmalloc( 1 );
  vappendString( vpixelFormatsLogName, "Total number of formats: " );
  if( wglGetPixelFormatAttribiv( wglGetCurrentDC(), 0, 0, 1, &numfarg, &numf ) == FALSE )
    vdie( "wglGetPixelFormatAttribiv failed!" );
  vappendInt( vpixelFormatsLogName, numf, 0 );
  for( i = 0; i < (u32)numf; ++i ){
    int ignore = 0;
    pixelFormat pf;
    if( wglGetPixelFormatAttribiv( wglGetCurrentDC(), i + 1, 0, sizeof( args ) / sizeof( args[ 0 ] ), args, rets ) == FALSE )
      vdie( "wglGetPixelFormatAttribiv failed!" );
    if( !rets[ 0 ] ) ignore = 1;//WGL_DRAW_TO_PBUFFER_ARB
    if( rets[ 1 ] != WGL_FULL_ACCELERATION_ARB ) ignore = 1; //WGL_ACCELERATION_ARB
    if( rets[ 7 ] && rets[ 2 ] != WGL_SWAP_EXCHANGE_ARB ) ignore = 1; //WGL_SWAP_METHOD_ARB, WGL_DOUBLE_BUFFER_ARB
    if( rets[ 7 ] ) pf.doubleBuffer = 1; else pf.doubleBuffer = 0; //WGL_DOUBLE_BUFFER_ARB
    if( rets[ 3 ] || rets[ 4 ] ) ignore = 1; //WGL_NUMBER_OVERLAYS_ARB, WGL_NUMBER_UNDERLAYS_ARB
    if( rets[ 5 ] ) ignore = 1; //WGL_TRANSPARENT_ARB
    if( !rets[ 6 ] ) ignore = 1; //WGL_SUPPORT_OPENGL_ARB
    if( rets[ 8 ] ) ignore = 1; //WGL_STEREO_ARB
    if( rets[ 9 ] == WGL_TYPE_COLORINDEX_ARB ) ignore = 1; //WGL_PIXEL_TYPE_ARB
    pf.redBits = rets[ 10 ]; //WGL_RED_BITS_ARB
    pf.redShift = rets[ 11 ]; //WGL_RED_SHIFT_ARB
    pf.greenBits = rets[ 12 ]; //WGL_GREEN_BITS_ARB
    pf.greenShift = rets[ 13 ]; //WGL_GREEN_SHIFT_ARB
    pf.blueBits = rets[ 14 ]; //WGL_BLUE_BITS_ARB
    pf.blueShift = rets[ 15 ]; //WGL_BLUE_SHIFT_ARB
    pf.alphaBits = rets[ 16 ]; //WGL_ALPHA_BITS_ARB
    pf.alphaShift = rets[ 17 ]; //WGL_ALPHA_SHIFT_ARB
    pf.accumBits = rets[ 18 ]; //WGL_ACCUM_BITS_ARB
    pf.depthBits = rets[ 19 ]; //WGL_DEPTH_BITS_ARB
    pf.stencilBits = rets[ 20 ]; //WGL_STENCIL_BITS_ARB
    pf.auxBuffers = rets[ 21 ]; //WGL_AUX_BUFFERS_ARB
    if( !rets[ 16 ] && !( rets[ 23 ] || rets[ 22 ] ) ) ignore = 1;//WGL_BIND_TO_TEXTURE_RGB_ARB
    if( rets[ 16 ] && !rets[ 23 ] ) ignore = 1;//WGL_BIND_TO_TEXTURE_RGBA_ARB
    pf.format = i + 1;
    pf.bpp = ( 1 + ( pf.doubleBuffer ? 1 : 0 ) ) * ( pf.accumBits + pf.depthBits + ( ( pf.redBits + pf.greenBits + pf.blueBits + pf.alphaBits ) * ( 1 + pf.auxBuffers ) ) );
    if( !ignore ){
      vappend( vpixelFormatsName, &pf, sizeof( pixelFormat ) );
      vappendString( vpixelFormatsLogName, "\n\n" );
      vappendString( vpixelFormatsLogName, vglPixelFormatString( &pf ) );
    }
  }
}
const u8* vglPixelFormatsString( void ){
  assess( vpixelFormatsLogName, "vpixelFormatsLogName is bad!" );
  return vmem( vpixelFormatsLogName );
}
const u8* vglPixelFormatString( const pixelFormat* pf ){
  static u32 tb = 0;
  if( !tb )
    tb = vmalloc( 1 );
  else
    verase( tb );
  vappendString( tb, "Format number:    " ); vappendInt( tb, pf->format, 0 );
  vappendString( tb, "\nRed bits+shift:   " ); vappendInt( tb, pf->redBits, 0 ); vappendString( tb, "+" ); vappendInt( tb, pf->redShift, 0 );
  vappendString( tb, "\nGreen bits+shift: " ); vappendInt( tb, pf->greenBits, 0 ); vappendString( tb, "+" ); vappendInt( tb, pf->greenShift, 0 );
  vappendString( tb, "\nBlue bits+shift:  " ); vappendInt( tb, pf->blueBits, 0 ); vappendString( tb, "+" ); vappendInt( tb, pf->blueShift, 0 );
  vappendString( tb, "\nAlpha bits+shift: " ); vappendInt( tb, pf->alphaBits, 0 ); vappendString( tb, "+" ); vappendInt( tb, pf->alphaShift, 0 );
  vappendString( tb, "\nDepth bits:       " ); vappendInt( tb, pf->depthBits, 0 );
  vappendString( tb, "\nStencil bits:     " ); vappendInt( tb, pf->stencilBits, 0 );
  vappendString( tb, "\nAccumulator bits: " ); vappendInt( tb, pf->accumBits, 0 );
  vappendString( tb, "\nAux buffers:      " ); vappendInt( tb, pf->auxBuffers, 0 );
  vappendString( tb, "\nDouble buffering: " ); vappendString( tb, pf->doubleBuffer ? "yes" : "no" );
  vappendString( tb, "\nbits per pixel:   " ); vappendInt( tb, pf->bpp, 0 );
  return( vmem( tb ) );
}
const pixelFormat* vglFindPixelFormat( int minRedBits, int maxRedBits, int minGreenBits, int maxGreenBits, int minBlueBits, int maxBlueBits, int minAlphaBits, int maxAlphaBits, int minDepthBits, int maxDepthBits, int minStencilBits, int maxStencilBits, int minAccumBits, int maxAccumBits, int minAuxBuffers, int maxAuxBuffers, int requireDB, int requireNoDB, int requireBGR, int requireRGB ){
  u32 i;
  u32 m = vglNumPixelFormats();
  pixelFormat* ans = NULL;
  for( i = 0; i < m; ++i ){
    int match = 1;
    pixelFormat* pf = ( (pixelFormat*)vmem( vpixelFormatsName ) ) + i;
    if( (int)pf->redBits < minRedBits ) match = 0;
    if( (int)pf->redBits > maxRedBits && maxRedBits >= 0 ) match = 0;
    if( (int)pf->greenBits < minGreenBits ) match = 0;
    if( (int)pf->greenBits > maxGreenBits && maxGreenBits >= 0 ) match = 0;
    if( (int)pf->blueBits < minBlueBits ) match = 0;
    if( (int)pf->blueBits > maxBlueBits && maxBlueBits >= 0 ) match = 0;
    if( (int)pf->alphaBits < minAlphaBits ) match = 0;
    if( (int)pf->alphaBits > maxAlphaBits && maxAlphaBits >= 0 ) match = 0;
    if( (int)pf->depthBits < minDepthBits ) match = 0;
    if( (int)pf->depthBits > maxDepthBits && maxDepthBits >= 0 ) match = 0;
    if( (int)pf->stencilBits < minStencilBits ) match = 0;
    if( (int)pf->stencilBits > maxStencilBits && maxStencilBits >= 0 ) match = 0;
    if( (int)pf->accumBits < minAccumBits ) match = 0;
    if( (int)pf->accumBits > maxAccumBits && maxAccumBits >= 0 ) match = 0;
    if( (int)pf->auxBuffers < minAuxBuffers ) match = 0;
    if( (int)pf->auxBuffers > maxAuxBuffers && maxAuxBuffers >= 0 ) match = 0;
    if( requireDB && !pf->doubleBuffer ) match = 0;
    if( requireNoDB && pf->doubleBuffer ) match = 0;
    if( requireRGB && ( ( pf->redShift > pf->greenShift ) || ( pf->greenShift > pf->blueShift ) ) ) match = 0;
    if( requireBGR && ( ( pf->blueShift > pf->greenShift ) || ( pf->greenShift > pf->redShift ) ) ) match = 0;
    if( ans != NULL && ( ans->bpp < pf->bpp ) ) match = 0;
    if( match )
      ans = pf;
  }
  return ans;
}
void vcleanup( void ){
  u32 i;

  pvnquit();
  ReleaseCapture();
  if( vfullscreen ){
    ChangeDisplaySettings( NULL, 0 );
  }
  {
    int dc = -1;
    while( dc < 0 )
      dc = ShowCursor( TRUE );
  }
  wglMakeCurrent( vdc, NULL );
  if( vrc != NULL )
    wglDeleteContext( vrc );
  if( vwindowHandle != NULL && vdc != NULL )
    ReleaseDC( vwindowHandle, vdc );
  if( vwindowHandle != NULL )
    DestroyWindow( vwindowHandle );
  if( vlogWindowHandle != NULL )
    DestroyWindow( vlogWindowHandle );
  if( vinstance != NULL );
  UnregisterClass( "Vislib", vinstance );

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


void vinform( const u8* msg, const u8* title ){
  vlogInfo( title );
  vlog( "\n" );
  vlog( msg );
  vlog( "\n" );
}
int vquery( const u8* qst ){
  int ans;
  vdeactivate();
  ans = MessageBox( NULL, qst, "Question", MB_YESNO | MB_ICONQUESTION | MB_SYSTEMMODAL ) == IDYES;
  vactivate();
  return ans;
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

void pvupdateLog( const u8* msg ){
  if( !vlogName ){
    vlogName = vmalloc( 0 );
  }
  vappendString( vlogName, msg );
}
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


const u8* vlog( const u8* msg ){
  CHARFORMAT cf = { sizeof( CHARFORMAT ), CFM_BOLD | CFM_COLOR | CFM_FACE, 0, 0, 0, RGB( 255, 255, 255 ), 0, 0, "Courier New" };
  vshowLogWindow();
  SendMessage( vlogWindowHandle, EM_SETSEL, (WPARAM)-1, (LPARAM)-1 );
  pvupdateLog( msg );
  SendMessage( vlogWindowHandle, EM_SCROLL, SB_BOTTOM, 0 );
  SendMessage( vlogWindowHandle, EM_SETCHARFORMAT, (WPARAM)SCF_SELECTION, (LPARAM)&cf );
  SendMessage( vlogWindowHandle, EM_REPLACESEL, (WPARAM)NULL, (LPARAM)msg );
  return vmem( vlogName );
}
const u8* vlogInfo( const u8* msg ){
  CHARFORMAT cf = { sizeof( CHARFORMAT ), CFM_BOLD | CFM_COLOR | CFM_FACE, CFE_BOLD, 0, 0, RGB( 167, 255, 167 ), 0, 0, "Courier New" };
  vshowLogWindow();
  SendMessage( vlogWindowHandle, EM_SETSEL, (WPARAM)-1, (LPARAM)-1 );
  pvupdateLog( msg );
  SendMessage( vlogWindowHandle, EM_SCROLL, SB_BOTTOM, 0 );
  SendMessage( vlogWindowHandle, EM_SETCHARFORMAT, (WPARAM)SCF_SELECTION, (LPARAM)&cf );
  SendMessage( vlogWindowHandle, EM_REPLACESEL, (WPARAM)NULL, (LPARAM)msg );
  return vmem( vlogName );
}
const u8* vlogWarning( const u8* msg ){
  CHARFORMAT cf = { sizeof( CHARFORMAT ), CFM_BOLD | CFM_COLOR | CFM_FACE, CFE_BOLD, 0, 0, RGB( 255, 255, 0 ), 0, 0, "Courier New" };
  vshowLogWindow();
  SendMessage( vlogWindowHandle, EM_SETSEL, (WPARAM)-1, (LPARAM)-1 );
  pvupdateLog( "Warning: " );
  pvupdateLog( msg );
  SendMessage( vlogWindowHandle, EM_SCROLL, SB_BOTTOM, 0 );
  SendMessage( vlogWindowHandle, EM_SETCHARFORMAT, (WPARAM)SCF_SELECTION, (LPARAM)&cf );
  SendMessage( vlogWindowHandle, EM_REPLACESEL, (WPARAM)NULL, (LPARAM)"Warning: " );
  SendMessage( vlogWindowHandle, EM_REPLACESEL, (WPARAM)NULL, (LPARAM)msg );
  return vmem( vlogName );
}
const u8* vlogError( const u8* msg ){
  CHARFORMAT cf = { sizeof( CHARFORMAT ), CFM_BOLD | CFM_COLOR | CFM_FACE, CFE_BOLD, 0, 0, RGB( 255, 0, 0 ), 0, 0, "Courier New" };
  vshowLogWindow();
  SendMessage( vlogWindowHandle, EM_SETSEL, (WPARAM)-1, (LPARAM)-1 );
  pvupdateLog( "Error: " );
  pvupdateLog( msg );
  SendMessage( vlogWindowHandle, EM_SCROLL, SB_BOTTOM, 0 );
  SendMessage( vlogWindowHandle, EM_SETCHARFORMAT, (WPARAM)SCF_SELECTION, (LPARAM)&cf );
  SendMessage( vlogWindowHandle, EM_REPLACESEL, (WPARAM)NULL, (LPARAM)"Error: " );
  SendMessage( vlogWindowHandle, EM_REPLACESEL, (WPARAM)NULL, (LPARAM)msg );
  return vmem( vlogName );
}
LRESULT	CALLBACK vcallback( HWND, UINT, WPARAM, LPARAM );
void vginit( u8* title, u32 width, u32 height, u32 x, u32 y, u32 bits, int border ){
  PIXELFORMATDESCRIPTOR pfd;
  GLuint pf;
  u32 exstyle;
  u32 style;

  RECT windrect;

  windrect.left = 0;
  windrect.right = (LONG)width;
  windrect.top = 0;
  windrect.bottom = (LONG)height;

  exstyle = border ? 0 : WS_EX_TOPMOST;
  style = border ? 0 : WS_POPUP;

  AdjustWindowRectEx( &windrect, style, FALSE, exstyle );
  {
    int dw = windrect.right - windrect.left;
    int dh = windrect.bottom - windrect.top;
    vwindowHandle = CreateWindowEx( exstyle, "Vislib", title, style, x, y, dw, dh, NULL, NULL, vinstance, NULL );
    if( vwindowHandle == NULL )
      vdie( "Couldn't create render window." );
  }

  pfd.nSize = sizeof( PIXELFORMATDESCRIPTOR );
  pfd.nVersion = 1;
  pfd.dwFlags =	PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
  pfd.iPixelType = PFD_TYPE_RGBA;
  pfd.cColorBits = (u8)bits;
  pfd.cRedBits = 0; pfd.cRedShift = 0;	
  pfd.cGreenBits = 0; pfd.cGreenShift = 0;	
  pfd.cBlueBits = 0; pfd.cBlueShift = 0;	
  pfd.cAlphaBits = 0; pfd.cAlphaShift = 0;	
  pfd.cAccumBits = 0;
  pfd.cAccumRedBits = 0; pfd.cAccumGreenBits = 0;
  pfd.cAccumBlueBits = 0; pfd.cAccumAlphaBits = 0;
  pfd.cDepthBits = 0;
  pfd.cStencilBits = 0;
  pfd.cAuxBuffers = 0;
  pfd.iLayerType = PFD_MAIN_PLANE;
  pfd.bReserved = 0;
  pfd.dwLayerMask = 0;
  pfd.dwVisibleMask = 0;
  pfd.dwDamageMask = 0;

  {
    const u8* emsg = NULL;
    if( ( vdc = GetDC( vwindowHandle ) ) == 0 )
      emsg = "Failed to create context.";
    if( ( pf = ChoosePixelFormat( vdc, &pfd ) ) == 0 )
      emsg = "Failed to get a pixel format.";
    if( !SetPixelFormat( vdc, pf, &pfd ) )
      emsg = "Failed to set the pixel format.";
    if( ( vrc = wglCreateContext( vdc ) ) == 0 )
      emsg = "Failed to ceate rendering context.";
    if( !wglMakeCurrent( vdc, vrc ) )
      emsg = "Failed to make renderign context active.";
    if( emsg != NULL )
      vdie( emsg );
  }

  {
    ShowWindow( vwindowHandle, SW_SHOW );
    SetForegroundWindow( vwindowHandle );
    SetFocus( vwindowHandle );
    SetCapture( vwindowHandle );
    {
      int dc = 0;
      while( dc > 0 )
        dc = ShowCursor( FALSE );
    }
  }


#ifdef DBG
  vglGetError();
#endif
  {
    u32 i;
    PROC* funcs[] = {
      (PROC*)&glSecondaryColor3f,
      (PROC*)&glBindBuffer,
      (PROC*)&glDeleteBuffers,
      (PROC*)&glGenBuffers,
      (PROC*)&glIsBuffer,
      (PROC*)&glBufferData,
      (PROC*)&glBufferSubData,
      (PROC*)&glGetBufferSubData,
      (PROC*)&glMapBuffer,
      (PROC*)&glUnmapBuffer,
      (PROC*)&glGetBufferParameteriv,
      (PROC*)&glGetBufferPointerv,
      (PROC*)&glGenPrograms,
      (PROC*)&glGetProgramiv,
      (PROC*)&glVertexAttrib1s,
      (PROC*)&glVertexAttrib1f,
      (PROC*)&glVertexAttrib1d,
      (PROC*)&glVertexAttrib2s,
      (PROC*)&glVertexAttrib2f,
      (PROC*)&glVertexAttrib2d,
      (PROC*)&glVertexAttrib3s,
      (PROC*)&glVertexAttrib3f,
      (PROC*)&glVertexAttrib3d,
      (PROC*)&glVertexAttrib4s,
      (PROC*)&glVertexAttrib4f,
      (PROC*)&glVertexAttrib4d,
      (PROC*)&glVertexAttrib4Nub,
      (PROC*)&glVertexAttrib1sv,
      (PROC*)&glVertexAttrib1fv,
      (PROC*)&glVertexAttrib1dv,
      (PROC*)&glVertexAttrib2sv,
      (PROC*)&glVertexAttrib2fv,
      (PROC*)&glVertexAttrib2dv,
      (PROC*)&glVertexAttrib3sv,
      (PROC*)&glVertexAttrib3fv,
      (PROC*)&glVertexAttrib3dv,
      (PROC*)&glVertexAttrib4bv,
      (PROC*)&glVertexAttrib4sv,
      (PROC*)&glVertexAttrib4iv,
      (PROC*)&glVertexAttrib4ubv,
      (PROC*)&glVertexAttrib4usv,
      (PROC*)&glVertexAttrib4uiv,
      (PROC*)&glVertexAttrib4fv,
      (PROC*)&glVertexAttrib4dv,
      (PROC*)&glVertexAttrib4Nbv,
      (PROC*)&glVertexAttrib4Nsv,
      (PROC*)&glVertexAttrib4Niv,
      (PROC*)&glVertexAttrib4Nubv,
      (PROC*)&glVertexAttrib4Nusv,
      (PROC*)&glVertexAttrib4Nuiv,
      (PROC*)&glVertexAttribPointer,
      (PROC*)&glEnableVertexAttribArray,
      (PROC*)&glDisableVertexAttribArray,
      (PROC*)&glGetVertexAttribdv,
      (PROC*)&glGetVertexAttribfv,
      (PROC*)&glGetVertexAttribiv,
      (PROC*)&glGetVertexAttribPointerv,
      (PROC*)&wglGetExtensionsString,
      (PROC*)&glActiveTexture,
      (PROC*)&glMultiTexCoord3f,
      (PROC*)&glMultiTexCoord4f,
      (PROC*)&wglSwapInterval,
      (PROC*)&wglGetPixelFormatAttribiv,
      (PROC*)&wglChoosePixelFormat,
      (PROC*)&wglCreatePbuffer,
      (PROC*)&wglGetPbufferDC,
      (PROC*)&wglQueryPbuffer,
      (PROC*)&wglDestroyPbuffer,
      (PROC*)&wglReleasePbufferDC,
      (PROC*)&wglBindTexImage,
      (PROC*)&wglReleaseTexImage,
      (PROC*)&glCreateShader,
      (PROC*)&glShaderSource,
      (PROC*)&glCompileShader,
      (PROC*)&glDeleteShader,
      (PROC*)&glIsShader,
      (PROC*)&glGetShaderiv,
      (PROC*)&glIsProgram,
      (PROC*)&glGetAttachedShaders,
      (PROC*)&glGetShaderInfoLog,
      (PROC*)&glGetProgramInfoLog,
      (PROC*)&glValidateProgram,
      (PROC*)&glGetShaderSource,
      (PROC*)&glCreateProgram,
      (PROC*)&glAttachShader,
      (PROC*)&glDetachShader,
      (PROC*)&glLinkProgram,
      (PROC*)&glUseProgram,
      (PROC*)&glDeleteProgram,
      (PROC*)&glGetUniformLocation,
      (PROC*)&glUniform1i,
      (PROC*)&glUniform2i,
      (PROC*)&glUniform3i,
      (PROC*)&glUniform4i,
      (PROC*)&glUniform1f,
      (PROC*)&glUniform2f,
      (PROC*)&glUniform3f,
      (PROC*)&glUniform4f,
      (PROC*)&glUniform1iv,
      (PROC*)&glUniform2iv,
      (PROC*)&glUniform3iv,
      (PROC*)&glUniform4iv,
      (PROC*)&glUniform1fv,
      (PROC*)&glUniform2fv,
      (PROC*)&glUniform3fv,
      (PROC*)&glUniform4fv,
      (PROC*)&glUniformMatrix2fv,
      (PROC*)&glUniformMatrix3fv,
      (PROC*)&glUniformMatrix4fv,
      (PROC*)&glUniformMatrix2x3fv,
      (PROC*)&glUniformMatrix3x2fv,
      (PROC*)&glUniformMatrix2x4fv,
      (PROC*)&glUniformMatrix4x2fv,
      (PROC*)&glUniformMatrix3x4fv,
      (PROC*)&glUniformMatrix4x3fv,
      (PROC*)&glGetAttribLocation,
      (PROC*)&glDrawBuffers,
      (PROC*)&glTexImage3D,
    };
    //extern {[^ ]*} {[^;]*};
    //(PROC*)&\2,
    u8* names[] = {
      "glSecondaryColor3f",
      "glBindBuffer",
      "glDeleteBuffers",
      "glGenBuffers",
      "glIsBuffer",
      "glBufferData",
      "glBufferSubData",
      "glGetBufferSubData",
      "glMapBuffer",
      "glUnmapBuffer",
      "glGetBufferParameteriv",
      "glGetBufferPointerv",
      "glGenPrograms",
      "glGetProgramiv",
      "glVertexAttrib1s",
      "glVertexAttrib1f",
      "glVertexAttrib1d",
      "glVertexAttrib2s",
      "glVertexAttrib2f",
      "glVertexAttrib2d",
      "glVertexAttrib3s",
      "glVertexAttrib3f",
      "glVertexAttrib3d",
      "glVertexAttrib4s",
      "glVertexAttrib4f",
      "glVertexAttrib4d",
      "glVertexAttrib4Nub",
      "glVertexAttrib1sv",
      "glVertexAttrib1fv",
      "glVertexAttrib1dv",
      "glVertexAttrib2sv",
      "glVertexAttrib2fv",
      "glVertexAttrib2dv",
      "glVertexAttrib3sv",
      "glVertexAttrib3fv",
      "glVertexAttrib3dv",
      "glVertexAttrib4bv",
      "glVertexAttrib4sv",
      "glVertexAttrib4iv",
      "glVertexAttrib4ubv",
      "glVertexAttrib4usv",
      "glVertexAttrib4uiv",
      "glVertexAttrib4fv",
      "glVertexAttrib4dv",
      "glVertexAttrib4Nbv",
      "glVertexAttrib4Nsv",
      "glVertexAttrib4Niv",
      "glVertexAttrib4Nubv",
      "glVertexAttrib4Nusv",
      "glVertexAttrib4Nuiv",
      "glVertexAttribPointer",
      "glEnableVertexAttribArray",
      "glDisableVertexAttribArray",
      "glGetVertexAttribdv",
      "glGetVertexAttribfv",
      "glGetVertexAttribiv",
      "glGetVertexAttribPointerv",
      "wglGetExtensionsString",
      "glActiveTexture",
      "glMultiTexCoord3f",
      "glMultiTexCoord4f",
      "wglSwapInterval",
      "wglGetPixelFormatAttribiv",
      "wglChoosePixelFormat",
      "wglCreatePbuffer",
      "wglGetPbufferDC",
      "wglQueryPbuffer",
      "wglDestroyPbuffer",
      "wglReleasePbufferDC",
      "wglBindTexImage",
      "wglReleaseTexImage",
      "glCreateShader",
      "glShaderSource",
      "glCompileShader",
      "glDeleteShader",
      "glIsShader",
      "glGetShaderiv",
      "glIsProgram",
      "glGetAttachedShaders",
      "glGetShaderInfoLog",
      "glGetProgramInfoLog",
      "glValidateProgram",
      "glGetShaderSource",
      "glCreateProgram",
      "glAttachShader",
      "glDetachShader",
      "glLinkProgram",
      "glUseProgram",
      "glDeleteProgram",
      "glGetUniformLocation",
      "glUniform1i",
      "glUniform2i",
      "glUniform3i",
      "glUniform4i",
      "glUniform1f",
      "glUniform2f",
      "glUniform3f",
      "glUniform4f",
      "glUniform1iv",
      "glUniform2iv",
      "glUniform3iv",
      "glUniform4iv",
      "glUniform1fv",
      "glUniform2fv",
      "glUniform3fv",
      "glUniform4fv",
      "glUniformMatrix2fv",
      "glUniformMatrix3fv",
      "glUniformMatrix4fv",
      "glUniformMatrix2x3fv",
      "glUniformMatrix3x2fv",
      "glUniformMatrix2x4fv",
      "glUniformMatrix4x2fv",
      "glUniformMatrix3x4fv",
      "glUniformMatrix4x3fv",
      "glGetAttribLocation",
      "glDrawBuffers",
      "glTexImage3D",
    };
    //extern {[^ ]*} {[^;]*};
    //"\2",

    for( i = 0; i < ( sizeof( names ) /  sizeof( names[ 0 ] ) ); ++i ){
      const u8* name = names[ i ];
      u32 nsz = vstrlen( name );
      u8* arbname = vsmalloc( nsz + 4 );
      u8* extname = vsmalloc( nsz + 4 );
      PROC ans;
      vstrcpy( arbname, name );
      vstrcpy( arbname + nsz, "ARB" );
      vstrcpy( extname, name );
      vstrcpy( extname + nsz, "EXT" );
      ans = (PROC)wglGetProcAddress( name );
      if( ans == NULL )
        ans = (PROC)wglGetProcAddress( arbname );
      if( ans == NULL )
        ans = (PROC)wglGetProcAddress( extname );
      vsfree( arbname ); vsfree( extname );
      if( ans == NULL ){
        u32 em = vmalloc( 1 );
        vappendString( em, "Unable to get function address for " );
        vappendString( em, name );
        vappendString( em, "." );
        vdie( vmem( em ) );
      }
      *funcs[ i ] = ans;
    }
  }

  // Check for required extensions.
  if( !vglExtensionAvailable( "GL_ARB_pixel_format" ) )
    vdie( "GL_ARB_pixel_format not available." );
  if( !vglExtensionAvailable( "WGL_ARB_pbuffer" ) )
    vdie( "GL_ARB_pixel_format not available." );

  // Check version.
  {
    u32 majorv, minorv;
    const u8* inf;
    inf = (const u8*)glGetString( GL_VERSION ); 
    if( inf == NULL )
      vdie( "glGetString failed!" );
    majorv = vreadInt( inf );
    while( *inf && *inf++ != '.' );
    minorv = vreadInt( inf );

    if( majorv < vislibRequiredOpenGLMajorVersion || ( majorv == vislibRequiredOpenGLMajorVersion && minorv < vislibRequiredOpenGLMinorVersion ) ){
      u32 msg = vmalloc( 1 );
      vappendString( msg, "Vislib " VISLIB_VERSION " requires at least OpenGL version " );
      vappendInt( msg, vislibRequiredOpenGLMajorVersion, 0 );
      vappendString( msg, "." );
      vappendInt( msg, vislibRequiredOpenGLMinorVersion, 0 );
      vdie( vmem( msg ) );
    }


#ifdef DBG
    vglGetError();
#endif

    vwidth = width;
    vheight = height;
  }

#ifdef DBG
  vglGetError();
#endif
  {
    u8* data = vsmalloc( vscreenWidth * vscreenHeight * 3 );
    osGetScreenBytes( data );
    vgetScreenTex( data );
    vsfree( data );
  }
#ifdef DBG
  vglGetError();
#endif
  pvlInitPixelFormats();
#ifdef DBG
  vglGetError();
#endif
  resize( vxpos, vypos, vwidth, vheight );
#ifdef DBG
  vglGetError();
#endif
}

void vinit(){
  vinstance = GetModuleHandle( NULL );

  if( !QueryPerformanceFrequency( &vhpFrequency ) )
    vdie( "Unable to aquire high performance timer" );
  if( !QueryPerformanceCounter( &vclock ) )
    vdie( "Unable to query high performance timer" );
  if( !QueryPerformanceCounter( &vstartClock ) )
    vdie( "Unable to query high performance timer" );

#ifdef DBG
  vseed( 1 );
#else 
  {
    u32 l, h;
    vtimeNonce( &l, &h );
    vseed( l );
  }
#endif

  // setup raw input.
  {
    RAWINPUTDEVICE rid[ 2 ];
    rid[0].usUsagePage = 0x01; 
    rid[0].usUsage = 0x02;  // mouse
    rid[0].dwFlags = 0;
    rid[0].hwndTarget = 0;
    rid[1].usUsagePage = 0x01; 
    rid[1].usUsage = 0x06;  // keyboard
    rid[1].dwFlags = 0;
    rid[1].hwndTarget = 0;
    if( RegisterRawInputDevices( rid, 2, sizeof( RAWINPUTDEVICE ) ) == FALSE )
      vdie( "Failed to register raw input" );
  }

  vscreenWidth = (u32)GetSystemMetrics( SM_CXSCREEN );
  vscreenHeight = (u32)GetSystemMetrics( SM_CYSCREEN );


  {
    WNDCLASS wc;
    wc.hIcon = LoadIcon( GetModuleHandle( NULL ), MAKEINTRESOURCE( BCJICO ) );
    if( wc.hIcon == NULL )
      vdie( "Unable to get icon!" );
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.lpfnWndProc = (WNDPROC)vcallback;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = vinstance;
    wc.hCursor = LoadCursor( NULL, IDC_ARROW );
    wc.hbrBackground = NULL;
    wc.lpszMenuName = NULL;
    wc.lpszClassName = "Vislib";

    if( !RegisterClass( &wc ) )
      vdie( "Couldn't register." );
  }

  vlogWindowParentHandle = CreateWindowEx( 0, "Vislib", "Vislib Log", WS_SIZEBOX, 
    0, 0, 100, 100, NULL, NULL, vinstance, NULL );
  if( vlogWindowParentHandle == NULL )
    vdie( "Couldn't create log parent window." );
  LoadLibrary( "Riched20.dll" );
  vlogWindowHandle = CreateWindowEx( 0, RICHEDIT_CLASS, "", ES_READONLY | WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL | ES_DISABLENOSCROLL, 
    0, 0, 50, 50, vlogWindowParentHandle, NULL, vinstance, NULL );
  if( vlogWindowHandle == NULL )
    vdie( "Couldn't create log window." );
  MoveWindow( vlogWindowParentHandle, logx, logy, logwidth, logheight, TRUE );
  SendMessage( vlogWindowHandle, EM_SETBKGNDCOLOR, (WPARAM)0, (LPARAM)0 );
}

void pvhandleInput( HRAWINPUT hr, int domouse ){
  u32 rsz;
  RAWINPUT* raw;
  u8* lpb;

  GetRawInputData( hr, RID_INPUT, NULL, &rsz, sizeof( RAWINPUTHEADER ) );
  lpb = vsmalloc( rsz );
  if( GetRawInputData( hr, RID_INPUT, lpb, &rsz, sizeof( RAWINPUTHEADER ) ) != rsz ){ 
    vsfree( lpb );
    vdie( "Failed to get raw input." );
  }

  raw = (RAWINPUT*)lpb;
  if( raw->header.dwType == RIM_TYPEMOUSE && domouse ){
    if( raw->data.mouse.usFlags & MOUSE_MOVE_ABSOLUTE ){
      static u32 lx = 536870912;
      static u32 ly = 536870912;
      if( lx == 536870912 && ly == 5368709-12 ){
        lx = raw->data.mouse.lLastX;
        ly = raw->data.mouse.lLastY;
      }
      vmouseXDelta = (f32)raw->data.mouse.lLastX - lx;
      vmouseYDelta = (f32)raw->data.mouse.lLastY - ly;
      lx = raw->data.mouse.lLastX;
      ly = raw->data.mouse.lLastY;
    }else {
      vmouseXDelta = (f32)raw->data.mouse.lLastX;
      vmouseYDelta = (f32)raw->data.mouse.lLastY;
    }
    if( raw->data.mouse.usButtonFlags & RI_MOUSE_WHEEL )
      vmouseWheelDelta = (f32)( ( (s16)raw->data.mouse.usButtonData ) / 120 );
    if( raw->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_1_DOWN ) vmouseButtons[ 0 ] = 1;
    if( raw->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_1_UP ) vmouseButtons[ 0 ] = 0;
    if( raw->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_2_DOWN ) vmouseButtons[ 1 ] = 1;
    if( raw->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_2_UP ) vmouseButtons[ 1 ] = 0;
    if( raw->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_3_DOWN ) vmouseButtons[ 2 ] = 1;
    if( raw->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_3_UP ) vmouseButtons[ 2 ] = 0;
    if( raw->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_4_DOWN ) vmouseButtons[ 3 ] = 1;
    if( raw->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_4_UP ) vmouseButtons[ 3 ] = 0;
    if( raw->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_5_DOWN ) vmouseButtons[ 4 ] = 1;
    if( raw->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_5_UP ) vmouseButtons[ 4 ] = 0;


    if( vactive && GetForegroundWindow() == vwindowHandle ){
      POINT p;
      p.x = (int)vwidth / 2;
      p.y = (int)vheight / 2;
      if( !ClientToScreen( vwindowHandle, &p ) )
        vdie( "Call to ClientToScreen failed." );
      SetCursorPos( p.x, p.y );
    }
  }else if( raw->header.dwType == RIM_TYPEKEYBOARD ){
    if( raw->data.keyboard.Flags & RI_KEY_BREAK ){
      vkeys[ raw->data.keyboard.VKey % 256 ] = FALSE;
      vkeyReleases[ raw->data.keyboard.VKey % 256 ] = TRUE;
    }else {
      if( vkeys[ raw->data.keyboard.VKey % 256 ] == FALSE )
        vkeyPresses[ raw->data.keyboard.VKey % 256 ] = TRUE;
      vkeys[ raw->data.keyboard.VKey % 256 ] = TRUE;
    }
  }
  vsfree( lpb );  
}

LRESULT CALLBACK vcallback( HWND vwh, UINT msg, WPARAM wp, LPARAM lp ){
  if( vwh == vwindowHandle ){ 
    switch( msg ){
    case WM_INPUT:
      pvhandleInput( (HRAWINPUT)lp, 1 );
      return DefWindowProc( vwindowHandle, msg, wp, lp );
    case WM_MOVE:
      vxpos = LOWORD( lp );
      vypos = HIWORD( lp );
      return 0;
    case WM_SIZE:
      vwidth = LOWORD( lp );
      vheight = HIWORD( lp );
      resize( vxpos, vypos, vwidth, vheight );
      return 0;
    case WM_CLOSE:
      PostQuitMessage(0);
      return 0;
    case WM_ACTIVATE:
      if( ( LOWORD(wp) != WA_INACTIVE ) && !( (BOOL)HIWORD(wp) ) ){
        vactivate();
      }else{
        vdeactivate();
      }
      return 0;
    case WM_SYSCOMMAND:
      if( wp == SC_SCREENSAVE || wp == SC_MONITORPOWER )
        return 0;
      break;
    }
    return DefWindowProc( vwindowHandle, msg, wp, lp );
  } else if( vwh == vlogWindowParentHandle || vwh == vlogWindowHandle ){
    if( msg == WM_INPUT )
      pvhandleInput( (HRAWINPUT)lp, 0 );
    else if( msg == WM_SIZE ){
      if( vwh == vlogWindowParentHandle ){
        MoveWindow( vlogWindowHandle, 0, 0, LOWORD( lp ), HIWORD( lp ), TRUE );
        return 0;
      }
    }
    return DefWindowProc( vwh, msg, wp, lp );
  } else{
    return DefWindowProc( vwh, msg, wp, lp );
  }
}

void main( void ){
  vinitInfo* vi;
  MSG msg = { 0 };
  int done = 0;
  int i;

  vinit();
  {

    {
      static u8 mn[ 4100 ] = { 0 };
      u8* cl = GetCommandLineA();
      const u8* rcl = cl;
      u32 v = GetModuleFileName( NULL, mn, 4096 );
      u8* rn = mn;
      if( !v || v == 4096 )
        rcl = NULL;
      mn[ v ] = 0;
      if( mn[ v - 1 ] == '"' )
        mn[ v - 1 ] = 0;
      {
        u8* p = mn;
        while( *p )
          if( *p == '\\' )
            rn = ++p;
          else
            ++p;
      }
      if( rcl != NULL ){
        rcl = vstrstr( cl, rn );
        if( rcl != NULL )
          rcl += strlen( rn );
      }
      if( *rcl == '"' )
        ++rcl;
      while( *rcl == ' ' )
        ++rcl;

      vi = init( rcl );
    }

  }

  if( vi != NULL ){
    MoveWindow( vlogWindowParentHandle, vi->logX, vi->logY, vi->logWidth, vi->logHeight, TRUE );
    if( vi->showLog )
      vshowLogWindow();
    if( vi->logName != NULL ){
      SendMessage( vlogWindowParentHandle, WM_SETTEXT, (WPARAM)0, (LPARAM)vi->logName );
    }
  }

  // Main loop
  while( !done ){
    if( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) ){
      if( msg.hwnd == vlogWindowHandle && msg.message == WM_INPUT )
        msg.hwnd = vlogWindowParentHandle;
      if( msg.message == WM_QUIT ){
        done = 1;
      }else{
        TranslateMessage(&msg);
        DispatchMessage(&msg);
      }
    }else{
      LARGE_INTEGER old;
      old.QuadPart = vclock.QuadPart;
      vntick();
      if( !QueryPerformanceCounter( &vclock ) )
        vdie( "Unable to query high performance timer" );
      if( old.QuadPart == vclock.QuadPart )
        old.QuadPart = vclock.QuadPart - 1;
      {
        f32 dta = (f32)( vclock.QuadPart - old.QuadPart ) / (f32)( vhpFrequency.QuadPart );

        if( vmouseXDelta < maxMouseDelta && vmouseYDelta < maxMouseDelta )
          done = done || tick( dta, vmouseXDelta, vmouseYDelta, vmouseWheelDelta );
        else
          done = done || tick( dta, 0, 0, vmouseWheelDelta );
        vmouseWheelDelta = 0.0;
        if( dta / ( (f32)1000.0 ) < VL_MINIMUM_MILLISECONDS )
          Sleep( (u32)( (f32)VL_MINIMUM_MILLISECONDS - ( dta / ( (f32)1000.0 ) ) ) );
      }
      for( i = 0; i < 256; ++i ){
        vkeyPresses[ i ] = 0;
        vkeyReleases[ i ] = 0;
      }
      vmouseXDelta = 0; vmouseYDelta = 0;
      SwapBuffers( vdc );
    }
  }
  // Main loop end

  cleanup();
  vcleanup();
}

void vmoveWindow( s32 ax, s32 ay, s32 aw, s32 ah ){
  int nx = (int)ax;
  int ny = (int)ay;
  int nw = (int)aw;
  int nh = (int)ah;
  nw = ( nw < minwidth ) ? minwidth : nw;
  nh = ( nh < minheight ) ? minheight : nh;
  nw = ( nw > (int)vscreenWidth ) ? (int)vscreenWidth : nw;
  nh = ( nh > (int)vscreenHeight ) ? (int)vscreenHeight : nh;
  SetWindowPos( vwindowHandle, HWND_TOP, nx, ny, nw, nh, SWP_NOZORDER );
  vxpos = nx;
  vypos = ny;
  vwidth = nw;
  vheight = nh;
  resize( nx, ny, nw, nh );
}
void vactivate( void ){
  SetCapture( vwindowHandle );
  SetFocus( vwindowHandle );
  ShowWindow( vwindowHandle, SW_RESTORE );
  {
    int dc = 0;
    while( dc >= 0 )
      dc = ShowCursor( FALSE );
  }
  vactive = 1;
}
void vdeactivate( void ){
  int i;
  for( i = 0; i < 256; ++i )
    vkeys[ i ] = vkeyReleases[ i ] = vkeyPresses[ i ] = 0;
  for( i = 0; i < 5; ++i )
    vmouseButtons[ i ] = 0;
  {
    int dc = -1;
    while( dc < 0 )
      dc = ShowCursor( TRUE );
  }
  SetFocus( NULL );
  ReleaseCapture();
  SetForegroundWindow( GetDesktopWindow() );
  vactive = 0;
}
void vshowRenderWindow( void ){ ShowWindow( vwindowHandle, SW_SHOW ); }
void vhideRenderWindow( void ){ ShowWindow( vwindowHandle, SW_HIDE ); }
void vshowLogWindow( void ){ ShowWindow( vlogWindowParentHandle, SW_SHOWNA ); }
void vhideLogWindow( void ){ ShowWindow( vlogWindowParentHandle, SW_HIDE ); }

void vgetScreenTex( u8* data ){
  screateSurface( &vscreenTex, GL_RGB8, GL_BGR, vscreenWidth, vscreenHeight, 0, data );
}
void osGetScreenBytes( u8* data ){
  HDC tdc, tcdc;
  HBITMAP tbm;
  u32 stw = vscreenWidth;
  u32 sth = vscreenHeight;
  u32 sz = stw * sth * 3;
  GLsizei tw = (GLsizei)stw;
  GLsizei th = (GLsizei)sth;

  tdc = CreateDC( "DISPLAY", NULL, NULL, NULL ); 
  tcdc = CreateCompatibleDC( tdc ); 
  tbm = CreateCompatibleBitmap( tdc, tw, th ); 
  if( tbm == NULL ) vdie( "Couldn't make bitmap!" );
  if( !SelectObject( tcdc, tbm ) ) vdie( "Couldn't select bitmap!" );
  if( !BitBlt( tcdc, 0, 0, tw, th, tdc, 0, 0, SRCCOPY) ) vdie( "Couldn't screencap!" );
  {
    BITMAPINFO bmi;
    BITMAPINFOHEADER bmih = { 0 }; 

    bmih.biSize = sizeof( BITMAPINFOHEADER );
    bmih.biHeight = th;
    bmih.biWidth = tw;
    bmih.biPlanes = 1;
    bmih.biBitCount = 24;
    bmih.biCompression = BI_RGB;
    bmih.biSizeImage = (int)sz;
    bmih.biClrUsed = 0;
    bmih.biClrImportant = 0;

    bmi.bmiHeader = bmih;

    GetDIBits( tdc, tbm, 0, th, (LPVOID)data, &bmi, DIB_RGB_COLORS );
  } 
  DeleteDC( tcdc );
  DeleteObject( tbm );
  DeleteDC( tdc );
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

// This qsort is based on a public domain version by Nelson Beebe.  Thanks Nelson :D

#define VISLIB_QSORT_THRESHHOLD  4
#define VISLIB_QSORT_MTHRESHHOLD 6

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
  thresh = qsz * VISLIB_QSORT_THRESHHOLD;
  mthresh = qsz * VISLIB_QSORT_MTHRESHHOLD;
  max = base + n * qsz;
  if( n >= VISLIB_QSORT_THRESHHOLD ) {
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

varchive* vnewArchive( void ){
  varchive* ans = vsmalloc( sizeof( varchive ) );
  ans->size = 0;
  ans->buffsize = 1;
  ans->names = vsmalloc( sizeof( u8* ) );
  ans->datas = vsmalloc( sizeof( u8* ) );
  ans->sizes = vsmalloc( sizeof( u32 ) );
  return ans;
}
void vdeleteArchive( varchive* varc ){
  u32 i;
  for( i = 0; i < varc->size; ++i ){
    vsfree( varc->names[ i ] );
    vsfree( varc->datas[ i ] );
  }
  vsfree( varc->names );
  vsfree( varc->datas );
  vsfree( varc->sizes );
  vsfree( varc );
}
void vaddToArchive( varchive* varc, const u8* name, const u8* data, u32 sz ){
  u32 i;
  assess( varc != NULL && name != NULL && data != NULL, "NULL pointer in vaddToArchive!" );
  while( varc->size >= varc->buffsize ){
    u8** tnp = vsmalloc( sizeof( u8* ) * varc->buffsize * 2 );
    u8** tdp = vsmalloc( sizeof( u8* ) * varc->buffsize * 2 );
    u32* tsp = vsmalloc( sizeof( u32 ) * varc->buffsize * 2 );
    varc->buffsize *= 2;
    for( i = 0; i < varc->size; ++i ){
      tnp[ i ] = varc->names[ i ];
      tdp[ i ] = varc->datas[ i ];
      tsp[ i ] = varc->sizes[ i ];
    }
    vsfree( varc->names ); varc->names = tnp;
    vsfree( varc->datas ); varc->datas = tdp;
    vsfree( varc->sizes ); varc->sizes = tsp;
  }
  i = 0;
  while( i < varc->size && ( vstrcmp( name, varc->names[ i ] ) > 0 ) )
    ++i;
  if( i == varc->size ){
    ++varc->size;
    varc->datas[ i ] = vsmalloc( sz );
    varc->names[ i ] = vsmalloc( strlen( name ) + 1 );
    vmemcpy( varc->datas[ i ], data, sz );
    vmemcpy( varc->names[ i ], name, strlen( name ) + 1 );    
    varc->sizes[ i ] = sz;
  }else if( !vstrcmp( name, varc->names[ i ] ) ){
    vsfree( varc->datas[ i ] );
    varc->datas[ i ] = vsmalloc( sz );
    varc->sizes[ i ] = sz;
    vmemcpy( varc->datas[ i ], data, sz );
  }else{
    u32 k;
    for( k = varc->size; k > i; --k ){
      varc->datas[ k ] = varc->datas[ k - 1 ];
      varc->names[ k ] = varc->names[ k - 1 ];
      varc->sizes[ k ] = varc->sizes[ k - 1 ];
    }
    ++varc->size;
    varc->datas[ i ] = vsmalloc( sz );
    varc->names[ i ] = vsmalloc( strlen( name ) + 1 );
    varc->sizes[ i ] = sz;
    vmemcpy( varc->datas[ i ], data, sz );
    vmemcpy( varc->names[ i ], name, strlen( name ) + 1 );
  }
}

// Returns the archives size if the name doesn't exist.
u32 vgetArchiveIndexByName( varchive* varc, const u8* name ){
  u32 g = varc->size / 2;
  u32 m = ( g + 1 ) / 2 + 1;
  int r;
  if( !varc->size )
    return 0;
  while( m && ( 0 != ( r = vstrcmp( name, varc->names[ ( g >= varc->size ? varc->size - 1 : g ) ] ) ) ) ){
    if( r < 0 )
      g -= m;
    else
      g += m;
    m = ( m == 1 ) ? 0 : ( ( m + 1 ) / 2 );
  }
  if( !vstrcmp( name, varc->names[ ( g >= varc->size ? varc->size - 1 : g ) ] ) )
    return ( g >= varc->size ? varc->size - 1 : g );
  else
    return varc->size;
}
u8* vwordToBytes( u32 v ){
  static u8 ans[ 4 ];
  u32 i;
  for( i = 0; i < 4; ++i )
    ans[ i ] = ( v >> ( i * 8 ) ) % 256;
  return ans;
}
u32 vbytesToWord( const u8* p ){
  return p[ 0 ] + p[ 1 ] * 256 + p[ 2 ] * 256 * 256 + p[ 3 ] * 256 * 256 * 256; 
}

u32 varchiveToBytes( varchive* varc ){
  static u32 ans = 0;
  u32 i;

  if( ans )
    verase( ans );
  else
    ans = vmalloc( 0 );

  vappend( ans, vwordToBytes( varc->size ), 4 );

  for( i = 0; i < varc->size; ++i ){
    vappend( ans, vwordToBytes( strlen( varc->names[ i ] ) ), 4 );
    vappend( ans, vwordToBytes( varc->sizes[ i ] ), 4 );
  }

  for( i = 0; i < varc->size; ++i ){
    vappend( ans, varc->names[ i ], strlen( varc->names[ i ] ) );
    vappend( ans, varc->datas[ i ], varc->sizes[ i ] );
  }

  return ans; 

}
varchive* vnewArchiveFromBytes( const u8* data, u32 sz ){
  u32 num, i, sum = 0;
  const u8* bp;
  if( sz < 4 )
    return NULL;

  num = vbytesToWord( data );

  if( num * 8 + 4 > sz )
    return NULL;

  bp = data + num * 8 + 4;
  for( i = 0; i < num * 2; ++i )
    sum += vbytesToWord( data + 4 + i * 4 );
  if( bp + sum != data + sz )
    return NULL;

  { 
    varchive* ans = vnewArchive();
    const u8* cs = data + 4;
    const u8* tp = bp;
    for( i = 0; i < num; ++i ){
      u32 csz = vbytesToWord( cs );
      u32 cdsz = vbytesToWord( cs + 4 );
      u8* tn = vsmalloc( csz + 1 );
      vmemcpy( tn, tp, csz );
      tn[ csz ] = 0;
      vaddToArchive( ans, tn, tp + csz, cdsz );
      cs += 8;
      tp += csz + cdsz;
    }
    return ans;
  }
}
