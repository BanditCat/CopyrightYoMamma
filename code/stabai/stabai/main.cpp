#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include "stabai.h"
#include <ctime>
// TODO: output file, record nonce, multithead. long term: OpenCL
using namespace std;

#define NONCE 0xFEED0FACE1C0FFEE

int main( int argc, char** argv ){
  ofstream ofs;
  ofs.open( "log.txt" );
  argc; argv;
  string s;
  try {
    deque< u32 > inp1;
    deque< const deque< u32 >* > inps;
    deque< u32 > stk;
    deque< u32 > insts;

    Program prog( insts, inps, stk, 0 );
    inp1.push_back( 0 );
    prog.addInput( inp1 );

    //Load adam
    for( u32 i = 0; i < ( sizeof( adamSource ) / sizeof( const char * ) ); ++i ){
      if( !prog.addInstruction( string( adamSource[ i ] ) ) ){
        ostringstream err;
        err << "Error parsing line " << i << ": " << adamSource[ i ] << "\n";
        throw err.str();
      }
    }

    {
      clock_t start, end;
      start = clock();

      FibProgramPool fp( prog, 800, 2.0, 1000, NONCE );
      ofs << "Using nonce " << NONCE << "\n" << fp.programPoolToString() << "\n\n";

      for( u32 i = 0; i < 10000000; ++i ){
        for( u32 j = 0; j < 2560; ++j )
          fp.populate();
        ofs << fp.programPoolToString() << "\niteration " << i << "\n" << fp.highestScore() << "\n\n";
        cout << fp.programPoolToString() << "\niteration " << i << "\n" << fp.highestScore() << "\n\n";
      }

      //ProgramRunInfo pri;TODO: encapsulate aes as thread safe class, make all instructions run in constant time.
      //for( u32 u = 0; u < 1000; ++u ){
      //  prog.reset();
      //  prog.run( pri, 165536, 1024 );
      //  {
      //    deque< u32 > s2;
      //    Program p2( prog.stack(), inps, s2, clock() );
      //    p2.run( pri, 165536, 1024 );
      //    {
      //      deque< u32 > s3;
      //      Program p3( p2.stack(), inps, s3, clock() + 1337  );
      //      p3.run( pri, 165536, 1024 );
      //      if( pri.instructionCount > 10 )
      //        ofs << "ticks: " << pri.instructionCount << ", random data consumed: " << pri.randomDataConsumed << ", maxStackSize: " << pri.maxStackSize << "\n\n";
      //    }
      //  }
      //}
      end = clock();
      ofs << "\n\ntotal time: " << end - start << "\n\n";
    }

    ofs << "Hello World!";
  } catch( string& s ){
    ofs << "\n\n\nERROR: " << s << "\n\n\n";
  } catch( const char* s ){
    ofs << "\n\n\nERROR: " << s << "\n\n\n";
  }
  ofs.close();
  cin >> s;
}