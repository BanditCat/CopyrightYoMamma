#include "vislib.h"
#include "rc.h"

#ifdef DBG
#define divscreen 4
#define offset 20
#else
#define divscreen 1
#define offset 0
#endif
#define DESIRED_WIDTH 1000
#define DESIRED_HEIGHT 1000
#define DESIRED_X 100
#define DESIRED_Y 50

typedef struct{
  varchive* varc;              // the archive

  int palrot;                  // palette rotation
  f32 prpos;                   // palette rotation position
  f32 prspeed;                 // palette rotation speed
  f32 pscale;                  // palette scale
  int psel;                    // palette seloctor

  f32 aspect;                  // aspect ratio
  f32 nearclip;                // near clip plane
  f32 farclip;                 // far clip plane

  int fmode;                   // mandelbrot or julia

  f32 xc, yc;                  // center
  f32 m;                       // scale
  f32 jx, jy;                  // julia param
  f32 px, py;                  // peturb

  int shader;                  // draw style

  int autoredraw;              // automatically redraw when moving around.

  f32 bailout;                 // bailout

  f64 minx, miny, maxx, maxy;  // current visble portion
  
  pair windowDim;              // window dimensions
  pair windowPos;              // window poistion

  shaders shads;
  programs progs;
} rstate;

static rstate initRState = {
  NULL,                        // the archive

  0,                           // palette rotation
  0,                           // palette rotation position
  0.5f,                        // palette rotation speed
  5.0f,                        // palette scale
  0,                           // palette seloctor

  0,                           // aspect ratio
  0.1f,                        // near clip plane
  1000.0f,                     // far clip plane

  0,                           // mandelbrot or julia

  0, 0,                        // center
  1.0f,                        // scale
  -0.2f, -0.7f,                // julia param
  0.0f, 0.0f,                  // peturb

  0,                           // draw style


  1,                           // automatically redraw when moving around.

  4.0f,                        // bailout
}; 

static rstate grs;

#include "draw.c"



void resize( u32 x, u32 y, u32 width, u32 height ){
  glViewport( 0, 0, (GLsizei)width, (GLsizei)height);	
}

int tick( f32 delta, f32 xdelta, f32 ydelta, f32 mousewheelDelta ){
  static int inited = 0;
#ifdef DBG
  vglGetError();
#endif
  grs.aspect = ( (f32)vwidth / (f32)( vheight?vheight:1 ) );

  if( !inited ){
    inited = 1;
    initRState.windowDim.x = DESIRED_WIDTH;
    initRState.windowDim.y = DESIRED_HEIGHT;
    initRState.windowPos.x = DESIRED_X;
    initRState.windowPos.y = DESIRED_Y;
    initRState.aspect = (f32)initRState.windowDim.x / (f32)initRState.windowDim.y;
    initRState.minx = -0.5f - 1.5f * grs.aspect;
    initRState.miny = -1.5f; 
    initRState.maxx = -0.5f + 1.5f * grs.aspect;
    initRState.maxy = 1.5f;
    {
      u32 vs;
      u32 dcm;
      const u8* dt = vgetResourceOrDie( VARC, &vs, "VARCHIVE" );
      dcm = vdecompress( dt, vs );
      initRState.varc = vnewArchiveFromBytes( vmem( dcm ), vsize( dcm ) );
      verase( dcm );
      if( initRState.varc == NULL )
        vdie( "Unable to load archive!" );
    }
    grs = initRState;
    drawinit( &grs );
  }


#ifdef DBG
  vglGetError();
#endif

  if( vkeys[ 'M' ] ){
    if( vmouseButtons[ VL_LBUTTON ] ){
      grs.windowDim.x += xdelta * 0.25f;
      grs.windowDim.y += ydelta * 0.25f;
    }else{
      grs.windowPos.x += xdelta * 0.25f;
      grs.windowPos.y += ydelta * 0.25f;
    }
    vmoveWindow( (u32)grs.windowPos.x, (u32)grs.windowPos.y, (u32)grs.windowDim.x, (u32)grs.windowDim.y );
  }else if( vkeys[ 'B' ] ){
    grs.bailout += ydelta * 0.0025f * grs.bailout;
    drawRTT( &grs, 1 );
    drawRTT( &grs, 0 ); drawRTT( &grs, 0 ); drawRTT( &grs, 0 ); drawRTT( &grs, 0 ); drawRTT( &grs, 0 );
  }else if( vkeys[ 'S' ] ){
    if( vmouseButtons[ VL_LBUTTON ] ){
      grs.pscale += ydelta * 0.00025f * grs.pscale;
    }else if( vmouseButtons[ VL_RBUTTON ] ){
      grs.pscale += ydelta * 0.0000025f * grs.pscale;
    }else{
      *( grs.palrot ? &grs.prspeed : &grs.prpos ) += ydelta * 0.002f;
    }
  }else if( vkeys[ 'U' ] ){
    grs.px += xdelta * (f32)( grs.maxy - grs.miny ) * 0.00001f;
    grs.py += ydelta * (f32)( grs.maxy - grs.miny ) * 0.00001f;
    drawRTT( &grs, 1 ); drawRTT( &grs, 0 ); drawRTT( &grs, 0 ); drawRTT( &grs, 0 ); drawRTT( &grs, 0 );
  }else if( vmouseButtons[ VL_LBUTTON ] ){
    grs.m += ydelta * grs.m * (f32)0.003;
  }else if( vmouseButtons[ VL_RBUTTON ] ){
    grs.jx += xdelta * (f32)( grs.maxy - grs.miny ) * 0.00001f;
    grs.jy += ydelta * (f32)( grs.maxy - grs.miny ) * 0.00001f;
    drawRTT( &grs, 1 ); drawRTT( &grs, 0 ); drawRTT( &grs, 0 ); drawRTT( &grs, 0 ); drawRTT( &grs, 0 );
  }else{
    grs.xc += xdelta * grs.m * 0.003f;
    grs.yc += ydelta * grs.m * 0.003f;
  }

#ifdef DBG
  vglGetError();
#endif

  if( grs.palrot ){
    grs.prpos += grs.prspeed * delta * 0.3f;
  }  

#ifdef DBG
  vglGetError();
#endif

  if( vkeyPresses[ '1' ] ) 
    vactivate();
  if( vkeyPresses[ '2' ] ) 
    vdeactivate();
  if( vkeyPresses[ 'P' ] ){
    grs.psel = ( grs.psel + 1 ) % numpalettes;
  }
  if( vkeyPresses[ 'T' ] )
    grs.shader = ( grs.shader + 1 ) % 4;
  if( vkeyPresses[ 'R' ] ) 
    grs.palrot = !grs.palrot;
  if( vkeyPresses[ 'A' ] ) 
    grs.autoredraw = !grs.autoredraw;
  if( vkeyPresses[ ' ' ] ){
    static f64 ominx, ominy, omaxx, omaxy;
    grs.fmode = !grs.fmode;
    drawRTT( &grs, 1 );
    if( grs.fmode ){
      ominx = grs.minx; omaxx = grs.maxx;
      ominy = grs.miny; omaxy = grs.maxy;
      grs.jx = (f32)( ( grs.minx + grs.maxx ) / 2.0 );
      grs.jy = (f32)( ( grs.miny + grs.maxy ) / 2.0 );
    }else{
      grs.minx = ominx; grs.maxx = omaxx;
      grs.miny = ominy; grs.maxy = omaxy;
    }
    drawRTT( &grs, 1 );
  }if( vkeyPresses[ 'X' ] ){
    static int fullscreen = 0;
    static pair oldpos, olddim;
    if( !fullscreen ){
      fullscreen = 1;
      oldpos = grs.windowPos;
      olddim = grs.windowDim;
      grs.windowPos.x = grs.windowPos.y = 0;
      grs.windowDim.x = (f32)vscreenWidth; grs.windowDim.y = (f32)vscreenHeight;
    }else{
      fullscreen = 0;
      grs.windowDim = olddim;
      grs.windowPos = oldpos;
    }
    vmoveWindow( (u32)grs.windowPos.x, (u32)grs.windowPos.y, (u32)grs.windowDim.x, (u32)grs.windowDim.y );
  }
  if( vkeyPresses[ 'I' ] )
    vinform( vsysInfo(), "Sytem Information" );
  if( vkeyPresses[ 'E' ] ){
    vinform( vglExtensions(), "OpenGL Extensions" );
  }
  if( vkeyPresses[ 'W' ] ){
    static u32 msg = 0;
    u32 i;
    if( !msg ){
      msg = vmalloc( 1 );
      vappendString( msg, "System Information:\n" );
      vappendString( msg, vsysInfo() );
      vappendString( msg, "\n\n\nOpenGL Extensions:\n" );
      vappendString( msg, vglExtensions() );
    }
    for( i = 0; i < grs.shads.size; ++i ){
      vappendString( msg, "\n\n" );
      vappendString( msg, vmem( grs.shads.usages[ i ] ));
    }
    vappendString( msg, "\n\nPixel format information for the " );
    vappendInt( msg, vglNumPixelFormats(), 0 );
    vappendString( msg, " valid formats:\n\n\n" );
    vappendString( msg, vglPixelFormatsString() );
    if( vwriteFile( "information.txt", vmem( msg ), vstrlen( vmem( msg ) ) ) )
      vinform( "Succesfully wrote information.txt", "Success" );
    else
      vinform( "Failed to write information.txt", "Failure" );
  }
  if( vkeyPresses[ 'C' ] ){
    shaders s = grs.shads;
    programs p = grs.progs;
    grs = initRState;
    grs.progs = p;
    grs.shads = s;
    vmoveWindow( (u32)grs.windowPos.x, (u32)grs.windowPos.y, (u32)grs.windowDim.x, (u32)grs.windowDim.y );
    drawRTT( &grs, 1 );
  }
  { 
    static f64 wait, om, oxc, oyc;
    static int redrawn;
    if( om != grs.m || oxc != grs.xc || oyc != grs.yc ){
      om = grs.m;
      oxc = grs.xc;
      oyc = grs.yc;
      wait = 0.5;
      redrawn = 0;
    }else
      wait -= (f64)delta;
    if( grs.autoredraw && ( ( !redrawn && wait < 0 ) || grs.m > 2.0 || grs.m < 0.5 || vabs( grs.xc ) > 0.25 || vabs( grs.yc ) > 0.25 ) ){
      drawRTT( &grs, 1 );
      om = grs.m;
      oxc = grs.xc;
      oyc = grs.yc;
      wait = 0.5;
      redrawn = 1;
    }
  }

#ifdef DBG
  vglGetError();
#endif

  draw( &grs );

#ifdef DBG
  vglGetError();
#endif
  return vkeys[ VK_ESCAPE ];
}
void cleanup( void ){
  drawcleanup( &grs );
  if( grs.varc != NULL )
    vdeleteArchive( grs.varc );
}
vinitInfo* init( const u8* cl ){
  static vinitInfo vi;
  vi.logX = vi.logY = vi.logWidth = vi.logHeight = 100;
  vi.logName = "Fract log";
  vi.showLog = 0;
  vginit( "Fract", DESIRED_WIDTH, DESIRED_HEIGHT, DESIRED_X, DESIRED_Y, 32, 1 );
  return &vi;
}