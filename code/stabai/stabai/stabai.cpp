#include "stabai.h"

u32 levenshteinDistance( const std::deque< u32 >& d1, const std::deque< u32 >& d2 ){
  if( !d1.size() )
    return d2.size();
  if( !d2.size() )
    return d1.size();
  
  u32** tmat = new u32*[ d1.size() + 1 ];
  for( u32 i = 0; i <= d1.size(); ++i )
    tmat[ i ] = new u32[ d2.size() + 1 ];

  for( u32 i = 0; i <= d1.size(); ++i )
    for( u32 j = 0; j <= d2.size(); ++j ){
      if( j == 0 )
        tmat[ i ][ j ] = i;
      else if( i == 0 )
        tmat[ i ][ j ] = j;
      else
        tmat[ i ][ j ] = 0;
    }

  for( u32 i = 1; i <= d1.size(); ++i ){
    u32 src = d1[ i - 1 ];
    for( u32 j = 1; j <= d2.size(); ++j ){
      u32 trg = d2[ j - 1 ];

      u32 cst;
      if( src == trg )
        cst = 0;
      else
        cst = 1;

      u32 abv = tmat[ i - 1 ][ j ];
      u32 lft = tmat[ i ][ j - 1 ];
      u32 diag = tmat[ i - 1 ][ j  - 1 ];
      u32 cell = diag + cst;
      if( lft + 1 < cell )
        cell = lft + 1;
      if( abv + 1 < cell )
        cell = abv + 1;

      if( i > 2 && j > 2 ){
        u32 trns = tmat[ i - 2 ][ j - 1 ] + 1;
        if( d1[ i - 2 ] != trg )
          ++trns;
        if( d2[ j - 2 ] != src )
          ++trns;
        if( cell > trns )
          cell = trns;
      }

      tmat[ i ][ j ] = cell;
    }
  }
  u32 ans = tmat[ d1.size() ][ d2.size() ];

  for( u32 i = 0; i <= d1.size(); ++i )
    delete[] tmat[ i ];
  delete[] tmat;


  return ans;
}