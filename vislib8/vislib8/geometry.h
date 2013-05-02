typedef struct{
  f32 x;
  f32 y;
  f32 z;
} triple;

typedef struct{
  f32 x;
  f32 y;
} pair;

typedef struct{
  f32 m[ 4 ][ 4 ];
} mat;



// Converts a triple from the HSV color space to the RGB color space.
void vhsvToRgb( triple* clr );

// Vector functions.
f32 vdot( const triple* x, const triple* y );
void vscale( triple* x, f32 s );
f32 vlength( const triple* x );
f32 vdistance( const triple* x, const triple* y );
void vadd( triple* x, const triple* y );
void vsub( triple* x, const triple* y );
void vnormalize( triple* x );
void vset( triple* x, const triple* y );
void vcross( triple* x, const triple* y );
void vmult( triple* v, const mat* m );
void vmultAll( triple* vs, const mat* m, u32 sz );
// This produces a unit vector facing a certian direction. 
void vface( triple* v, f32 xrot, f32 yrot ); 
// This sorts an array of tripples by there z value so that the furthest away is first.
// if m is NULL no transform is performed.
void vdepthSort( const mat* m, u32* inds, const triple* vs, u32 sz );
void vsphereSort( const mat* m, u32* inds, const triple* vs, const f32* radiuss, u32 sz );


// Matrix functions.
void midentity( mat* m );
void mtranslate( mat* m, f32 x, f32 y, f32 z );
void mscale( mat* ma, f32 xs, f32 ys, f32 zs );
void mrotate( mat* m, const triple* v, f32 a );
void mmult( mat* d, const mat* s );
void mglload( const mat* d );
