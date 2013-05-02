#include "vislib.h"

typedef struct IStreamVtbl{
  s32 ( __stdcall *QueryInterface )( u8* This, const u8* const riid, void** ppvObject );
  u32 ( __stdcall *AddRef )( u8* This );
  u32 ( __stdcall *Release )( u8* This );
  HRESULT ( __stdcall *Read )( u8* This, void* pv, ULONG cb, ULONG* pcbRead );
  HRESULT ( __stdcall *Write )( u8* This, const void* pv, ULONG cb, ULONG* pcbWritten );
  HRESULT ( __stdcall *Seek )( u8* This, LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER* plibNewPosition );
  HRESULT ( __stdcall *SetSize )( u8* This, ULARGE_INTEGER libNewSize );
  HRESULT ( __stdcall *CopyTo )( u8* This, u8* pstm, ULARGE_INTEGER cb, ULARGE_INTEGER* pcbRead, ULARGE_INTEGER* pcbWritten );
  HRESULT ( __stdcall *Commit )( u8* This, u32 grfCommitFlags);
  HRESULT ( __stdcall *Revert )( u8* This );
  HRESULT ( __stdcall *LockRegion )( u8* This, ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType);
  HRESULT ( __stdcall *UnlockRegion )( u8* This, ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType );
  HRESULT ( __stdcall *Stat )( u8* This, u8* pstatstg, DWORD grfStatFlag );
  HRESULT ( __stdcall *Clone )( u8* This, u8** ppstm );
} IStreamVtbl;
typedef struct{
  const struct IStreamVtbl *lpVtbl;
} IStream;
typedef struct{
  u32 x, y, width, height;
} prect;
typedef struct{
  u32 GdiplusVersion;
  u8** DebugEventCallback;
  BOOL SuppressBackgroundThread;
  BOOL SuppressExternalCodecs;
} GdiplusStartupInput;

typedef struct{
  u32 Width;
  u32 Height;
  s32 Stride;
  s32 PixelFormat1;
  u8* Scan0;
  int* Reserved;
} BitmapData;
u32 __stdcall GdiplusStartup( u32** token,  GdiplusStartupInput* si, u8* isNull );
void __stdcall GdiplusShutdown( u32* token );
u32 __stdcall GdipBitmapLockBits( u8* bitmap, const prect* rect, u32 flags, s32 format, BitmapData* lockedBitmapData );
u32 __stdcall GdipBitmapUnlockBits( u8* bitmap, BitmapData* lockedBitmapData );
u32 __stdcall GdipGetImageWidth( u8* image, u32* dim );
u32 __stdcall GdipGetImageHeight( u8* image, u32* dim );
u32 __stdcall GdipCreateBitmapFromFile( const u8* filename, u8** bitmap );
u32 __stdcall GdipCreateBitmapFromStream( IStream* str, u8** bitmap );

void pvloadbmap( u8* bmap, u8** data, u32* width, u32* height ){
  prect pr = { 0, 0, 0, 0 };
  BitmapData lbd;

  GdipGetImageWidth( bmap, width );
  GdipGetImageHeight( bmap, height );
  pr.width = *width; 
  pr.height = *height;
  if( GdipBitmapLockBits( bmap, &pr, 1, ( 8 | (24 << 8) | 0x00020000 ), &lbd ) )
    vdie( "Unable to lock image in vloadJpegFromFile" );

  *data = vsmalloc( 3 * *width * *height );

  {
    u32 x, y;
    for( x = 0; x < pr.width; ++x )
      for( y = 0; y < pr.height; ++y ){
        ( *data )[ ( x + y * pr.width ) * 3 + 0 ] = lbd.Scan0[ ( x + ( ( pr.height - y ) - 1 ) * pr.width ) * 3 + 2 ];
        ( *data )[ ( x + y * pr.width ) * 3 + 1 ] = lbd.Scan0[ ( x + ( ( pr.height - y ) - 1 ) * pr.width ) * 3 + 1 ];
        ( *data )[ ( x + y * pr.width ) * 3 + 2 ] = lbd.Scan0[ ( x + ( ( pr.height - y ) - 1 ) * pr.width ) * 3 + 0 ];
      }
  }
  GdipBitmapUnlockBits( bmap, &lbd );      
}

// returns 0 on failure.  data must be freed with vsfree.
int vloadImageFromFile( const u8* filename, u8** data, u32* width, u32* height ){
  GdiplusStartupInput si;
  u8* bmap = NULL;
  u32* token;
  u32 i;
  u32 fnsz = vstrlen( filename );
  u32 rfnsz = fnsz * 2 + 10;
  u8* rfn = vsmalloc( rfnsz );
  for( i = 0; i < rfnsz; ++i )
    if( ( i % 2 ) || ( i / 2 >= fnsz ) )
      rfn[ i ] = '\0'; 
    else
      rfn[ i ] = filename[ i / 2 ];

  *data = NULL;

  si.DebugEventCallback = NULL;
  si.GdiplusVersion = 1;
  si.SuppressBackgroundThread = 0;
  si.SuppressExternalCodecs = 0;
  if( GdiplusStartup( &token, &si, NULL ) )
    return 0;
  if( !GdipCreateBitmapFromFile( rfn, &bmap ) )
    pvloadbmap( bmap, data, width, height );
  else{
    GdiplusShutdown( token );
    return 0;
  }

  GdiplusShutdown( token );
  return 1;
}

s32 __stdcall CreateStreamOnHGlobal( HGLOBAL hg, BOOL delOnRelease, IStream **is );

// returns 0 on failure.  data must be freed with vsfree.
int vloadImageFromBytes( const u8* inData, u32 inSize, u8** data, u32* width, u32* height ){
  GdiplusStartupInput si;
  u32* token;

  si.DebugEventCallback = NULL;
  si.GdiplusVersion = 1;
  si.SuppressBackgroundThread = 0;
  si.SuppressExternalCodecs = 0;
  if( GdiplusStartup( &token, &si, NULL ) )
    return 0;

  {
    IStream* is;
    u8* bmap = NULL;
    u32 wrtn;
    if( CreateStreamOnHGlobal( NULL, TRUE, &is ) != S_OK ){
      GdiplusShutdown( token );
      return 0;
    }
    if( is->lpVtbl->Write( (u8*)is, inData, inSize, &wrtn ) != S_OK ){
      is->lpVtbl->Release( (u8*)is );
      GdiplusShutdown( token );
      return 0;    
    }

    if( !GdipCreateBitmapFromStream( is, &bmap ) )
      pvloadbmap( bmap, data, width, height );
    else{
      is->lpVtbl->Release( (u8*)is );
      GdiplusShutdown( token );
      return 0;
    }

    is->lpVtbl->Release( (u8*)is );
  }

  GdiplusShutdown( token );
  return 1;
}