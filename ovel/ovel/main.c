#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "vislib3.h"
#include "rc.h"

void usage( void ){
  u32 msg = vlAutoMalloc( 0 );
  vlAppendString( msg, "\nUsage: " );
  {
    u32 sn = vlSelfName();
    vlPopNul( msg );
    vlAppend( msg, vlMem( sn ), vlSize( sn ) - 1 );
    vlAutoFree( sn );
  }
  vlAppendString( msg, " [ -e string | -f file ] [arg1] [arg2] [argN]...\n" );
  vlAppendString( msg, "\n    If -e is supplied as the first argument, then the second argument will be" );
  vlAppendString( msg, "\n    loaded and executed as code. If -f is supplied as the first argument, then" );
  vlAppendString( msg, "\n    code will be loaded from the file named by the second argument and then" );
  vlAppendString( msg, "\n    executed. In either case, no IDE is displayed." );
  vlPopNul( msg );
  vlPrint( vlMem( msg ), vlSize( msg ), 0 );
  vlAutoFree( msg );
}

int vlMain( int argc, const char** argv ){
  // Set dispatch thread id
  u32 tid = vlGetThreadId();
  {
    vlGetGlobal()->dispatchThread = tid;
    vlReleaseGlobal();
  }

  // Uncompress the preprocessed code.
  {
    u8* pp;
    u32 psz;
    u32 unc;
    if( !vlLoadResource( 690, &pp, &psz, "TXT" ) )
      vlDie( "Failed to load preprocessed lua code resource." );
    unc = vlUncompress( pp, psz );
    if( !unc )
      vlDie( "Failed to uncompress preprocessed lua code resource." );
    vlAppendString( unc, "" );
    vlRunString( vlMem( unc ), "<preprocessed>" );
    vlAutoFree( unc );
  }

  // Parse command line.
  if( !argc )
    vlDie( "Failed to get command line!" );

  // Load command line arguments into process global memory.
  {
    u32 i;
    u32 tm = vlAutoMalloc( 0 );
    vlAppendString( tm, "vl.setGlobal( 'commandLineArgumentCount', " );
    vlAppendInt( tm, argc );
    vlAppendString( tm, " )" );
    vlRunString( vlMem( tm ), "<commandLine>" );
    vlAutoFree( tm );
    for( i = 0; i < (u32)argc; ++i ){
      u32 j;
      u32 ne = vlGetNumQuoteEquals( argv[ i ] );
      tm = vlAutoMalloc( 0 );
      vlAppendString( tm, "vl.setGlobal( 'commandLineArgument" );
      vlAppendInt( tm, i );
      vlAppendString( tm, "', [" );
      for( j = 0; j < ne; ++j ) vlAppendString( tm, "=" );
      vlAppendString( tm, "[\n" );
      vlAppendString( tm, argv[ i ] );
      vlAppendString( tm, "]" );
      for( j = 0; j < ne; ++j ) vlAppendString( tm, "=" );
      vlAppendString( tm, "] )" );
      vlRunString( vlMem( tm ), "<commandLine>" );
      vlAutoFree( tm );
    }
  }
  if( argc >= 3 && ( !strcmp( argv[ 1 ], "-e" ) || !strcmp( argv[ 1 ], "/e" ) ) ){
    vlNewThread( argv[ 2 ], NULL, NULL, "<command line>" );
  }else if( argc >= 3 && ( !strcmp( argv[ 1 ], "-f" ) || !strcmp( argv[ 1 ], "/f" ) ) ){
    u32 fl = vlLoadFile( argv[ 2 ] );
    if( !fl ){
      const u8* msg = "Unable to load file: ";
      vlPrint( msg, strlen( msg ), 1 );
      vlPrint( argv[ 2 ], strlen( argv[ 2 ] ), 1 );
    } else{
      vlAppend( fl, "", 1 );
      vlNewThread( vlMem( fl ), NULL, NULL, "<command line file>" );
    }
  }else if( argc >= 2 && strlen( argv[ 1 ] ) && ( ( argv[ 1 ][ 0 ] == '-' ) || ( argv[ 1 ][ 0 ] == '/' ) ) ){
    usage();
    return 0;
  }else {
    vlNewThread( "ovel.ide()", NULL, NULL, "<ide>" );
    vlSleep( 100 );
  }

  // Act as dispach thread until all other threads are done.
  while( vlNumThreads() != 1 ){
    u32 ans = vlItcRetrieve( tid );
    if( ans ){
      const u8* pid = vlMem( ans ); 
      const u8* inh = pid + strlen( pid ) + 1; 
      const u8* exe = inh + strlen( inh ) + 1; 
      vlNewThread( exe, pid, inh, NULL );
      vlAutoFree( ans );      
    } else
      vlSleep( 10 );
  }

  return 0;
}




//// If we wanted to do it in this thread (which we shouldnt because we want all threads to be identical):
//// Run the prelude.
//vlRunString( "assert( loadstring( vl.getGlobal( 'prelude' ), '<prelude>' ) )()", "<prelude>" );
//// Run the program.
//vlRunString( "assert( loadstring( vl.getGlobal( 'main' ), '<main>' ) )()", "<main>" );
//// Run the finale.
//vlRunString( "assert( loadstring( vl.getGlobal( 'finale' ), '<finale>' ) )()", "<finale>" );