#define vsync 0
#define numspheres 32
#define odim 1024



surface black, spheres, threed, jpg;
static const u8 blackd[] = { 0, 0, 0 };

static f32 sphered[ ( numspheres + 1 ) * 8 ] = { 0.0f };

void initgl( rstate* rs ){
  glShadeModel( GL_SMOOTH );
  glEnable( GL_CULL_FACE );
  glColorMask( 1, 1, 1, 1 );
  glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
  glEnable( GL_DEPTH_TEST );
  glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST );
  glDisable( GL_BLEND );
  glDepthMask( 1 );
  glClear( GL_COLOR_BUFFER_BIT );
}

void drawinit( rstate* rs, int pass ){
  const u8* pfg[] = { "vsprite", "fbg" };
  const u8* prtt[] = { "vstandard", "frtt" };
  const u8* prttinit[] = { "vstandard", "frttinit" };
  vglSwapControl( vsync );
  initgl( rs );
  rsrenderToSurface( &grs.rs ); initgl( rs ); glViewport( 0, 0, DIMX, DIMY ); rsflip( &grs.rs ); rsstopSurfaceRendering( &grs.rs );
  rsrenderToSurface( &grs.rs ); initgl( rs ); glViewport( 0, 0, DIMX, DIMY ); rsflip( &grs.rs ); rsstopSurfaceRendering( &grs.rs );
  addShaderFromArchiveOrDie( &rs->shads, rs->varc, "vsprite", GL_VERTEX_SHADER );
  addShaderFromArchiveOrDie( &rs->shads, rs->varc, "fbg", GL_FRAGMENT_SHADER );
  createProgramOrDie( &rs->progs, &rs->shads, 2, pfg, "fg" );
  screateSurface( &black, GL_RGB8, GL_RGB, 1, 1, 0, blackd );
  
  {
    u32 w, h;
    u32 jpgind;
    u8* jpdt;
    
    jpgind = vgetArchiveIndexByName( grs.varc, "dirtHeight" );
    if( jpgind == grs.varc->size )
      vdie( "Unable to find texture in archive" );
    
    if( !vloadImageFromBytes( grs.varc->datas[ jpgind ], grs.varc->sizes[ jpgind ], &jpdt, &w, &h ) )
      vdie( "Unable to load texture from internal archive!" );
    
    screateSurface( &jpg, GL_RGB8, GL_RGB, w, h, 0, jpdt );
    vsfree( jpdt );
  }

  {
    u32 i;
    triple c;
    for( i = 0; i < numspheres; ++i ){
      u32 k;
      int redo = 1;
      sphered[ i * 4 + 3 ] = vrand( 0.1f, 0.5f );
      while( redo ){
        f32 md = 1 - sphered[ i * 4 + 3 ];
        for( k = 0; k < 3; ++k )
          sphered[ i * 4 + k ] = vrand( -1 * md, md );
        redo = 0;
        for( k = 0; k < i; ++k ){
          f32 r = sphered[ i * 4 + 3 ] + sphered[ k * 4 + 3 ];
          f32 x = sphered[ i * 4 + 0 ] - sphered[ k * 4 + 0 ];
          f32 y = sphered[ i * 4 + 1 ] - sphered[ k * 4 + 1 ];
          f32 z = sphered[ i * 4 + 2 ] - sphered[ k * 4 + 2 ];
          if( x * x + y * y + z * z < r * r )
            redo = 1;
        }
        if( redo ){
          sphered[ i * 4 + 3 ] *= 0.9f;
          sphered[ i * 4 + 1 ] = sphered[ i * 4 + 3 ] - 4.0f;
        }
      }
    }
    for( i = 0; i <= numspheres; ++i ){
      c.x = (f32)i * ( 360.0f / ( numspheres + 1 ) ); c.y = 1.0f; c.z = 1.0f; vhsvToRgb( &c );
      sphered[ i * 4 + ( numspheres + 1 ) * 4 + 0 ] = c.x;
      sphered[ i * 4 + ( numspheres + 1 ) * 4 + 1 ] = c.y;
      sphered[ i * 4 + ( numspheres + 1 ) * 4 + 2 ] = c.z;
      sphered[ i * 4 + ( numspheres + 1 ) * 4 + 3 ] = vrand( 0.3f, 1.0f );
    }
    screateSurface( &spheres, GL_RGBA32F_ARB, GL_RGBA, numspheres + 1, 2, 0, sphered );
  }
  rsaddShaderFromArchiveOrDie( &rs->rs, rs->varc, "vstandard", GL_VERTEX_SHADER );
  rsaddShaderFromArchiveOrDie( &rs->rs, rs->varc, "frtt", GL_FRAGMENT_SHADER );
  rsaddShaderFromArchiveOrDie( &rs->rs, rs->varc, "frttinit", GL_FRAGMENT_SHADER );
  rscreateProgramOrDie( &rs->rs, 2, prtt, "rtt" );
  rscreateProgramOrDie( &rs->rs, 2, prttinit, "rttinit" );
}
void drawRTT( rstate* rs ){
  static int inited = 0;
  
  if( !inited ){
    inited = 1;
  }
  rsrenderToSurface( &rs->rs );
  {
    static int prg = 0;
    if( !prg )
      prg = rsGetProgram( &rs->rs, "rtt" );
    glUseProgram( prg );
    glUniform1i( glGetUniformLocation( prg, "spheres" ), 0 );
    glUniform1f( glGetUniformLocation( prg, "numspheres" ), ( numspheres - (vactive ? 0.0f : 3.0f) ) );
    glUniform1f( glGetUniformLocation( prg, "shader" ), (f32)rs->shader );
    glUniform4f( glGetUniformLocation( prg, "tiles" ), (f32)TILESX, (f32)TILESY, 1.0f / ( (f32)TILESX * (f32)TILESY ), 1.0f / (f32)TILESY );
    {
      f32 h, w;
      h = ( vtan( torad * rs->fov * 0.5f ) ) / vsqrt( rs->aspect );
      w = h * rs->aspect;
      glUniform4f( glGetUniformLocation( prg, "aspect" ), w, h, 1.0f, 1.0f );
    }
    glUniform4f( glGetUniformLocation( prg, "window" ), rs->windowPos.x, vscreenHeight - rs->windowDim.y - rs->windowPos.y, rs->windowDim.x, rs->windowDim.y );
    glUniform2f( glGetUniformLocation( prg, "screen" ), (GLfloat)vscreenWidth, (GLfloat)vscreenHeight );
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();	
    sbind( &spheres, 0, 0 );
    {
      glBegin( GL_QUADS );
      glVertex2f( -1, -1 );
      glVertex2f(  1, -1 );
      glVertex2f(  1,  1 );
      glVertex2f( -1,  1 );
      glEnd();
    }
    glFlush();
    rsstopSurfaceRendering( &rs->rs );
    rsflip( &rs->rs );
  }
}
void draw( rstate* rs ){
  static int pfg = 0;
  static int uvtex = -1, utweak = -1, utiles = -1, ustex = -1;
  if( pfg == 0 ){
    pfg = getProgramByName( &rs->progs, "fg" );
    assess( pfg, "Failed to get a shader!" );
    uvtex = glGetUniformLocation( pfg, "vtex" );
    utweak = glGetUniformLocation( pfg, "tweak" );
    utiles = glGetUniformLocation( pfg, "tiles" );
    ustex = glGetUniformLocation( pfg, "stex" );
    drawRTT( rs );
    {
      u8* td = vsmalloc( DIMY * DIMX * 4 );
      u8* td2 = vsmalloc( DIMY * DIMX * 4 );
      rsbindRenderSurface( &rs->rs, 0, 0, WGL_FRONT_LEFT_ARB );
      glActiveTexture( GL_TEXTURE0 );
      glGetTexImage( GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, td );
      {
        u32 i, k;
        u32 sx = 0, sy = 0, bx = 0, by = 0;
        for( i = 0; i < DIMY * DIMX; ++i ){
          for( k = 0; k < 4; ++k )
            td2[ i * 4 + k ] = td[ ( by * DIMX * DIM3 + bx * DIM3 + sy * DIMX + sx ) * 4 + k ];
          ++sx;
          if( sx == DIM3 ){
            ++sy;
            sx = 0;
          }
          if( sy == DIM3 ){
            ++bx;
            sy = 0;
          }
          if( bx == TILESX ){
            ++by;
            bx = 0;
          }
        }
      }
      screateSurface( &threed, GL_RGBA8, GL_RGBA, DIM3, DIM3, DIM3, td2 );
      vsfree( td );
      vsfree( td2 );
    }
  }
  glUseProgram( pfg );
  glEnable( GL_TEXTURE_3D );
  {
    mat m;
    triple v;
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();	
    vglPerspective( rs->fov, rs->aspect, rs->nearclip, rs->farclip );
    midentity( &m ); 
    v.x = 1; v.y = 0; v.z = 0; mrotate( &m, &v, rs->rot.y * torad );
    v.x = 0; v.y = 1; v.z = 0; mrotate( &m, &v, rs->rot.x * torad );
    v.x = 0; v.y = 0; v.z = 1; mrotate( &m, &v, rs->rot.z * torad );
    mtranslate( &m, rs->pos.x, rs->pos.y, rs->pos.z );
    glMatrixMode( GL_MODELVIEW );
    mglload( &m );
    glUniform1i( uvtex, 0 );
    glUniform1i( ustex, 1 );
    glUniform4fv( utweak, 1, grs.tweak );
    glUniform4f( utiles, (f32)TILESX, (f32)TILESY, 1.0f / ( (f32)TILESX * (f32)TILESY ), 1.0f / (f32)TILESY );
    rsbindRenderSurface( &rs->rs, 0, 1, WGL_FRONT_LEFT_ARB );
    sbind( &threed, 0, 0 );
    //sbind( &jpg, 1, 0 );
    {
      glBegin( GL_QUADS );
      glTexCoord3f( 0, 0, 0 ); glVertex3f( -1, -1, 1 );
      glTexCoord3f( 1, 0, 0 ); glVertex3f( 1, -1, 1 );
      glTexCoord3f( 1, 1, 0 ); glVertex3f( 1, 1, 1 );
      glTexCoord3f( 0, 1, 0 ); glVertex3f( -1, 1, 1 );
      glEnd();
      glBegin( GL_QUADS );
      glTexCoord3f( 0, 1, -1 ); glVertex3f( -1, 1, -1 );
      glTexCoord3f( 1, 1, -1 ); glVertex3f( 1, 1, -1 );
      glTexCoord3f( 1, 0, -1 ); glVertex3f( 1, -1, -1 );
      glTexCoord3f( 0, 0, -1 ); glVertex3f( -1, -1, -1 );
      glEnd();

      glBegin( GL_QUADS );
      glTexCoord3f( 0, 0, -1 ); glVertex3f( -1, -1, -1 );
      glTexCoord3f( 1, 0, -1 ); glVertex3f( 1, -1, -1 );
      glTexCoord3f( 1, 0, 0 ); glVertex3f( 1, -1, 1 );
      glTexCoord3f( 0, 0, 0 ); glVertex3f( -1, -1, 1 );
      glEnd();
      glBegin( GL_QUADS );
      glTexCoord3f( 0, 1, 0 ); glVertex3f( -1, 1, 1 );
      glTexCoord3f( 1, 1, 0 ); glVertex3f( 1, 1, 1 );
      glTexCoord3f( 1, 1, -1 ); glVertex3f( 1, 1, -1 );
      glTexCoord3f( 0, 1, -1 ); glVertex3f( -1, 1, -1 );
      glEnd();

      glBegin( GL_QUADS );
      glTexCoord3f( 0, 0, -1 ); glVertex3f( -1, -1, -1 );
      glTexCoord3f( 0, 0, 0 ); glVertex3f( -1, -1, 1 );
      glTexCoord3f( 0, 1, 0 ); glVertex3f( -1, 1, 1 );
      glTexCoord3f( 0, 1, -1 ); glVertex3f( -1, 1, -1 );
      glEnd();
      glBegin( GL_QUADS );
      glTexCoord3f( 1, 1, -1 ); glVertex3f( 1, 1, -1 );
      glTexCoord3f( 1, 1, 0 ); glVertex3f( 1, 1, 1 );
      glTexCoord3f( 1, 0, 0 ); glVertex3f( 1, -1, 1 );
      glTexCoord3f( 1, 0, -1 ); glVertex3f( 1, -1, -1 );
      glEnd();

    }
    rsunbindRenderSurface( &rs->rs, WGL_FRONT_LEFT_ARB );
  }
  glFlush();
}
void drawcleanup( rstate* rs ){
}

