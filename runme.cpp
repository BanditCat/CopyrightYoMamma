// This is hereby placed in the public domain pro bono publico.

// 2006 BanditCat



#include <windows.h>
#include <string>
#include <iostream>
#include <cstdio>
#include "resource.h"


// The help dialog message.
static const LPWSTR helpMessage = \
L"This program is used to launch commands based on in its own filename.  The \n"
L"primary use is to launch batch files without showing a window.  This program\n"
L"has three modes of operation, depending on its filename when executed.  In all \n"
L"modes of operation, the first thing the program does is to change the current \n"
L"working directory to the path where the file is located.  \n"
L"\n"
L"If the program is run as either runme.exe or just runme it will just display\n"
L"this dialog.\n"
L"\n"
L"The second mode of operation is triggered if there are no command line \n"
L"arguments, but are at least two periods; in the second mode of operation, the\n"
L"program parses from left to right everything between the last two periods and \n"
L"treats it as follows:\n"
L"\n"
L"w\n"
L"      w causes the program to wait for the launched process to terminate \n"
L"      before exiting itself.  The default is to quit immediately after \n"
L"      launching the command.\n"
L"s\n"
L"      s causes the program to show a window for the executed command, although\n"
L"      many programs (e.g. notepad) will display a window anyway.  Without\n"
L"      this option a batch file would not show a dosbox.\n"
L"m\n"
L"      m causes a messagebox to display with the command to be executed, this\n"
L"      is useful if you run across this program and you forget what command\n"
L"      it executes.  This option precludes the command from actually running.\n"
L"d\n"
L"      d signals the end of options so that you can include a comment in the\n"
L"      filename.\n"
L"\n"
L"Any unrecognized options are ignored.\n"
L"\n"
L"After option parsing, the command to be executed is determined from everything\n"
L"preceding the second to last period.  Any characters that are not in the set\n"
L"[a-p] will corrupt the command. Every two characters are decoded to one character\n"
L"until there are less then two characters remaining.  Decoding is in the order low\n"
L"nybble then high nybble.  The encoded command is then executed.  If there is a\n"
L"error, nothing happens.\n"
L"\n"
L"If any command line arguments are given, then no command is executed, instead\n"
L"the first argument is encoded, then the executable is copied to a new name \n"
L"that corresponds to the encoded command.  If a file exists with that name then\n"
L"copying does not occur.\n\n"
L"Examples:\n\n"
L"runme.exe \"foo.bat /?\"\n"
L"           This will produce an exucutable named ggpgpgoccgbgehacpcpd.s.exe that\n"
L"           will execute the above command when run.\n"
L"ggpgpgoccgbgehacpcpd.s.exe\n"
L"           This will run \"foo.bat /?\" with a window.\n"
L"ggpgpgoccgbgehacpcpd.m.exe\n"
L"           This will display a message box with \"foo.bat /?\".";


using namespace std;

int WinMain(  HINSTANCE notUsedH,
              HINSTANCE notUsedPI,
              LPSTR notUsedCL,
              int notUsedCS ){

  wstring name( L"" );

  // Get the current directory.
  {
    LPWSTR cd = new WCHAR[ MAX_PATH + 10 ];
    DWORD sz = GetModuleFileNameW( NULL, cd, MAX_PATH + 5 );
    name = cd;
    --sz;
    while( cd[ sz ] != '\\' && sz )
      --sz;
    if( !sz )
      exit( EXIT_SUCCESS );
    cd[ sz ] = '\0';
    SetCurrentDirectoryW( cd );
    delete[] cd;
  }


  // Take off path.
  {
    size_t p( name.size() - 1 );
    while( name[ p ] != '\\' && p )
      --p;
    if( p == name.size() - 1 )
      exit( EXIT_SUCCESS );
    if( p )
      ++p;
    name = name.substr( p, name.size() - p );
  }
  // Check for bogusness.
  if( name.empty() )
    exit( EXIT_SUCCESS );

  // If there are args then copy executable to new name.
  int num;
  LPWSTR* namev = CommandLineToArgvW( GetCommandLineW(), &num );
  if( num > 1 ){
    wstring preEnc( namev[ 1 ] );
    wstring enc( L"" );
    for( size_t i = 0; i < preEnc.size(); ++i ){
      enc += ( ( (unsigned char)( preEnc[ i ] ) ) & 0x0f ) + 'a';
      enc += ( ( (unsigned char)( preEnc[ i ] ) ) >> 4 ) + 'a';
    }
    enc += L"..exe";
    CopyFileW( name.c_str(), enc.c_str(), TRUE );
    // Free namev.
    LocalFree( namev );
    exit( EXIT_SUCCESS );
  }
  LocalFree( namev );


  // Take off extension.
  if( name.size() <= 4 )
    exit( EXIT_SUCCESS );
  name.resize( name.size() - 4 );

  // Check for bogusness.
  if( name.empty() )
    exit( EXIT_SUCCESS );

  // If it is named runme spit out help.
  if( name == L"runme" ){
    MessageBoxW( NULL, helpMessage, L"Help", 0 );
    exit( EXIT_SUCCESS );
  }

  // Read options into second string.
  wstring options( L"" );
  {
    wstring::iterator optp( name.end() - 1 );
    while( *optp != '.' && optp != name.begin() ){
      options = *optp + options;
      --optp;
    }
    if( optp == name.begin() )
      exit( EXIT_SUCCESS );
  }
  // Chop options off name.
  name.resize( name.size() - ( options.size() + 1 ) );

  // Now set name to actual command line.
  {
    wstring preDecode( name );
    name = L"";
    size_t i = 0;
    while( i < preDecode.size() ){
      unsigned char c1 = ( preDecode[ i ] - 'a' ) & 0x0f;
      i += 1;
      if( i != preDecode.size() ){
        unsigned char c2 = ( preDecode[ i ] - 'a' ) & 0x0f;
        name += ( c2 << 4 ) + c1;
        i += 1;
      }
    }
  }

  // Options parsing.
  bool waitForProcess = false;
  DWORD showWindow = CREATE_NO_WINDOW;
  for( size_t i = 0; i < options.size(); ++i ){
    wchar_t c = tolower( options[ i ] );
    switch( c ){
      case 'd':
        goto doneParsing;
      case 's':
        showWindow = 0;
        break;
      case 'w':
        waitForProcess = true;
        break;
      case 'm':
        MessageBoxW( NULL, name.c_str(), L"The command:", 0 );
        exit( EXIT_SUCCESS );
    }
  }
doneParsing:

  // Debug with these if you like.
  //MessageBoxW( NULL, name.c_str(), name.c_str(), 0 );
  //MessageBoxW( NULL, options.c_str(), options.c_str(), 0 );

  STARTUPINFOW si;
  PROCESS_INFORMATION pi;

  ZeroMemory( &si, sizeof( si ) );
  si.cb = sizeof(si);
  ZeroMemory( &pi, sizeof( pi ) );

  // Start it. 
  if( !CreateProcessW( NULL,             // Use a command line.
                       (LPWSTR)name.c_str(),     // What to run.
                       NULL,             // Not inheritable. 
                       NULL,             // Not inheritable. 
                       FALSE,            // Not Inheritable. 
                       showWindow,       // The whole point of this program. 
                       NULL,             // Use the parents environment block. 
                       NULL,             // Use the parents starting directory. 
                       &si,              // The startup information.
                       &pi ) ){          // Where to storte process information.
    exit( EXIT_SUCCESS );  // Die if unsucessfull.
  }

  // Wait for process if we want.
  if( waitForProcess )
    WaitForSingleObject( pi.hProcess, INFINITE );

  // Close handles. 
  CloseHandle( pi.hProcess );
  CloseHandle( pi.hThread );
  
  exit( EXIT_SUCCESS );
}
