
typedef struct{
  GLuint name;
  GLenum type;
  int dim;
  u32 width, height, depth;
} surface;


void sbind( const surface* s, GLuint textureUnit, int nice );
// If depth is non-zero it is a 3d texture, otherwise 2d.
void screateSurface( surface* s, GLint internalType, GLenum type, u32 width, u32 height, u32 depth, const void* data );
// returna a void* containing texture data that must be vsfreed'
void* sgetData( surface* s );

typedef struct{
  pixelFormat pf;
  u32 width, height;

//private
  GLuint names[ 2 ][ 32 ];
  HPBUFFERARB pbs[ 2 ];
  HDC dcs[ 2 ];
  HGLRC rcs[ 2 ];
  int mipmap;
  shaders shads[ 2 ];
  programs progs[ 2 ];
  int front;
  int pingpong;
} renderSurface;


void rscreateRenderSurface( renderSurface* s, const pixelFormat* pf, u32 width, u32 height, int mipmap, int pingpong );
void rsdestroyRenderSurface( renderSurface* s );
void rsrenderToSurface( renderSurface* s );
void rsstopSurfaceRendering( renderSurface* s );
void rsflip( renderSurface* s );
void rsaddShaderFromResourceOrDie( renderSurface* s, u32 rsc, const u8* name, int type );
void rsaddShaderFromArchiveOrDie( renderSurface* s, varchive* varc, const u8* name, int type );
void rscreateProgramOrDie( renderSurface* s, u32 cnt, const u8** shds, const u8* name );
GLint rsGetProgram( renderSurface* s, const u8* name );
// These will automatically use the back buffer on a pingpong renderSurface.
void rsbindRenderSurface( renderSurface* s, GLuint textureUnit, int nice, int buffer );
void rsunbindRenderSurface( const renderSurface* s, int buffer );


