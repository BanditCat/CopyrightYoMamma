#include "vislib.h"

void vhsvToRgb( triple* clr ){
  f32 flr = ( (f32)( (u32)( clr->x / 60.0 ) ) );
  int sel = ( (int)flr ) % 6;
  f32 vs = ( clr->x / 60 ) - flr;
  f32 v = clr->z;
  f32 p = v * ( 1 - clr->y );
  f32 q = v * ( 1 - clr->y * vs );
  f32 t = v * ( 1 - clr->y * ( 1 - vs ) );

  switch( sel ){
    default:
    case 0: clr->x = v; clr->y = t; clr->z = p; break;
    case 1: clr->x = q; clr->y = v; clr->z = p; break;
    case 2: clr->x = p; clr->y = v; clr->z = t; break;
    case 3: clr->x = p; clr->y = q; clr->z = v; break;
    case 4: clr->x = t; clr->y = p; clr->z = v; break;
    case 5: clr->x = v; clr->y = p; clr->z = q; break;
  }
}

f32 vdot( const triple* x, const triple* y ){
  return( x->x * y->x + x->y * y->y + x->z * y->z );
}
void vscale( triple* x, f32 s ){
  x->x *= s;x->y *= s;x->z *= s;
}
f32 vlength( const triple* x ){ return (f32)vsqrt( x->x * x->x + x->y * x->y + x->z * x->z ); }
void vadd( triple* x, const triple* y ){
  x->x += y->x; x->y += y->y; x->z += y->z;
}
void vsub( triple* x, const triple* y ){
  x->x -= y->x; x->y -= y->y; x->z -= y->z;
}
f32 vdistance( const triple* x, const triple* y ){
  triple v = *y;
  vsub( &v, x );
  return vlength( &v );
}
void vnormalize( triple* x ){
  f32 len = vlength( x );
  if( len == 0 ){
    x->x = 0; x->y = 0; x->z = 1;
  }else{
    len = (f32)1.0 / len;
    vscale( x, len );
  }
}
void vset( triple* x, const triple* y ){ x->x = y->x; x->y = y->y; x->z = y->z; }
void vcross( triple* x, const triple* y ){
  triple t;
  vset( &t, x );
  x->x = ( t.y * y->z - t.z * y->y );
  x->y = ( t.z * y->x - t.x * y->z );
  x->z = ( t.x * y->y - t.y * y->x );
}
void vmult( triple* v, const mat* m ){
  f32 xp = m->m[ 0 ][ 0 ] * v->x + m->m[ 1 ][ 0 ] * v->y + m->m[ 2 ][ 0 ] * v->z + m->m[ 3 ][ 0 ];
  f32 yp = m->m[ 0 ][ 1 ] * v->x + m->m[ 1 ][ 1 ] * v->y + m->m[ 2 ][ 1 ] * v->z + m->m[ 3 ][ 1 ];
  v->z = m->m[ 0 ][ 2 ] * v->x + m->m[ 1 ][ 2 ] * v->y + m->m[ 2 ][ 2 ] * v->z + m->m[ 3 ][ 2 ];
  v->x = xp;
  v->y = yp;
}
void vmultAll( triple* vs, const mat* m, u32 sz ){
  u32 i;
  for( i = 0; i < sz; ++i )
    vmult( vs + i, m );
}
void vface( triple* v, f32 xrot, f32 yrot ){
  static const triple xr = { 0.0f, 1.0f, 0.0f };
  static const triple yr = { 1.0f, 0.0f, 0.0f };
  static mat m; 
  midentity( &m );
  mrotate( &m, &xr, xrot * torad );
  mrotate( &m, &yr, yrot * torad );
  v->x = v->y = 0.0f; v->z = 1.0f;
  vmult( v, &m );
}
static f32* vdshma = NULL;
int vdepthSortHelper( const void* v1, const void* v2 ){
  u32 i1 = *( (u32*)v1 );
  u32 i2 = *( (u32*)v2 );
  if( vdshma[ i1 ] > vdshma[ i2 ] )
    return 1;
  if( vdshma[ i1 ] < vdshma[ i2 ] )
    return -1;
  return 0;
}
void vdepthSort( const mat* m, u32* inds, const triple* vs, u32 sz ){
  static u32 vdshmap = 0;
  static u32 vdshmasz = 0;
  u32 i;
  for( i = 0; i < sz; ++i )
    inds[ i ] = i;
  if( vdshma == NULL ){
    vdshmasz = sz;
    vdshmap = vmalloc( sz * sizeof( f32 ) );
    vdshma = vmem( vdshmap );
  }else if( vdshmasz < sz ){
    vgrow( vdshmap, ( sz - vdshmasz ) );
    vdshma = vmem( vdshmap );
  }
  for( i = 0; i < sz; ++i )
    vdshma[ i ] = m->m[ 0 ][ 2 ] * vs[ i ].x + m->m[ 1 ][ 2 ] * vs[ i ].y + m->m[ 2 ][ 2 ] * vs[ i ].z + m->m[ 3 ][ 2 ];
  vqsort( (u8*)inds, sz, sizeof( u32 ), vdepthSortHelper );
}
static f32* vlshma = NULL;
int vsphereSortHelper( const void* v1, const void* v2 ){
  u32 i1 = *( (u32*)v1 );
  u32 i2 = *( (u32*)v2 );
  if( vlshma[ i1 ] > vlshma[ i2 ] )
    return -1;
  if( vlshma[ i1 ] < vlshma[ i2 ] )
    return 1;
  return 0;
}
void vsphereSort( const mat* m, u32* inds, const triple* vs, const f32* radiuss, u32 sz ){
  static u32 vlshmap = 0;
  static u32 vlshmasz = 0;
  u32 i;
  for( i = 0; i < sz; ++i )
    inds[ i ] = i;
  if( vlshma == NULL ){
    vlshmasz = sz;
    vlshmap = vmalloc( sz * sizeof( f32 ) );
    vlshma = vmem( vlshmap );
  }else if( vlshmasz < sz ){
    vgrow( vlshmap, ( sz - vlshmasz ) );
    vlshma = vmem( vlshmap );
  }
  for( i = 0; i < sz; ++i ){
    f32 d2, n, r2, d;
    triple v = vs[ i ];
    vmult( &v, m );
    r2 = radiuss[ i ] * radiuss[ i ];
    d2 = vdot( &v, &v );
    d = vsqrt( d2 );
    if( d2 > r2 )
      n = ( r2 * 0.5f ) / vsqrt( d2 - r2 );
    else
      n = ( r2 * 0.5f ) / vsqrt( r2 - d2 );
    vlshma[ i ] = d - n;
  }
  vqsort( (u8*)inds, sz, sizeof( u32 ), vsphereSortHelper );
}
void midentity( mat* m ){
  u32 i, j;
  for( i = 0; i < 4; ++i )
    for( j = 0; j < 4; ++j )
      if( i == j ) 
        m->m[ i ][ j ] = 1;
      else
        m->m[ i ][ j ] = 0;
}
void mrotate( mat* ma, const triple* v, f32 a ){
  f32 c = vcos( a );
  f32 s = vsin( a );
  f32 t = 1 - c;
  static mat m;

  m.m[ 0 ][ 0 ] = t * v->x * v->x + c;
  m.m[ 0 ][ 1 ] = t * v->x * v->y + s * v->z;
  m.m[ 0 ][ 2 ] = t * v->x * v->z - s * v->y;
  m.m[ 0 ][ 3 ] = 0;

  m.m[ 1 ][ 0 ] = t * v->x * v->y - s * v->z;
  m.m[ 1 ][ 1 ] = t * v->y * v->y + c;
  m.m[ 1 ][ 2 ] = t * v->y * v->z + s * v->x;
  m.m[ 1 ][ 3 ] = 0;

  m.m[ 2 ][ 0 ] = t * v->x * v->z + s * v->y;
  m.m[ 2 ][ 1 ] = t * v->y * v->z - s * v->x;
  m.m[ 2 ][ 2 ] = t * v->z * v->z + c;
  m.m[ 2 ][ 3 ] = 0;

  m.m[ 3 ][ 0 ] = 0;
  m.m[ 3 ][ 1 ] = 0;
  m.m[ 3 ][ 2 ] = 0;
  m.m[ 3 ][ 3 ] = 1;
  
  mmult( ma, &m );
}
void mtranslate( mat* ma, f32 x, f32 y, f32 z ){
  static mat m;
  u32 i, j;
  for( i = 0; i < 4; ++i )
    for( j = 0; j < 4; ++j )
      if( i == j )
        m.m[ i ][ j ] = 1;
      else
        m.m[ i ][ j ] = 0;
  m.m[ 3 ][ 0 ] = x; m.m[ 3 ][ 1 ] = y; m.m[ 3 ][ 2 ] = z;
  mmult( ma, &m );
}
void mscale( mat* ma, f32 xs, f32 ys, f32 zs ){
  static mat m;
  u32 i, j;
  for( i = 0; i < 4; ++i )
    for( j = 0; j < 4; ++j )
      if( i == j )
        if( i == 3 )
          m.m[ i ][ j ] = 1;
        else if( i == 0 )
          m.m[ i ][ j ] = xs;
        else if( i == 1 )
          m.m[ i ][ j ] = ys;
        else
          m.m[ i ][ j ] = zs;
      else
        m.m[ i ][ j ] = 0;
  mmult( ma, &m );
}
void mmult( mat*d, const mat* s ){
  static mat tg;
  u32 i, j, k;
  for( i = 0; i < 4; ++i ){
    for( j = 0; j < 4; ++j ){
      tg.m[ i ][ j ] = 0;
      for( k = 0; k < 4; ++k )
        tg.m[ i ][ j ] += d->m[ i ][ k ] * s->m[ k ][ j ]; 
    }
  }
  for( i = 0; i < 4; ++i )
    for( j = 0; j < 4; ++j )
      d->m[ i ][ j ] = tg.m[ i ][ j ];
}
void mglload( const mat* m ){
  u32 i, j;
  static f32 gm[ 16 ];
  for( i = 0; i < 4; ++i )
    for( j = 0; j < 4; ++j )
      gm[ i + j * 4 ] = m->m[ j ][ i ];
  glLoadMatrixf( gm );
}








