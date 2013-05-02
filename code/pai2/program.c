// All code not explictly labled as others work is copyright Jon DuBois 2011.  All rights reserved.

#include "vutil.h"
#include "program.h"


Program* newProgram( const vbitField* vbf ){
  u32 i;
  Program* ans = (Program*)vsmalloc( sizeof( Program ) );
  if( ( vbf->bits < 32 ) || 
      ( ( ans->bits = vgetBits( vbf, 0, 32 ) ) > 32 ) || 
      ( vbf->bits < 32 + ans->bits ) ||
      ( ( vbf->bits - ( 32 + ans->bits ) ) % ans->bits ) ){
    vsfree( ans );
    return NULL;
  }
  
  ans->size = vgetBits( vbf, 32, ans->bits );
  ans->arguments = vmalloc( ans->size * sizeof( u32 ) );
  ans->operations = vmalloc( ans->size * sizeof( u32 ) );
  ans->results = vmalloc( ans->size * sizeof( u32 ) );
for( i = 32 + ans->bits; i < vbf->bits; i += ans->bits ){
  
  }

  return ans;
}