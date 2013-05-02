// All code not explictly labled as others work is copyright Jon DuBois 2011.  All rights reserved.

#ifndef PROGRAM_H
#define PROGRAM_H

#include "vutil.h"

typedef struct {
  u32 operations;
  u32 results;
  u32 arguments;
  u32 size;
  u32 bits;
} Program;


Program* newProgram( const vbitField* );




#endif // PROGRAM_H