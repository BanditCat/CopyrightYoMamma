#define vsync 0

static u32 dimx = 1, dimy = 1;
static f32 ratx, raty;

renderSurface frs;
surface pal, black;

#define palsize 3
#define numpalettes 6
static const u8 pald[] = {
  0, 255, 255,
255,   0, 255,
255, 255,   0,
  0,   0,   0,

255,   0,   0,
  0, 255,   0,    
  0,   0, 255,
  0,   0,   0,

  0,   0,   0,
128, 128, 128,    
255, 255, 255,
  0,   0,   0,

  0,   0,   0,
128,   0,   0,
255,   0,   0,
  0,   0,   0,

  0,   0,   0,
  0, 128,   0,
  0, 255,   0,
  0,   0,   0,

  0,   0,   0,
  0,   0, 128,
  0,   0, 255,
  0,   0,   0,
};

void initgl( rstate* rs ){
  glShadeModel( GL_SMOOTH );
  glColorMask( 1, 1, 1, 1 );
  glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
  glDisable( GL_DEPTH_TEST );
  glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST );
  glDisable( GL_BLEND );
  //glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
  //glDepthMask( 1 );
  glDepthMask( 0 );
  glClear( GL_COLOR_BUFFER_BIT + GL_DEPTH_BUFFER_BIT );
}
void drawinit( rstate* rs ){
  const u8* pfg[] = { "vstandard", "fbg" };
  const u8* pfg2[] = { "vstandard", "fbgb" };
  vglSwapControl( vsync );
  initgl( rs );
  initShaders( &rs->shads );
  initPrograms( &rs->progs );
  addShaderFromArchiveOrDie( &rs->shads, rs->varc, "vstandard", GL_VERTEX_SHADER );
  addShaderFromArchiveOrDie( &rs->shads, rs->varc, "fbg", GL_FRAGMENT_SHADER );
  addShaderFromArchiveOrDie( &rs->shads, rs->varc, "fbgb", GL_FRAGMENT_SHADER );
  createProgramOrDie( &rs->progs, &rs->shads, 2, pfg, "foreground" );
  createProgramOrDie( &rs->progs, &rs->shads, 2, pfg2, "foregroundBorder" );

  screateSurface( &pal, GL_RGB8, GL_RGB, palsize, numpalettes, 0, pald );
  screateSurface( &black, GL_RGB8, GL_RGB, 1, 1, 0, pald + 8 );
}
void drawRTT( rstate* rs, int reinit ){
  static const pixelFormat* complex32PixelFormat = NULL;
  const u8* pfracn[] = { "vfrac", "ffrac" };
  const u8* pfracinitn[] = { "vfrac", "ffracinit" };
  static int inited = 0;

  if( !inited ){
    inited = 1;
    // BUGBUG make 2 compenent
    complex32PixelFormat = vglFindPixelFormat( 32, -1, 32, -1, 32, -1, 0, -1, 0, -1, 0, -1, 0, -1, 0, -1, 0, 0, 0, 0 );
    if( complex32PixelFormat == NULL )
      vdie( "Unable to find pixel formats!" );
    while( dimx < vscreenWidth )
      dimx *= 2;
    while( dimy < vscreenHeight )
      dimy *= 2;
    ratx = (f32)vscreenWidth / dimx;
    raty = (f32)vscreenHeight / dimy;
    rscreateRenderSurface( &frs, complex32PixelFormat, dimx, dimy, 1, 1 );
    
    rsaddShaderFromArchiveOrDie( &frs, rs->varc, "vfrac", GL_VERTEX_SHADER );
    rsaddShaderFromArchiveOrDie( &frs, rs->varc, "ffrac", GL_FRAGMENT_SHADER );
    rsaddShaderFromArchiveOrDie( &frs, rs->varc, "ffracinit", GL_FRAGMENT_SHADER );
    rscreateProgramOrDie( &frs, 2, pfracn, "frac" );
    rscreateProgramOrDie( &frs, 2, pfracinitn, "fracinit" );

    rsrenderToSurface( &frs ); initgl( rs ); glViewport( 0, 0, vscreenWidth, vscreenHeight ); rsstopSurfaceRendering( &frs );
    rsflip( &frs );
    rsrenderToSurface( &frs ); initgl( rs ); glViewport( 0, 0, vscreenWidth, vscreenHeight ); rsstopSurfaceRendering( &frs );
  }

  if( reinit ){
    f64 lminx, lminy, lmaxx, lmaxy, cx, cy, m;
    m = rs->m / 2.0f;
    cx = rs->xc + 0.5f;
    cy = rs->yc + 0.5f;
    lminx = ( rs->maxx - rs->minx ) * ( cx - m ) + rs->minx;
    lminy = ( rs->maxy - rs->miny ) * ( cy - m ) + rs->miny;
    lmaxx = ( rs->maxx - rs->minx ) * ( cx + m ) + rs->minx;
    lmaxy = ( rs->maxy - rs->miny ) * ( cy + m ) + rs->miny;
    rs->minx = lminx; rs->maxx = lmaxx; rs->miny = lminy; rs->maxy = lmaxy;
    rs->xc = rs->yc = 0; rs->m = 1;

    rsrenderToSurface( &frs );
    glUseProgram( rsGetProgram( &frs, "fracinit" ) );
    glBegin( GL_QUADS );
    glTexCoord2f( (f32)rs->minx, (f32)rs->miny ); glVertex2f( -1, -1 );
    glTexCoord2f( (f32)rs->maxx, (f32)rs->miny ); glVertex2f(  1, -1 );
    glTexCoord2f( (f32)rs->maxx, (f32)rs->maxy ); glVertex2f(  1,  1 );
    glTexCoord2f( (f32)rs->minx, (f32)rs->maxy ); glVertex2f( -1,  1 );
    glEnd();
    glFlush();
    rsstopSurfaceRendering( &frs );
  }else{
    GLint prg;
    rsflip( &frs );
    rsrenderToSurface( &frs );
    prg = rsGetProgram( &frs, "frac" );
    glUseProgram( prg );
    rsbindRenderSurface( &frs, 0, 0, WGL_FRONT_LEFT_ARB );
    glUniform1f( glGetUniformLocation( prg, "bailout" ), rs->bailout );
    glUniform2f( glGetUniformLocation( prg, "aspect" ), ratx, raty );
    glUniform2f( glGetUniformLocation( prg, "peturb" ), rs->px, rs->py );


    glBegin( GL_QUADS );
    if( rs->fmode ){
      glTexCoord2f( rs->jx, rs->jy ); glVertex2f( -1, -1 );
      glTexCoord2f( rs->jx, rs->jy ); glVertex2f(  1, -1 );
      glTexCoord2f( rs->jx, rs->jy ); glVertex2f(  1,  1 );
      glTexCoord2f( rs->jx, rs->jy ); glVertex2f( -1,  1 );
    }else{
      glTexCoord2f( (f32)rs->minx, (f32)rs->miny ); glVertex2f( -1, -1 );
      glTexCoord2f( (f32)rs->maxx, (f32)rs->miny ); glVertex2f(  1, -1 );
      glTexCoord2f( (f32)rs->maxx, (f32)rs->maxy ); glVertex2f(  1,  1 );
      glTexCoord2f( (f32)rs->minx, (f32)rs->maxy ); glVertex2f( -1,  1 );
    }
    glEnd();

    rsunbindRenderSurface( &frs, WGL_FRONT_LEFT_ARB );
    glFlush();
    rsstopSurfaceRendering( &frs );
  }
}
void draw( rstate* rs ){
  static int inited = 0;
  static int pfg = 0;

  if( !inited ){
    inited = 1;
    drawRTT( rs, 1 );
  }
  if( rs->shader % 2 )
    pfg = getProgramByName( &rs->progs, "foregroundBorder" );
  else
    pfg = getProgramByName( &rs->progs, "foreground" );
  glMatrixMode( GL_PROJECTION );
  glLoadIdentity();	
  glMatrixMode( GL_MODELVIEW );  
  glLoadIdentity();	

  drawRTT( rs, 0 ); 
  drawRTT( rs, 0 );
  glUseProgram( pfg );
  sbind( &pal, 1, 1 );
  if( rs->shader > 1 )
    sbind( &vscreenTex, 2, 0 );
  else
    sbind( &black, 2, 0 );
  rsbindRenderSurface( &frs, 0, 1, WGL_FRONT_LEFT_ARB );
  glUniform1i( glGetUniformLocation( pfg, "tex" ), 0 );
  glUniform1i( glGetUniformLocation( pfg, "pal" ), 1 );
  glUniform1i( glGetUniformLocation( pfg, "bg" ), 2 );
  glUniform1f( glGetUniformLocation( pfg, "palpos" ), rs->prpos );
  glUniform1f( glGetUniformLocation( pfg, "palscale" ), rs->pscale );
  glUniform1f( glGetUniformLocation( pfg, "palsel" ), (f32)rs->psel / numpalettes + 1.0f / ( numpalettes * 2 ) );
  glUniform2f( glGetUniformLocation( pfg, "aspect" ), ratx, raty );
  {
    f32 lminx, lminy, lmaxx, lmaxy, cx, cy, m;
    m = rs->m / 2.0f;
    cx = rs->xc + 0.5f;
    cy = rs->yc + 0.5f;
    lminx = cx - m;
    lminy = cy + m;
    lmaxx = cx + m;
    lmaxy = cy - m;

    glBegin( GL_QUADS );
    glTexCoord2f( lminx, lminy ); glVertex2f( -1, -1 );
    glTexCoord2f( lmaxx, lminy ); glVertex2f(  1, -1 );
    glTexCoord2f( lmaxx, lmaxy ); glVertex2f(  1,  1 );
    glTexCoord2f( lminx, lmaxy ); glVertex2f( -1,  1 );
    glEnd();
  }

  rsunbindRenderSurface( &frs, WGL_FRONT_LEFT_ARB );

  glFlush();
}
void drawcleanup( rstate* rs ){
  rsdestroyRenderSurface( &frs );
  destroyShaders( &rs->shads );
  destroyPrograms( &rs->progs );
}