#include "vislib.h"

void vglGetError( void ){
  int e = glGetError();
  switch( e ){
    case GL_NO_ERROR:
      break;
    case GL_INVALID_ENUM:
      vdie( "GL_INVALID_ENUM\n" );
      break;
    case GL_INVALID_VALUE:
      vdie( "GL_INVALID_VALUE\n" );
      break;
    case GL_INVALID_OPERATION:
      vdie( "GL_INVALID_OPERATION\n" );
      break;
    case GL_STACK_OVERFLOW:
      vdie( "GL_STACK_OVERFLOW\n" );
      break;
    case GL_STACK_UNDERFLOW:
      vdie( "GL_STACK_UNDERFLOW\n" );
      break;
    case GL_OUT_OF_MEMORY:
      vdie( "GL_OUT_OF_MEMORY\n" );
      break;
    case GL_TABLE_TOO_LARGE:
      vdie( "GL_TABLE_TOO_LARGE\n" );
      break;
  }
}
void vglPerspective( f32 fov, f32 aspect, f32 nearclip, f32 farclip ){
  f32 h, w;
  h = ( vtan( torad * fov * 0.5f ) ) / vsqrt( aspect );
  w = h * aspect;
  glFrustum( -w * nearclip, w * nearclip, -h * nearclip, h * nearclip, nearclip, farclip );
}
void vglSwapControl( u32 f ){
  wglSwapInterval( f );
}
