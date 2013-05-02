#include "vislib.h"


void initShaders( shaders* s ){
  s->bufsize = 1;
  s->shaders = vsmalloc( s->bufsize * sizeof( GLuint ) );
  s->usages = vsmalloc( s->bufsize * sizeof( u32 ) );
  s->names = vsmalloc( s->bufsize * sizeof( u32 ) );
}
void destroyShaders( shaders* s ){
  u32 i;
  for( i = 0; i < s->size; ++i )
    glDeleteShader( s->shaders[ i ] );
  vsfree( s->shaders );
  vsfree( s->usages );
  vsfree( s->names );

}
void pexpandShaders( shaders* s ){
  if( s->size + 1 >= s->bufsize ){
    u32 i;
    GLuint* ns = vsmalloc( s->bufsize * 2 * sizeof( GLuint ) );
    u32* nu = vsmalloc( s->bufsize * 2 * sizeof( u32 ) );
    u32* nn = vsmalloc( s->bufsize * 2 * sizeof( u32 ) );
    for( i = 0; i < s->size; ++i ){
      ns[ i ] = s->shaders[ i ];
      nn[ i ] = s->names[ i ];
      nu[ i ] = s->usages[ i ];
    }
    vsfree( s->shaders ); 
    vsfree( s->usages );
    vsfree( s->names );
    s->names = nn;
    s->shaders = ns;
    s->usages = nu;
    s->bufsize *= 2;
  }
}
   
void addShaderOrDie( shaders* s, const u8* src, u32 srcsz, const u8* name, int type ){
  GLuint ns;
#ifdef DBG
  vglGetError();
#endif
  ns = glCreateShader( type );
  assess( ns, "Unable to create shader object" ); 

  glShaderSource( ns, 1, &src, (const GLint*)&srcsz );

  glCompileShader( ns );
  {
    GLint stat;
    glGetShaderiv( ns, GL_COMPILE_STATUS, &stat );
    if( stat == GL_FALSE ){
      GLint ilsz;
      u8* il;
      u32 msg = vmalloc( 1 );
      vappendString( msg, "Error compiling shader " );
      vappendString( msg, name );
      vappendString( msg, ".\n" );
      glGetShaderiv( ns, GL_INFO_LOG_LENGTH, &ilsz );
      il = vsmalloc( ilsz + 10 );
      glGetShaderInfoLog( ns, ilsz + 10, NULL, il );
      vappendString( msg, il );
      vsfree( il );
      glDeleteShader( ns );
      vdie( vmem( msg ) );
    }
  }
  pexpandShaders( s );
  s->names[ s->size ] = vmalloc( 1 );
  vappendString( s->names[ s->size ], name );
  s->shaders[ s->size ] = ns;
  s->usages[ s->size ] = vmalloc( 1 );
  {
    GLenum stats[] = { 
      GL_SHADER_SOURCE_LENGTH,
    };
    u8* names[] = { 
//    "shader type:     "     
      "Source length:   ",
    };
    GLenum vstats[] = { 
      GL_VERTEX_PROGRAM_TWO_SIDE,
      GL_VERTEX_PROGRAM_POINT_SIZE,
    };
    u8* vnames[] = { 
//    "shader type:     "     
      "Two sided:       ",
      "Point size mode: ",
    };
    GLint i, t;
    u32 j;
    vappendString( s->usages[ s->size ], "shader type:     " );
    glGetShaderiv( s->shaders[ s->size ], GL_SHADER_TYPE, &t );
    if( t == GL_VERTEX_SHADER )
      vappendString( s->usages[ s->size ], "Vertex" ); 
    else if( t == GL_FRAGMENT_SHADER )
      vappendString( s->usages[ s->size ], "Fragment" ); 
    else
      vappendString( s->usages[ s->size ], "Unknown" ); 
    for( j = 0; j < sizeof( stats ) / sizeof( stats[ 0 ] ); ++j ){
      vappendString( s->usages[ s->size ], "\n" );
      vappendString( s->usages[ s->size ], names[ j ] );
      glGetShaderiv( s->shaders[ s->size ], stats[ j ], &i );
      vappendInt( s->usages[ s->size ], i, 0 );
    }
    if( t == GL_VERTEX_SHADER )
      for( j = 0; j < sizeof( vstats ) / sizeof( vstats[ 0 ] ); ++j ){
        vappendString( s->usages[ s->size ], "\n" );
        vappendString( s->usages[ s->size ], vnames[ j ] );
        vappendInt( s->usages[ s->size ], glIsEnabled( vstats[ j ] ), 0 );
      }
  }
  ++s->size;
}

void addShaderFromResourceOrDie( shaders* s, u32 rsc, const u8* name, int type ){
  GLint rsz;
  const u8* ans;
  ans = vgetResourceOrDie( rsc, (u32*)&rsz, "SHADER" );
  addShaderOrDie( s, ans, rsz, name, type );
}
void addShaderFromArchiveOrDie( shaders* s, varchive* varc, const u8* name, int type ){
  u32 ind = vgetArchiveIndexByName( varc, name );
  if( ind == varc->size )
    vdie( "Unable to get archive entry in addShaderFromArchiveOrDie" );
  addShaderOrDie( s, varc->datas[ ind ], varc->sizes[ ind ], name, type );
}

GLuint getShaderByName( shaders* s, const u8* name ){
  u32 i = 0;
  for( i = 0; i < s->size; ++i ){
    if( !vstrcmp( name, vmem( s->names[ i ] ) ) )
      return s->shaders[ i ];
  }
  return 0;
}




void initPrograms( programs* p ){
  p->bufsize = 1;
  p->programs = vsmalloc( p->bufsize * sizeof( GLuint ) );
  p->usages = vsmalloc( p->bufsize * sizeof( u32 ) );
  p->names = vsmalloc( p->bufsize * sizeof( u32 ) );
}
void destroyPrograms( programs* p ){
  u32 i;
  for( i = 0; i < p->size; ++i )
    glDeleteProgram( p->programs[ i ] );
  vsfree( p->programs );
  vsfree( p->usages );
  vsfree( p->names );

}
void pexpandPrograms( programs* p ){
  if( p->size + 1 >= p->bufsize ){
    u32 i;
    GLuint* ns = vsmalloc( p->bufsize * 2 * sizeof( GLuint ) );
    u32* nu = vsmalloc( p->bufsize * 2 * sizeof( u32 ) );
    u32* nn = vsmalloc( p->bufsize * 2 * sizeof( u32 ) );
    for( i = 0; i < p->size; ++i ){
      ns[ i ] = p->programs[ i ];
      nn[ i ] = p->names[ i ];
      nu[ i ] = p->usages[ i ];
    }
    vsfree( p->programs ); 
    vsfree( p->usages );
    vsfree( p->names );
    p->names = nn;
    p->programs = ns;
    p->usages = nu;
    p->bufsize *= 2;
  }
}
   
void createProgramOrDie( programs* p, shaders* s, u32 cnt, const u8** shds, const u8* name ){
  u32 k, j;
  GLint i;
  GLuint np = glCreateProgram();
#ifdef DBG
  vglGetError();
#endif
  assess( np, "Unable to create program object" ); 
  for( k = 0; k < cnt; ++k ){
    GLuint cs;
#ifdef DBG
    if( shds[ k ] == NULL )
      vdie( "NULL pointer in createProgramOrDie." );
#endif
    cs = getShaderByName( s, shds[ k ] );
    if( !cs )
      vdie( "Failed to find a shader in createProgramOrDie." );
#ifdef DBG
  vglGetError();
#endif
    glAttachShader( np, cs );
#ifdef DBG
  vglGetError();
#endif
  }
  glLinkProgram( np );
#ifdef DBG
  vglGetError();
#endif
  {
    GLint stat;
    glGetProgramiv( np, GL_LINK_STATUS, &stat );
#ifdef DBG
  vglGetError();
#endif
    if( stat == GL_FALSE ){
      GLint ilsz;
      u8* il;
      u32 msg = vmalloc( 1 );
      vappendString( msg, "Error linking program " );
      vappendString( msg, name );
      vappendString( msg, ".\n" );
      glGetProgramiv( np, GL_INFO_LOG_LENGTH, &ilsz );
#ifdef DBG
  vglGetError();
#endif
      il = vsmalloc( ilsz + 10 );
      glGetProgramInfoLog( np, ilsz + 10, NULL, il );
      vappendString( msg, il );
      vsfree( il );
      glDeleteProgram( np );
#ifdef DBG
  vglGetError();
#endif
      vdie( vmem( msg ) );
    }
  }
  pexpandPrograms( p );
  p->names[ p->size ] = vmalloc( 1 );
  vappendString( p->names[ p->size ], name );
  p->programs[ p->size ] = np;
  p->usages[ p->size ] = vmalloc( 1 );
  {
    GLenum stats[] = { 
      GL_ACTIVE_UNIFORMS,
      GL_ACTIVE_ATTRIBUTES
    };
    u8* names[] = { 
      "Active uniforms:   ",
      "Atcive attributes: ",
    };
    for( j = 0; j < sizeof( stats ) / sizeof( stats[ 0 ] ); ++j ){
      vappendString( p->usages[ p->size ], names[ j ] );
      glGetProgramiv( p->programs[ p->size ], stats[ j ], &i );
      vappendInt( p->usages[ p->size ], i, 0 );
      vappendString( p->usages[ p->size ], "\n" );
    }
  }
  ++p->size;
}
GLuint getProgramByName( programs* p, const u8* name ){
  u32 i = 0;
  for( i = 0; i < p->size; ++i )
    if( !vstrcmp( name, vmem( p->names[ i ] ) ) )
      return p->programs[ i ];
  return 0;
}
void useProgramByName( programs* p, const u8* name ){
  GLuint up = getProgramByName( p, name );
  glUseProgram( up );
}
void useProgramByIndex( programs* p, u32 ind ){
  glUseProgram( p->programs[ ind ] );
}