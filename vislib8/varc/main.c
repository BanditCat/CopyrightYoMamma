#include "vislib.h"

static varchive* arc;
static u32 ofn = 0;
static int done = 0;
#define DESIRED_WIDTH 600
#define DESIRED_HEIGHT 600
#define DESIRED_X 100
#define DESIRED_Y 50
const u8* pparseLine( int lineNumber, const u8* line, u32 sz ){
  u8* fn;
  u8* an;
  u32 q1 = 0, q2, q3, q4;
  if( !sz || line[ 0 ] == '#' )
    return NULL;
  if( line[ 0 ] == '>' ){
    u32 tfn = 1;
    if( ofn )
      return "Attempt to set output filename twice.";
    if( sz == 1 )
      return "Attempt to set a blank output filename.";
    ofn = vmalloc( 0 );
    while( tfn < sz )
      vappend( ofn, line + (tfn++), 1 );
    vappendString( ofn, "" );
    return NULL;
  }else{
    while( q1 < sz && visspace( line[ q1 ] ) )
      ++q1;
    if( q1 >= sz )
      return NULL;
    if( line[ q1 ] != '"' )
      return "Failed to find any quotes.";
    q2 = q1 + 1;
    if( q2 >= sz || line[ q2 ] == '"' )
      return "Empty filename.";
    while( q2 < sz && line[ q2 ] != '"' )
      ++q2;
    if( q2 >= sz )
      return "Unterminated filename.";
    q3 = q2 + 1;
    while( q3 < sz && visspace( line[ q3 ] ) )
      ++q3;
    if( q3 >= sz )
      return "No archive name given.";
    if( line[ q3 ] != '"' )
      return "Malformed line, unable to find opening quote for archived name.";
    q4 = q3 + 1;  
    if( q4 >= sz || line[ q4 ] == '"' )
      return "Empty archived name.";
    while( q4 < sz && line[ q4 ] != '"' )
      ++q4;
    if( q4 >= sz )
      return "Unable to find closing quote for archived name.";
    ++q1; ++q3; --q2; --q4;



    fn = vsmalloc( 2 + q2 - q1 );
    vmemcpy( fn, line + q1, 1 + q2 - q1 );
    fn[ 1 + q2 - q1 ] = 0;
    an = vsmalloc( 2 + q4 - q3 );
    vmemcpy( an, line + q3, 1 + q4 - q3 );
    an[ 1 + q4 - q3 ] = 0;

    {
      u32 ldf = vloadFile( fn );
      if( !ldf ){
        vlogInfo( "Unable to open file \"" ); vlogInfo( fn ); vlogInfo( "\"\n" );
        vsfree( fn );
        vsfree( an );
        return "Failed to open file";
      }
      vlog( "Loaded \"" ); vlog( fn ); vlog( "\" as \"" ); vlog( an ); vlog( "\".\n" );
      vaddToArchive( arc, an, vmem( ldf ), vsize( ldf ) );
      verase( ldf );
    }

    vsfree( fn );
    vsfree( an );
    return NULL;
  }
}
vinitInfo* init( const u8* commandline ){
  int erf = 0;
  u32 listFile = 0;
  u32 err = vmalloc( 0 );
  static vinitInfo vi;

  vi.logX = 60;
  vi.logY = 30;
  vi.logWidth = 700;
  vi.logHeight = 500;
  vi.showLog = 1;
  vi.logName = "Varc log";

  arc = vnewArchive();

  if( commandline == NULL || *commandline == 0 )
    vappendString( err, "Unable to get command line or no arguments given, expected a list file filename." );
  if( !vsize( err ) ){
    listFile = vloadFile( commandline );
    if( !listFile ){
      vappendString( err, "Unable to load list file \"" ); vappendString( err, commandline ); vappendString( err, "\"." );
    }
  }
  if( vsize( err ) ){
    vappendString( err, "\nPress esc to exit" );
    vlog( "\n\n" );
    vlogError( vmem( err ) );
    erf = 1;
  } else{
    u8* cl = vmem( listFile );
    int i = 0;
    int l = 1;
    while( ( cl + i ) < ( (u8*)vmem( listFile ) + vsize( listFile ) ) ){
      const u8* em;
      while( cl[ i ] != '\n' && ( cl + i < (u8*)vmem( listFile ) + vsize( listFile ) ) )
        ++i;
      if( ( em = pparseLine( l, cl, i - 1 ) ) != NULL ){
        vappendString( err, "Unable to parse line " ); vappendInt( err, l, 0 ); vappendString( err, ":\n" );
        vappendString( err, em );
        break;
      }
      cl = cl + i + 1;
      i = 0;
      ++l;
    }
    if( vsize( err ) ){
      vlog( "\n\n" );
      vappendString( err, "\nPress esc to exit" );
      vlogError( vmem( err ) );
      erf = 1;
    }
  }



  if( !erf ){
    u32 msg = vmalloc( 0 );
    u32 bts = varchiveToBytes( arc );
    u32 cmp = vcompress( vmem( bts ), vsize( bts ) );

    vappendString( msg, "Compressing from " );
    vappendInt( msg, vsize( bts ), 0 ); vappendString( msg, " to " );
    vappendInt( msg, vsize( cmp ), 0 ); vappendString( msg, " bytes.\n\n" );
    vlog( "\n" );
    vlogInfo( vmem( msg ) );
    if( !ofn || !vsize( ofn ) ){
      vlog( "\n\n" );
      vlogError( "No output filename supplied.\nPress esc to exit" );
      erf = 1;
    }else{
      u32 tsto = vloadFile( vmem( ofn ) );
      vstrcmp( vmem( ofn ), "..\\ssprite.vrc" );
      if( tsto ){
        vlog( "\n\n" );
        vlogError( "Output file exists.\nPress esc to exit" );
        erf = 1;
      } else{
        vlogInfo( "Outputting to " );  
        vlogInfo( vmem( ofn ) );
        vwriteFileOrDie( vmem( ofn ), vmem( cmp ), vsize( cmp ) );
      }
    }

    {
      u32 x, y;
      u32 d = vdecompress( vmem( cmp ), vsize( cmp ) );
      varchive* na = vnewArchiveFromBytes( vmem( d ), vsize( d ) );
      if( na == NULL )
        vdie( "Failed to build test archive." );
      if( na->size != arc->size )
        vdie( "Test archive had wrong size." );
      for( x = 0; x < na->size; ++x ){
        if( na->sizes[ x ] != arc->sizes[ x ] )
          vdie( "Test archive entry had wrong size." );
        if( na->datas[ x ] == arc->datas[ x ] )
          vdie( "Test archive entry had duplicate pointers." );
        if( na->names[ x ] == arc->names[ x ] )
          vdie( "Test archive entry had duplicate pointers." );
        if( vstrcmp( na->names[ x ], arc->names[ x ] ) )
          vdie( "Test archive entry had bad name." );
        for( y = 0; y < na->sizes[ x ]; ++y )
          if( na->datas[ x ][ y ] != arc->datas[ x ][ y ] )
            vdie( "Test archive entry had bad data." );
      }
      vdeleteArchive( na );
    }

  }
  vdeleteArchive( arc );
  if( !erf )
    done = 1;
  return &vi;
}
void cleanup( void ){}
void resize( u32 x, u32 y, u32 width, u32 height ){}
int tick( f32 delta, f32 xdelta, f32 ydelta, f32 mwheelDelta ){
  if( vkeys[ VK_ESCAPE ] || done )
    return 1;
  else
    return 0;
}


