#ifndef STABAI_H
#define STABAI_H

// TODO: random data implemintation is too slow, replace deque< u32 > with better replacement.

#include <limits>
#include <deque>
#include <set>

#define STABAI_VERSION "0.01.051612sa"


#ifdef SAFE
#define assess( t, msg ) if( !( t ) ) vdie( msg )
#else
#define assess( t, msg ) 
#endif

#define assert( t ) if( !( t ) ) vdie( "Assertion failed!" );

#define pi 3.14159265358979323846264338327950288419716939937510
#define torad ( 3.14159265358979323846264338327950288419716939937510f / 180.0f )

// Types.
typedef unsigned char u8;
typedef unsigned short int u16;
typedef unsigned int u32;
typedef unsigned long long int u64;
typedef signed char s8;
typedef signed short int s16;
typedef signed int s32;
typedef signed long long int s64;
typedef float f32;
typedef double f64;



#if CHAR_BIT != 8 
#error Alien build envioronment, check vislib.h for correct typedefs.
#endif 
#if INT_MAX != 2147483647 || INT_MIN != -2147483648
#error Alien build envioronment, check vislib.h for correct typedefs.
#endif 
#if UINT_MAX != 4294967295 
#error Alien build envioronment, check vislib.h for correct typedefs.
#endif 
#if SHRT_MAX != 32767 || SHRT_MIN != -32768 
#error Alien build envioronment, check vislib.h for correct typedefs.
#endif 
#if USHRT_MAX != 65535 
#error Alien build envioronment, check vislib.h for correct typedefs.
#endif 
#if SCHAR_MAX != 127 || SCHAR_MIN != -128
#error Alien build envioronment, check vislib.h for correct typedefs.
#endif 
#if UCHAR_MAX != 255 
#error Alien build envioronment, check vislib.h for correct typedefs.
#endif 
#if LLONG_MAX != 9223372036854775807 || LLONG_MIN != 9223372036854775808
#error Alien build envioronment, check vislib.h for correct typedefs.
#endif 
#if ULLONG_MAX != 18446744073709551615
#error Alien build envioronment, check vislib.h for correct typedefs.
#endif 



// readable AES noise with a limit of 2 GB
class AESRand{
public:
  AESRand( u64 nonce, u32 size = 1 );
  ~AESRand();

  const u32& operator[]( u32 idx );
private:
  class AESRandImpl* impl;
};


struct ProgramRunInfo{
  u64 instructionCount;
  u32 maxStackSize;
  u32 randomDataConsumed;
};
// A 32-bit RISC stack based program.  It has multiple inputs and the contents of the stack at termination are the output.
// There is an insturction pointer as the only state besides the stack.  Input 0 is always the program itself and input 1 is always a cryptographically secure noise source (AES-256).
class Program{
public:
  Program( std::deque< u32 >& insts, std::deque< const std::deque< u32 >* >& inps, std::deque< u32 >& stk, u64 nonce );  
  ~Program( void );

  // Ticks the program. returns false iff the program is done executing.
  bool tick( void );
  // Runs the program until it stops, exceeds the max stack size, or instruction count.
  // returns a programRunInfo with details of the execution.
  void run( ProgramRunInfo& pri, u64 maxInstructionCount, u32 maxStackSize );

  // Traces the program, showing the stack and ip at break points.
  std::string trace( u32 steps, const std::set< u32 >& breakpoints );
  std::string programToString( void );
  std::string stackToString( void );
  
  void renonce( u64 nonce ); 
  inline void setInstructions( std::deque< u32 >& i ){ insts = &i; }

  void reset( void );
  void clear( void );

  inline std::deque< u32 >& stack( void ){ return *stk; }
  inline const std::deque< u32 >& stack( void ) const{ return *stk; }
  inline void setStack( std::deque< u32 >& newStack ){ stk = &newStack; }
  inline std::deque< u32 >& instructions( void ){ return *insts; }
  inline const std::deque< u32 >& instructions( void ) const{ return *insts; }
  inline std::deque< const std::deque< u32 >* >& inputs( void ){ return *inps; };
  inline const std::deque< const std::deque< u32 >* >& inputs( void ) const{ return *inps; };
  u32 instructionPointer( void ) const;

  // parses a string and adds it to the instuctions
  bool addInstruction( const std::string& instruction );
  // adds a deque to the list of inputs at the back
  void addInput( const std::deque< u32 >& input );
  static std::string instructionToString( u32 instruction );


private:
  Program operator=( const Program& p );
  std::deque< u32 >* insts;
  std::deque< const std::deque< u32 >* >* inps;
  u32 ip;
  u32 rp;
  std::deque< u32 >* stk;
  AESRand* rand;
};


// This represents a pool of programs that evolves over time.  
// This class must e subclassed with defined preps, breed and score.  score and breed assigns a
// score based on a program run info and the stack/inputs.  the preps sets up the inputs
// for a run, and score and breed must set it back.  Both breed and score are always run, there scores added, the difference being a scores stack is discarded while a breeds stack is treated as a new program.
// The max stack size and max instruction count of a run is determined by 
// a sliding average over the last slidingAverageCount runs.  When the pool size
// reaches averagePoolSize * cullScalar, it is culled down to avaragePoolSize / cullScalar.


#define GENERATION_BASE 0.5
// number of times to test new offspring.
#define START_TEST_COUNT 2

class ProgramPool{
public:
  ProgramPool( const Program& adam, u32 averagePoolSize, f64 cullScalar, u32 slidingAverageCount, u64 nonce );
  ~ProgramPool( void ); 

  virtual void scorePrep( void ) = 0;
  virtual f64 score( const ProgramRunInfo& pri, const std::deque< u32 >& stack, const std::deque< u32 >& newProg ) = 0;
  virtual void breedPrep( void );
  virtual f64 breed( const ProgramRunInfo& pri, const std::deque< u32 >& prog, const std::deque< u32 >& newProg );

  std::string programPoolToString( void );
  std::string highestScore( void );
  

  void populate( void );

protected:
  inline f64 scaleScore( u32 generation ){ return pow( GENERATION_BASE, (f64)generation ); }
  inline f64 averageTicks( void ){ if( !tcs.size() ) return 0.0; f64 ans = 0.0; for( u32 i = 0; i < tcs.size(); ++i ) ans += (f64)tcs[ i ]; return ans / tcs.size(); }
  inline f64 averageMaxStackSize( void ){ if( !mss.size() ) return 0.0; f64 ans = 0.0; for( u32 i = 0; i < mss.size(); ++i ) ans += (f64)mss[ i ]; return ans / tcs.size(); }
  void cull( void );

  struct PPMember{
    std::deque< u32 > insts;
    f64 score;
    f64 scoreDenom;

    PPMember* parent;
    u32 parentGenerationGap;
  };
  std::deque< PPMember* > progs;

  std::deque< const std::deque< u32 >* > inps;

  u32 aps;
  u32 rp;
  u32 slidingCnt;
  std::deque< u64 > tcs;
  std::deque< u32 > mss;
  f64 cullSclr;
  AESRand* rand;
};

class FibProgramPool : public ProgramPool{
public:
  FibProgramPool( const Program& adam, u32 averagePoolSize, f64 cullScalar, u32 slidingAverageCount, u64 nonce );
  ~FibProgramPool( void ); 

  void scorePrep( void );
  f64 score( const ProgramRunInfo& pri, const std::deque< u32 >& stack, const std::deque< u32 >& newProg );

};

u32 levenshteinDistance( const std::deque< u32 >& d1, const std::deque< u32 >& d2 ); 

#define opcodeCount "60"
#define opcodeImmediateJumpStart "29"
#define opcodeImmediateJumpEnd "38"


#include "adam.h"


#endif //STABAI_H