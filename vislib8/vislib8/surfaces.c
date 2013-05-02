
#include "vislib.h"

// Privates.

void screateSurface( surface* s, GLint internalType, GLenum type, u32 width, u32 height, u32 depth, const void* data ){  
  GLuint tn = 0;
  GLenum st = depth ? GL_TEXTURE_3D : GL_TEXTURE_2D;
  s->width = width;
  s->height = height;
  s->depth = depth;
vglGetError();
  glGenTextures( 1, &tn );
  glBindTexture( st, tn );
  s->name = tn;
vglGetError();

  glTexParameteri( st, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR ); 
  glTexParameteri( st, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
  glTexParameteri( st, GL_TEXTURE_WRAP_S, GL_REPEAT );
  glTexParameteri( st, GL_TEXTURE_WRAP_T, GL_REPEAT );
  if( st == GL_TEXTURE_3D )
    glTexParameteri( st, GL_TEXTURE_WRAP_R, GL_REPEAT );
  glTexParameteri( st, GL_GENERATE_MIPMAP, GL_TRUE );
  vglGetError();

  {
    f32 f;
    glGetFloatv( GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &f );
    vglGetError();

    glTexParameterf( st, GL_TEXTURE_MAX_ANISOTROPY_EXT, f );
    vglGetError();

  }
  if( depth ){
    glTexImage3D( GL_TEXTURE_3D, 0, internalType, width, height, depth, 0, type, ( ( internalType == GL_RGB32F_ARB || internalType == GL_RGBA32F_ARB ) ? GL_FLOAT : GL_UNSIGNED_BYTE ), data );
  }else
    glTexImage2D( GL_TEXTURE_2D, 0, internalType, width, height, 0, type, ( ( internalType == GL_RGB32F_ARB || internalType == GL_RGBA32F_ARB ) ? GL_FLOAT : GL_UNSIGNED_BYTE ), data );
  vglGetError();
}
void rscreateRenderSurface( renderSurface* s, const pixelFormat* pf, u32 width, u32 height, int mipmap, int pingpong ){
  u32 i;
  int attrs[] = { WGL_TEXTURE_FORMAT_ARB, WGL_TEXTURE_RGB_ARB, WGL_TEXTURE_TARGET_ARB, WGL_TEXTURE_2D_ARB, WGL_MIPMAP_TEXTURE_ARB, 1, 0 };
  if( pf->alphaBits )
    attrs[ 1 ] = WGL_TEXTURE_RGBA_ARB;
  attrs[ 5 ] = mipmap ? 1 : 0;

  s->pbs[ 0 ] = wglCreatePbuffer( vdc, pf->format, width, height, attrs );
  if( s->pbs[ 0 ] == NULL ) vdie( "Failed to create pbuffer!" );
  s->dcs[ 0 ] = wglGetPbufferDC( s->pbs[ 0 ] );
  if( s->dcs[ 0 ] == NULL ) vdie( "Failed to get pbuffer device context!" );
  s->rcs[ 0 ] = wglCreateContext( s->dcs[ 0 ] );
  if( s->rcs[ 0 ] == NULL ) vdie( "Failed to create pbuffer render context!" );
  if( pingpong ){
    s->pbs[ 1 ] = wglCreatePbuffer( vdc, pf->format, width, height, attrs );
    if( s->pbs[ 1 ] == NULL ) vdie( "Failed to create pbuffer!" );
    s->dcs[ 1 ] = wglGetPbufferDC( s->pbs[ 1 ] );
    if( s->dcs[ 1 ] == NULL ) vdie( "Failed to get pbuffer device context!" );
    s->rcs[ 1 ] = wglCreateContext( s->dcs[ 1 ] );
    if( s->rcs[ 1 ] == NULL ) vdie( "Failed to create pbuffer render context!" );
  }
  {
    int w, h;
	  wglQueryPbuffer( s->pbs[ 0 ], WGL_PBUFFER_WIDTH_ARB, &w );
	  wglQueryPbuffer( s->pbs[ 0 ], WGL_PBUFFER_HEIGHT_ARB, &h );
    if( w != (int)width || h != (int)height )
      vdie( "Failed to create pbuffer of the correct dimensions!" );
    if( pingpong ){
	    wglQueryPbuffer( s->pbs[ 1 ], WGL_PBUFFER_WIDTH_ARB, &w );
	    wglQueryPbuffer( s->pbs[ 1 ], WGL_PBUFFER_HEIGHT_ARB, &h );
      if( w != (int)width || h != (int)height )
        vdie( "Failed to create pbuffer of the correct dimensions!" );
    }
  }


  s->width = width;
  s->height = height;
  s->pf = *pf;
  s->mipmap = mipmap ? 1 : 0;
  s->pingpong = pingpong ? 1 : 0;
  s->front = 0;

  for( i = 0; i < 32; ++i )
    s->names[ 0 ][ i ] = 0;
  initShaders( &s->shads[ 0 ] );
  initPrograms( &s->progs[ 0 ] );
  wglShareLists( vrc, s->rcs[ 0 ] );
  if( pingpong ){
    for( i = 0; i < 32; ++i )
      s->names[ 1 ][ i ] = 0;
    initShaders( &s->shads[ 1 ] );
    initPrograms( &s->progs[ 1 ] );
    wglShareLists( vrc, s->rcs[ 1 ] );
  }
}
void rsdestroyRenderSurface( renderSurface* s ){
  destroyShaders( &s->shads[ 0 ] );
  destroyPrograms( &s->progs[ 0 ] );
  wglReleasePbufferDC( s->pbs[ 0 ], s->dcs[ 0 ] );
  wglDestroyPbuffer( s->pbs[ 0 ] );
  if( s->pingpong ){
    destroyShaders( &s->shads[ 1 ] );
    destroyPrograms( &s->progs[ 1 ] );
    wglReleasePbufferDC( s->pbs[ 1 ], s->dcs[ 1 ] );
    wglDestroyPbuffer( s->pbs[ 1 ] );
  }
}
void rsrenderToSurface( renderSurface* s ){
  if( s->pingpong && s->front )
    wglMakeCurrent( s->dcs[ 1 ], s->rcs[ 1 ] );
  else
    wglMakeCurrent( s->dcs[ 0 ], s->rcs[ 0 ] );
}
void rsstopSurfaceRendering( renderSurface* s ){
  if( s->pf.doubleBuffer )
    SwapBuffers( s->dcs[ s->front ? 1 : 0 ] );
  wglMakeCurrent( vdc, vrc );
}
void rsflip( renderSurface* s ){
  s->front = !s->front;
}
void rsaddShaderFromResourceOrDie( renderSurface* s, u32 rsc, const u8* name, int type ){
  HDC tdc = wglGetCurrentDC();
  HGLRC trc = wglGetCurrentContext();
  wglMakeCurrent( s->dcs[ 0 ], s->rcs[ 0 ] );
  addShaderFromResourceOrDie( &s->shads[ 0 ], rsc, name, type );
  if( s->pingpong ){
    wglMakeCurrent( s->dcs[ 1 ], s->rcs[ 1 ] );
    addShaderFromResourceOrDie( &s->shads[ 1 ], rsc, name, type );
  }
  wglMakeCurrent( tdc, trc );
}
void rsaddShaderFromArchiveOrDie( renderSurface* s, varchive* varc, const u8* name, int type ){
  HDC tdc = wglGetCurrentDC();
  HGLRC trc = wglGetCurrentContext();
  wglMakeCurrent( s->dcs[ 0 ], s->rcs[ 0 ] );
  addShaderFromArchiveOrDie( &s->shads[ 0 ], varc, name, type );
  if( s->pingpong ){
    wglMakeCurrent( s->dcs[ 1 ], s->rcs[ 1 ] );
    addShaderFromArchiveOrDie( &s->shads[ 1 ], varc, name, type );
  }
  wglMakeCurrent( tdc, trc );
}
void rscreateProgramOrDie( renderSurface* s, u32 cnt, const u8** shds, const u8* name ){
  HDC tdc = wglGetCurrentDC();
  HGLRC trc = wglGetCurrentContext();
  wglMakeCurrent( s->dcs[ 0 ], s->rcs[ 0 ] );
  createProgramOrDie( &s->progs[ 0 ], &s->shads[ 0 ], cnt, shds, name );
  if( s->pingpong ){
    wglMakeCurrent( s->dcs[ 1 ], s->rcs[ 1 ] );
    createProgramOrDie( &s->progs[ 1 ], &s->shads[ 1 ], cnt, shds, name );
  }
  wglMakeCurrent( tdc, trc );
}
GLint rsGetProgram( renderSurface* s, const u8* name ){
  return getProgramByName( &s->progs[ s->front ? 1 : 0 ], name );
}
void rsbindRenderSurface( renderSurface* s, GLuint textureUnit, int nice, int buffer ){
  f32 f;
  GLint n;
  if( s->pingpong && !s->front )
    n = 1;
  else
    n = 0;
  if( !s->names[ n ][ buffer - WGL_FRONT_LEFT_ARB ] ){
    glGenTextures( 1, &s->names[ n ][ buffer - WGL_FRONT_LEFT_ARB ] );
    glBindTexture( GL_TEXTURE_2D, s->names[ n ][ buffer - WGL_FRONT_LEFT_ARB ] );						
    glTexImage2D( GL_TEXTURE_2D, 0, 4, s->width, s->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL ); 
  }
  glActiveTexture( GL_TEXTURE0_ARB + textureUnit );
  glEnable( GL_TEXTURE_2D );    
  glBindTexture( GL_TEXTURE_2D, s->names[ n ][ buffer - WGL_FRONT_LEFT_ARB ] );
  wglBindTexImage( s->pbs[ n ], buffer );
  glEnable( GL_TEXTURE_2D );
  if( nice ){
    if( s->mipmap )
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR ); 
    else
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR ); 
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    if( s->mipmap ){
      glTexParameteri( GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE );
      glGetFloatv( GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &f );
      glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, f );
    }
  }else{
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST ); 
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.0f );
  }
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
}
void rsunbindRenderSurface( const renderSurface* s, int buffer ){
  wglReleaseTexImage( s->pbs[ ( s->pingpong && !s->front ) ? 1 : 0 ], buffer );
}
void sbind( const surface* s, GLuint textureUnit, int nice ){
  f32 f;
  GLenum st = s->depth ? GL_TEXTURE_3D : GL_TEXTURE_2D;
  glActiveTexture( GL_TEXTURE0_ARB + textureUnit );
  glBindTexture( st, s->name );
  glEnable( st );
  if( nice ){
    glTexParameteri( st, GL_GENERATE_MIPMAP, GL_TRUE );
    glTexParameteri( st, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR ); 
    glTexParameteri( st, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glGetFloatv( GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &f );
    glTexParameterf( st, GL_TEXTURE_MAX_ANISOTROPY_EXT, f );
  }else{
    glTexParameteri( st, GL_GENERATE_MIPMAP, GL_FALSE );
    glTexParameteri( st, GL_TEXTURE_MIN_FILTER, GL_NEAREST ); 
    glTexParameteri( st, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameterf( st, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.0f );
  }
  glTexParameteri( st, GL_TEXTURE_WRAP_S, GL_REPEAT );
  glTexParameteri( st, GL_TEXTURE_WRAP_T, GL_REPEAT );
  if( st == GL_TEXTURE_3D )
    glTexParameteri( st, GL_TEXTURE_WRAP_R, GL_REPEAT );
}