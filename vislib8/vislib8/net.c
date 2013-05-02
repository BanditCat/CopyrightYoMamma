// make timeouts
#include "vislib.h"


#pragma warning( push )
#pragma warning( disable: 4668 4820 4255 4574 )
#include "winsock2.h"
#include "Ws2tcpip.h"
#pragma warning( pop )


static WSADATA pvndata;

static SOCKET* pvnscks = NULL;
static vncallback** pvncbs = NULL;

static SOCKET* pvncons = NULL;
static vncallback** pvnccbs = NULL;

static u32 pvnqs = 0;
static u32 pvnnqs = 0;
static u32 pvnscksSize = 0, pvnconsSize = 0;
static u32 pvnscksBSize = 1, pvnconsBSize = 1;
static u32* pvncids = NULL;
static u32* pvnfids = NULL;
static u32 pvnfrees = 0;
static f32* pvntimes = NULL;

#define vnbufsize 66600

void pvninit( void ){
  int s;
  s = WSAStartup( MAKEWORD(2,2), &pvndata );
  if( s ){
    vdie( "WSAStartup failed!" );
    return;
  }
  vlogInfo( "vninit completed\n" );
  if( !pvnqs ){
    pvnqs = vmalloc( 0 );
    pvnnqs = 0;
  }
  pvnscks = vsmalloc( sizeof( SOCKET ) );
  pvncons = vsmalloc( sizeof( SOCKET ) );
  pvncbs = vsmalloc( sizeof( vncallback* ) );
  pvnccbs = vsmalloc( sizeof( vncallback* ) );
  pvncids = vsmalloc( sizeof( u32 ) );
  pvnfids = vsmalloc( sizeof( u32 ) );
  pvntimes = vsmalloc( sizeof( f32 ) );
}

void pvncheck( const u8* msg, int s, SOCKET* cs, struct addrinfo* ai ){
  if( s == SOCKET_ERROR ){
    int err = WSAGetLastError();
    if( err != 10053 && err != 10054 ){
      u32 em = vmalloc( 0 );
      vappendString( em, msg );
      vappendString( em, "\nError " ); vappendInt( em, err, 0 ); vappendString( em, ": " );
      {
        u8* ls;
        FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, err, 0, (LPTSTR)&ls, 0, NULL );
        vappendString( em, ls );
        LocalFree( ls );
      }
      if( cs != NULL )
        closesocket( *cs );
      if( ai != NULL )
        freeaddrinfo( ai );
      vdie( vmem( em ) );
    }
  }
}

void pvnquit( void ){ 
  u32 i;
  if( pvnqs ){
    u32* qs = (u32*)vmem( pvnqs );
    for( i = 0; i < pvnnqs; ++i ){
      if( qs[ i ] )
        verase( qs[ i ] );
    }
  }
  if( pvnscks != NULL ){
    for( i = 0; i < pvnscksSize; ++i )
      closesocket( pvnscks[ i ] );
    vsfree( pvnscks );
    pvnscks = NULL;
  }
  if( pvncons != NULL ){
    for( i = 0; i < pvnconsSize; ++i )
      closesocket( pvncons[ i ] );
    vsfree( pvncons );
    pvncons = NULL;
  }
  if( pvncbs != NULL ){
    vsfree( pvncbs );
    pvncbs = NULL;
  }
  if( pvnccbs != NULL ){
    vsfree( pvnccbs );
    pvnccbs = NULL;
  }
  if( pvnfids != NULL ){
    vsfree( pvnfids );
    pvnfids = NULL;
  }
  if( pvncids != NULL ){
    vsfree( pvncids );
    pvncids = NULL;
  }
  if( pvntimes != NULL ){
    vsfree( pvntimes );
    pvntimes = NULL;
  }
  pvnfrees = 0;
  pvnscksSize = pvnconsSize = 0;
  pvnscksBSize = pvnconsBSize = 1;
  WSACleanup(); 
}

void pvnaddsock( const SOCKET* cs, vncallback vcb ){
  if( pvnscksSize >= pvnscksBSize ){
    u32 i;
    SOCKET* ts = vsmalloc( sizeof( SOCKET ) * pvnscksBSize * 2 );
    vncallback** tcbs = vsmalloc( sizeof( vncallback* ) * pvnscksBSize * 2 );
    pvnscksBSize *= 2;
    for( i = 0; i < pvnscksSize; ++i ){
      ts[ i ] = pvnscks[ i ];
      tcbs[ i ] = pvncbs[ i ];
    }
    vsfree( pvnscks ); pvnscks = ts;
    vsfree( pvncbs ); pvncbs = tcbs;
  }
  pvncbs[ pvnscksSize ] = vcb;
  pvnscks[ pvnscksSize ] = *cs;
  ++pvnscksSize;
}

void pvnremcon( u32 ind ){
  u32 i = ind;
  u32* qs = (u32*)vmem( pvnqs );
  u32 tq = qs[ ind ];
  pvnccbs[ ind ]( NULL, 0, pvncids[ i ], vcurTime() );
  closesocket( pvncons[ ind ] );
  --pvnconsSize;
  verase( qs[ i ] );
  pvnfids[ pvnfrees++ ] =  pvncids[ i ];
  while( i < pvnconsSize ){
    pvncons[ i ] = pvncons[ i + 1 ];
    pvnccbs[ i ] = pvnccbs[ i + 1 ];
    pvncids[ i ] = pvncids[ i + 1 ];
    qs[ i ] = qs[ i + 1 ];
    ++i;
  }
  qs[ pvnconsSize ] = tq;
}
void pvnaddcon( const SOCKET* cs, vncallback vcb ){
  if( pvnconsSize >= pvnconsBSize ){
    u32 i;
    SOCKET* ts = vsmalloc( sizeof( SOCKET ) * pvnconsBSize * 2 );
    vncallback** tcbs = vsmalloc( sizeof( vncallback* ) * pvnconsBSize * 2 );
    u32* tids = vsmalloc( sizeof( u32 ) * pvnconsBSize * 2 );
    u32* fids = vsmalloc( sizeof( u32 ) * pvnconsBSize * 2 );
    f32* times = vsmalloc( sizeof( f32 ) * pvnconsBSize * 2 );
    pvnconsBSize *= 2;
    for( i = 0; i < pvnconsSize; ++i ){
      ts[ i ] = pvncons[ i ];
      tcbs[ i ] = pvnccbs[ i ];
      tids[ i ] = pvncids[ i ];
      fids[ i ] = pvnfids[ i ];
      times[ i ] = pvntimes[ i ];
    }
    vsfree( pvncons ); pvncons = ts;
    vsfree( pvnccbs ); pvnccbs = tcbs;
    vsfree( pvncids ); pvncids = tids;
    vsfree( pvnfids ); pvnfids = fids;
    vsfree( pvntimes ); pvntimes = times;
  }
  while( pvnnqs < pvnconsBSize ){
    u32 nq = vmalloc( 0 );
    vappend( pvnqs, &nq, sizeof( u32 ) );
    ++pvnnqs;
  }
  pvnccbs[ pvnconsSize ] = vcb;
  verase( ( (u32*)vmem( pvnqs ) )[ pvnconsSize ] );
  if( pvnfrees )
    pvncids[ pvnconsSize ] = pvnfids[ --pvnfrees ];
  else
    pvncids[ pvnconsSize ] = pvnconsSize;
  pvncons[ pvnconsSize ] = *cs;
  pvntimes[ pvnconsSize ] = vcurTime();
  ++pvnconsSize;
  if( !vcb( NULL, 1, pvncids[ pvnconsSize - 1 ], vcurTime() ) )
    pvnremcon( pvnconsSize - 1 );
}



void vnlisten( u32 port, vncallback cb ){
  int s; 
  struct addrinfo* res = NULL;
  struct addrinfo hnts;

  if( pvnscks == NULL )
    pvninit();

  vzero( hnts );
  hnts.ai_family = AF_INET;
  hnts.ai_socktype = SOCK_STREAM;
  hnts.ai_protocol = IPPROTO_TCP;
  hnts.ai_flags = AI_PASSIVE;

  s = getaddrinfo( NULL, vintToString( port, 0 ), &hnts, &res );
  if( s ){
    vdie( "getaddrinfo failed!" );
  }

  {
    SOCKET cs;
    cs = socket( res->ai_family, res->ai_socktype, res->ai_protocol );
    if( cs == INVALID_SOCKET ){
      freeaddrinfo( res );
      vdie( "socket failed!" );
    }

    { int i = 1; setsockopt( cs, SOL_SOCKET, SO_EXCLUSIVEADDRUSE, (char *)&i, sizeof( int ) ); }

    pvncheck( "bind failed", bind( cs, res->ai_addr, (int)res->ai_addrlen ), &cs, res );

    freeaddrinfo( res );

    pvncheck( "listen failed", listen( cs, SOMAXCONN ), &cs, NULL );

    pvnaddsock( &cs, cb );
    vlogInfo( "Now listening on port " ); vlogInfo( vintToString( port, 0 ) ); vlogInfo( "\n" );
  }
}

void vntick(){
  if( pvnscks != NULL ){
    static fd_set rds, wts;
    u32 i, mfd = 0;
    struct timeval tv = { 0, 0 };
    f32 curt = vcurTime();

    mfd = 0;

    for( i = 0; i < pvnconsSize; ++i ){
      if( curt - pvntimes[ i ] > vn_timeout )
        pvnremcon( i );
    }  

    FD_ZERO( &rds );
    for( i = 0; i < pvnscksSize; ++i ){
      if( pvnscks[ i ] > mfd )
        mfd = pvnscks[ i ];
#pragma warning( push )
#pragma warning( disable: 4127 )
      FD_SET( pvnscks[ i ], &rds );
#pragma warning( pop )
    }  
    if( mfd ){ 
      pvncheck( "select failed!", select( mfd + 1, &rds, NULL, NULL, &tv ), NULL, NULL );
      for( i = 0; i < pvnscksSize; ++i ){
        if( FD_ISSET( pvnscks[ i ], &rds ) ){
          SOCKET as = accept( pvnscks[ i ], NULL, NULL );
          pvncheck( "accept failed", as, NULL, NULL );
          pvnaddcon( &as, pvncbs[ i ] );
        }
      }
    }

    FD_ZERO( &rds );
    FD_ZERO( &wts );
    mfd = 0;
    for( i = 0; i < pvnconsSize; ++i ){
      if( pvncons[ i ] > mfd )
        mfd = pvncons[ i ];
#pragma warning( push )
#pragma warning( disable: 4127 )
      FD_SET( pvncons[ i ], &rds );
      FD_SET( pvncons[ i ], &wts );
#pragma warning( pop )
    }  
    if( mfd ){
      pvncheck( "select failed!", select( mfd + 1, &rds, NULL, NULL, &tv ), NULL, NULL );

      for( i = 0; i < pvnconsSize; ++i ){
        u32 qn = ( (u32*)vmem( pvnqs ) )[ i ];
        if( FD_ISSET( pvncons[ i ], &rds ) ){
          static u8 buf[ vnbufsize ];
          int ans = recv( pvncons[ i ], buf, vnbufsize - 1, 0 );
          pvncheck( "recv failed!", ans, NULL, NULL );
          if( ans == SOCKET_ERROR )
            ans = 0;
          if( !ans ){
            pvnremcon( i );
          } else{
            ans = pvnccbs[ i ]( buf, ans, pvncids[ i ], vcurTime() );
            if( !ans )
              pvnremcon( i ); 
            else if( vsize( ans ) ){
              vappend( qn, vmem( ans ), vsize( ans ) );
            }
          }
        }
        if( FD_ISSET( pvncons[ i ], &wts ) && vsize( qn ) ){
          int ans = send( pvncons[ i ], vmem( qn ), vsize( qn ), 0 );
          pvncheck( "send failed!", ans, NULL, NULL );
          if( ans == SOCKET_ERROR )
            ans = 0;
          if( (u32)ans != vsize( qn ) ){
            u32 ns = vsize( qn ) - (u32)ans;
            u8* tb = vsmalloc( ns );
            vmemcpy( tb, ((u8*)vmem( qn )) + ans, ns );
            verase( qn );
            vappend( qn, tb, ns );
            vsfree( tb );
          }else
            verase( qn );
        }
      }
    }
  } 
}