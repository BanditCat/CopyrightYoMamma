// All code not explictly labled as others work is copyright Jon DuBois 2011.  All rights reserved.

#include "stdio.h"
#include "vutil.h"





int main( int argc, char* argv[] ){

  u8* noise = vgenPad( 0, 1, 40 );
  int i;

  for( i = 0; i < 40; ++i ){
    printf( "%d, %d\n", i, ( (int)noise[ i ] ) );
  }

  vsfree( noise );
  (void)argc;(void)argv;

  {
#define numprms 100000
    u32 ct = 2;
    u32 primes[ numprms ];
    u32 p = 0;
    u32 k, isp;
    for( ct = 2; ct < numprms; ++ct ){
      isp = 1;
      for( k = 0; k < p; ++k )
        if( ct % primes[ k ] == 0 ){
          isp = 0;
          break;
        }
      if( isp )
        primes[ p++ ] = ct;
    }
    for( k = 0; k < p; ++k )
      printf( "%d,", primes[ k ] );
  }

  getchar();
	return 0;
}

