#include "vislib.h"
#include "rc.h"

#define DESIRED_WIDTH 600
#define DESIRED_HEIGHT 600
#define DESIRED_X 100
#define DESIRED_Y 50


#ifdef DBG
#define DIM3 256
#define TILESX 16
#define TILESY 16
#define DIMX ( DIM3 * TILESX )
#define DIMY ( DIM3 * TILESY )
#else
#define DIM3 256
#define TILESX 16
#define TILESY 16
#define DIMX ( DIM3 * TILESX )
#define DIMY ( DIM3 * TILESY )
#endif

typedef struct{
  int fpsing;				            		 // nonzero if we are showing fps every second

  f32 aspect;                        // aspect ratio of window

  f32 fov;                           // field of view
  f32 nearclip;                      // near clip plane
  f32 farclip;                       // far clip plane

  triple pos;                        // view position
  triple rot;                        // view rotation
  triple rotd;                       // view rotation delta
  int der;                           // relative rotation

  triple lightpos[ 3 ];              // light positions
  triple lightcolor[ 3 ];            // light colors
  int curlight;                      // current light

  int shader;                        // which shader to use.
  int fullscreen;                    // fullscreen

  f32 tweak[ 4 ];                    // generic tweak value

  const pixelFormat* rgba32format;   // f32 pixel format

  varchive* varc;                    // The archive

  shaders shads;
  programs progs;

  renderSurface rs;

  pair windowDim;                    // window dimensions
  pair windowPos;                    // window poistion
  pair oldWindowPos;
  pair oldWindowDim;                 // old ones;

} rstate;

static rstate initRState = {
  1,							                	 // nonzero if we are showing fps every second
  1.0f,                              // aspect ratio of window

  90.0f,                             // field of view
  0.1f,                              // near clip plane
  1000.0f,                           // far clip plane

  { 0.0f, 0.0f, -3.0f },             // view position
  { 0.0f, 0.0f, 0.0f },              // view rotation
  { 0.0f, 0.0f, 0.0f },              // view rotation delta
  0,                                 // relative rotation

  { 0 },                             // light positions
  { 0 },                             // light colors
  0,                                 // current light

  0,                                 // which shader to use.
  0,                                 // fullscreen

  { 0.0f, 0.0f, 0.0f, 2.2f },        // generic tweak value

  NULL,                              // f32 pixel format
  NULL,                              // The archive
}; 

static rstate grs;

#include "draw.c"
void setInitRState( void ){
  initRState.windowDim.x = DESIRED_WIDTH;
  initRState.windowDim.y = DESIRED_HEIGHT;
  initRState.windowPos.x = DESIRED_X;
  initRState.windowPos.y = DESIRED_Y;
  initRState.oldWindowPos.x = initRState.oldWindowPos.y = 50;
  initRState.oldWindowDim.x = 320;
  initRState.oldWindowDim.y = 200;
  initRState.aspect = (f32)initRState.windowDim.x / (f32)initRState.windowDim.y;
  initRState.rgba32format = vglFindPixelFormat( 32, -1, 32, -1, 32, -1, 32, -1, 0, -1, 0, -1, 0, -1, 0, -1, 0, 1, 0, 0 );
  initRState.lightpos[ 0 ].x = 5.0f; initRState.lightpos[ 0 ].y = 5.0f; initRState.lightpos[ 0 ].z = 1.0f; initRState.lightcolor[ 0 ].x = 1.0f; initRState.lightcolor[ 0 ].y = 1.0f; initRState.lightcolor[ 0 ].z = 1.0f;
  initRState.lightpos[ 1 ].x = 5.0f; initRState.lightpos[ 1 ].y = 5.0f; initRState.lightpos[ 1 ].z = -1.0f; initRState.lightcolor[ 1 ].x = 0.8f; initRState.lightcolor[ 1 ].y = 1.0f; initRState.lightcolor[ 1 ].z = 0.6f;
  initRState.lightpos[ 2 ].x = -5.0f; initRState.lightpos[ 2 ].y = 5.0f; initRState.lightpos[ 2 ].z = -1.0f; initRState.lightcolor[ 2 ].x = 0.5f; initRState.lightcolor[ 2 ].y = 1.0f; initRState.lightcolor[ 2 ].z = 1.0f;
  if( initRState.rgba32format == NULL )
    vdie( "Unable to find rgba32 pixel format!" );
  rscreateRenderSurface( &initRState.rs, initRState.rgba32format, DIMX, DIMY, 0, 1 );
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
}
void resetRState( void ){
  shaders s = grs.shads;
  programs p = grs.progs;
  renderSurface rs = grs.rs;
  grs = initRState;
  grs.progs = p;
  grs.shads = s;
  grs.rs = rs;
  vmoveWindow( (s32)grs.windowPos.x, (s32)grs.windowPos.y, (s32)grs.windowDim.x, (s32)grs.windowDim.y );
}
void resize( u32 x, u32 y, u32 width, u32 height ){
  if( grs.rs.height ){
    //rsrenderToSurface( &grs.rs ); glViewport( 0, 0, (GLsizei)( width + rttrat - 1 ) / rttrat, (GLsizei)( height + rttrat - 1 ) / rttrat ); rsstopSurfaceRendering( &grs.rs ); rsflip( &grs.rs );
    //rsrenderToSurface( &grs.rs ); glViewport( 0, 0, (GLsizei)( width + rttrat - 1 ) / rttrat, (GLsizei)( height + rttrat - 1 ) / rttrat ); rsstopSurfaceRendering( &grs.rs ); rsflip( &grs.rs );
  }
  glViewport( 0, 0, (GLsizei)width, (GLsizei)height );	
  grs.aspect = ( (f32)width / (f32)( height?height:1 ) );
}
int tick( f32 delta, f32 xdelta, f32 ydelta, f32 mwheelDelta ){
  static f32 ttime, afps;
  static int inited = 0;
#ifdef DBG
  vglGetError(); 
#endif


  if( grs.fpsing ){
    static f32 lt = 0; 
    lt += delta;
    if( lt > 1 ){ 
      lt -= 1.0f;
      vlog( "FPS: " );
      vlog( vintToString( (int)afps, 0 ) );
      vlog( "." );
      vlog( vintToString( ( (int)( afps * 1000 ) ) % 1000, 3 ) );
      vlog( "\n" );
    }
  }

  if( !inited ){
    ttime = 0;
    afps = 0;
    inited = 1;
    setInitRState();
    grs = initRState;
    initShaders( &grs.shads );
    initPrograms( &grs.progs );
    drawinit( &grs, 0 );
    vshowRenderWindow();
  } else{
    ttime += delta;
    if( delta > 0 ){
      afps += ( 1.0f / delta - afps ) * 0.125f;
    }
  }


#ifdef DBG
  vglGetError();
#endif
  if( vkeys[ 'M' ] ){
    if( !grs.fullscreen ){
      if( vmouseButtons[ VL_LBUTTON ] ){
        grs.windowDim.x += xdelta * 0.25f;
        grs.windowDim.y += ydelta * 0.25f;
      }else{
        grs.windowPos.x += xdelta * 0.25f;
        grs.windowPos.y += ydelta * 0.25f;
      }
    }
    vmoveWindow( (s32)grs.windowPos.x, (s32)grs.windowPos.y, (s32)grs.windowDim.x, (s32)grs.windowDim.y );
  }else if( vkeys[ 'V' ] ){
    grs.fov += ydelta * 0.025f;
    if( grs.fov < 5.0f ) 
      grs.fov = 5.0f;
    else if( grs.fov > 179.5f )
      grs.fov = 179.5f;
  }else if( vkeys[ '1' ] )
    grs.tweak[ 0 ] += ydelta * 0.025f;
  else if( vkeys[ '2' ] )
    grs.tweak[ 1 ] += ydelta * 0.025f;
  else if( vkeys[ '3' ] )
    grs.tweak[ 2 ] += ydelta * 0.025f;
  else if( vkeys[ '4' ] )
    grs.tweak[ 3 ] += ydelta * 0.025f;
  else if( !grs.curlight ){
    if( vmouseButtons[ VL_LBUTTON ] ){
      grs.pos.x += xdelta * -0.03f;
      grs.pos.y += ydelta * 0.03f;
    }else if( vmouseButtons[ VL_RBUTTON ] ){
      grs.pos.z += ydelta * -0.03f;
    }else if( vmouseButtons[ VL_MBUTTON ] ){
      if( grs.der ){
        grs.rotd.z += xdelta * (f32)0.25;
      }else{
        grs.rot.z += xdelta * (f32)0.25;
      }
    }else{
      if( grs.der ){
        grs.rotd.x += xdelta * (f32)0.25;
        grs.rotd.y += ydelta * (f32)0.25;
      }else{
        grs.rot.x += xdelta * (f32)0.25;
        grs.rot.y += ydelta * (f32)0.25;
      }
      grs.pos.z += mwheelDelta * 3.0f;
    }
  }else{
    if( vmouseButtons[ VL_LBUTTON ] ){
      grs.lightpos[ grs.curlight - 1 ].z += ydelta *-0.03f;
    }else{
      grs.lightpos[ grs.curlight - 1 ].x += xdelta *-0.03f;
      grs.lightpos[ grs.curlight - 1 ].y += ydelta *-0.03f;
    }
  }

  if( grs.der ){
    grs.rot.x += grs.rotd.x * delta * (f32)0.3;
    grs.rot.y += grs.rotd.y * delta * (f32)0.3;
    grs.rot.z += grs.rotd.z * delta * (f32)0.3;
  } else{   
    grs.rotd.x = 0;
    grs.rotd.y = 0;
    grs.rotd.z = 0;
  }  
  while( grs.rot.x > 360.0f )
    grs.rot.x -= 360.0f;
  while( grs.rot.y > 360.0f )
    grs.rot.y -= 360.0f;
  while( grs.rot.x < 0.0f )
    grs.rot.x += 360.0f;
  while( grs.rot.y < 0.0f )
    grs.rot.y += 360.0f;

#ifdef DBG
  vglGetError();
#endif
  if( vkeyPresses[ '0' ] )
    vlog( "foooo\n" );
  if( vkeyPresses[ 'N' ] ){
    int i;
    for( i = -1000; i < 1000; ++i ){
      vlog( vintToString( i, 0 ) );
      vlog( ": " );
      vlog( vintToName( i ) );
      vlog( "\n" );
    }
  }
  if( vkeyPresses[ '5' ] ) 
    vshowRenderWindow();

  if( vkeyPresses[ '9' ] ) 
    vlogWarning( "fooas\nfassfoo\n" );
  if( vkeyPresses[ '8' ] ) 
    vlogError( "f\ndfas\nasdasfasdf\nsadoooo\n" );
  if( vkeyPresses[ 'L' ] ){
    grs.curlight = ( grs.curlight + 1 ) % 4;
  }
  if( vkeyPresses[ 'X' ] ){
    if( !grs.fullscreen ){
      grs.fullscreen = 1;
      grs.oldWindowPos = grs.windowPos;
      grs.oldWindowDim = grs.windowDim;
      grs.windowPos.x = grs.windowPos.y = 0;
      grs.windowDim.x = (f32)vscreenWidth;
      grs.windowDim.y = (f32)vscreenHeight;
    }else{
      grs.windowPos = grs.oldWindowPos;
      grs.windowDim = grs.oldWindowDim;
      grs.fullscreen = 0;
    }

    vmoveWindow( (s32)grs.windowPos.x, (s32)grs.windowPos.y, (s32)grs.windowDim.x, (s32)grs.windowDim.y );
  }
  if( vkeyPresses[ 'F' ] )
    grs.shader = ( grs.shader + 1 ) % 2;
  if( vkeyPresses[ 'R' ] )
    grs.der = !grs.der;
  if( vkeyPresses[ 'I' ] )
    vinform( vsysInfo(), "Sytem Information" );
  if( vkeyPresses[ 'E' ] ){
    vinform( vglExtensions(), "OpenGL Extensions" );
  }
  if( vkeyPresses[ VK_SPACE ] ){
    grs.fpsing = !grs.fpsing;
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
    resetRState();
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
vinitInfo* init( const u8* cl ){
  static vinitInfo vi;
  vi.logX = DESIRED_X + DESIRED_WIDTH + 1;
  vi.logY = DESIRED_Y;
  vi.logWidth = 200;
  vi.logHeight = DESIRED_HEIGHT;
  vi.showLog = 1;
  vi.logName = "Super Sprites log";
  vginit( "Super Sprites", DESIRED_WIDTH, DESIRED_HEIGHT, DESIRED_X, DESIRED_Y, 32, 1 );
  return &vi;
}

void cleanup( void ){
  drawcleanup( &grs );
  if( grs.varc != NULL )
    vdeleteArchive( grs.varc );
  rsdestroyRenderSurface( &grs.rs );
  destroyShaders( &grs.shads );
  destroyPrograms( &grs.progs );
}
