#ifndef ADAM_H
#define ADAM_H


#define ADAM_MAX_INSTRUCTION_COUNT 131072
#define ADAM_MAX_STACK_SIZE 1024


#define ADAM_BREED_SCALAR 100000.0
#define ADAM_MIN_TICKS 131072
#define ADAM_MIN_MAX_STACK_SIZE 1024

static const char* adamSource[] = {
  "PSIM 0",

  "DUPL 0",//1:  Start               | counter, insts[ 0 ], insts[ 1 ]....
  "PSIM 0",
  "PBIN 0",
  "ADDI 1",
  "DUPL 0",
  "PSSI 0",
  "BRNE 1",
  "POPI 1",

  "JMPI 123",//call?
  "STOP 0",






  "PSSS 0",//11: Remove random element, adjusting immediate jumps. ignore back element
  "PSIM 2",
  "BRLS 15",
  "RETR 0",//14: return if stack is empty.

  "PSSS 0",
  "SUBI 2",
  "PSII 4096",
  "MODR 1",
  "PSIM 0",//19:                     | counter = 0, rand % ssize, insts[ 0 ], ...

  "PSRL 1",
  "PSRL 1",
  "BRLS 26",//22: skip if cntr < rand % ssize

  "DUPL 0",
  "ADDI 4",
  "JMPI 28",
  "DUPL 0",
  "ADDI 3",


  "PSRD 0",//28:                     | ni = insts[ cntr + 0|1 ], cntr, rand % ssize, insts[ 0 ], ...




  "DUPL 0",
  "SHRI 24",//30:                    | niOpcode, ni, cntr, rand % ssize, insts[ 0 ], ... 

  "PSIM " opcodeImmediateJumpStart,
  "PSRL 1",
  "BRLS 50",

  "PSIM " opcodeImmediateJumpEnd,
  "PSRL 1",
  "BRGR 50",

  "PSIM 1",
  "SHLI 24",
  "SUBI 1",
  "PSRL 2",
  "ANDR 1",//41:                    | niData, niOpcode, ni, cntr, rand % ssize, insts[ 0 ], ... 

  "PSRL 4",
  "PSRL 1",
  "BRLS 46",

  "SUBI 1",

  "SWAP 1",
  "SHLI 24",
  "ADDR 1",
  "SWAP 1",
  "POPI 1",//50:                     | ni, cntr, rand % ssize, insts[ 0 ], ... 


  "PSRL 1",
  "ADDI 4",
  "SETR 1",//53:                     | cntr + 4, ni, cntr, rand % ssize, insts[ 0 ], ...
  "POPI 2",

  "ADDI 1",//55:                     | cntr += 1, rand % ssize, insts[ 0 ], ...
  "PSSS 0",
  "SUBI 3",
  "PSRL 1",//                        | cntr, ss, cntr, rand % ssize, insts[ 0 ], ...
  "BRLS 20",
  "PPBI 1",
  "RETR 2",


  "PSII 4096",//62: Add random op to top of stack
  "PSSI 0",
  "PSRL 1",
  "MODR 1",
  "SWAP 1",
  "SHRI 24",
  "MODI " opcodeCount,
  "SHLI 24",
  "ADDR 1",
  "RETR 0",



  "CALL 62",//72: insert random op randomly into stack. ignore back element
  "DUPL 0",
  "PSSS 0",
  "SUBI 3",
  "PSII 4096",
  "MODR 1",
  "PSIM 0",//78:                      | cntr = 0, rand % ssize, insts[ 0 ], insts[ 0 ], insts[ 1 ],...

  "DUPL 0",
  "ADDI 4",//80:                      | cntr + 4(*insts[ cntr ]), cntr, rand % ssize, insts[ 0 ], insts[ 0 ], insts[ 1 ],...

  "PSRL 2",//81: cntr < rand % ssize?
  "PSRL 2",
  "BRGE 85",
  "ADDI 1",

  "PSRL 2",//85: cntr == rand % ssize?
  "PSRL 2",
  "BRNE 90",
  "POPI 1",
  "PSIM 3",

  "PSRD 0",//90:                     | ni, cntr, rand % ssize, insts[ 0 ], insts[ 0 ], insts[ 1 ],...


  "DUPL 0",
  "SHRI 24",//92:                    | niOpcode, ni, cntr, rand % ssize, insts[ 0 ], insts[ 0 ], insts[ 1 ],...

  "PSIM " opcodeImmediateJumpStart,
  "PSRL 1",
  "BRLS 112",

  "PSIM " opcodeImmediateJumpEnd,
  "PSRL 1",
  "BRGR 112",

  "PSIM 1",
  "SHLI 24",
  "SUBI 1",
  "PSRL 2",
  "ANDR 1",//103:                    | niData, niOpcode, ni, cntr, rand % ssize, insts[ 0 ], insts[ 0 ], insts[ 1 ],...

  "PSRL 4",
  "PSRL 1",
  "BRLS 108",

  "ADDI 1",

  "SWAP 1",
  "SHLI 24",
  "ADDR 1",
  "SWAP 1",
  "POPI 1",//112:                     | ni, cntr, rand % ssize, insts[ 0 ], insts[ 0 ], insts[ 1 ],...

  "PSRL 1",
  "ADDI 5",
  "SETR 1",
  "POPI 2",

  "ADDI 1",//117:                     | cntr += 1, rand % ssize, insts[ 0 ], insts[ 0 ], insts[ 1 ],...

  "PSSS 0",
  "SUBI 4",
  "PSRL 1",
  "BRLS 79",

  "RETR 3",

  "PSIM 7",//123   remove rand % 7 elements
  "PSII 4096",
  "MODR 1",
  "DUPL 0",
  "PSIM 0",
  "BREQ 141",//128: jmp if remove none.
  "PBRL 0",
  "POPI 1",

  "CALL 11",//131:
  "PSSS 0",
  "PSRD 0",
  "SUBI 1",
  "PPBI 1",
  "PBRL 0",

  "PSIM 0",
  "BRNE 131",
  "PPBI 1",
  "JMPI 142",

  "POPI 1",//141: remove spurious 0


  "PSIM 7",//142: modify rand % 7 elements
  "PSII 4096",
  "MODR 1",
  "DUPL 0",
  "PSIM 0",
  "BREQ 160",//147: break if mod none

  "CALL 62",
  "PSSS 0",
  "SUBI 2",
  "PSII 4096",
  "MODR 1",//152:                  | rand % psize, ni, cntr, ...

  "ADDI 3",
  "SETR 1",
  "POPI 2",//155:                  | cntr, ...

  "SUBI 1",
  "PSIM 0",
  "PSRL 1",
  "BRNE 148",

  "POPI 1",//160: pop cntr

  "PSIM 7",//161:   remove rand % 7 elements
  "PSII 4096",
  "MODR 1",
  "DUPL 0",
  "PSIM 0",
  "BREQ 179",//166: jmp if remove none.
  "PBRL 0",
  "POPI 1",

  "CALL 72",//169:
  "PSSS 0",
  "PSRD 0",
  "SUBI 1",
  "PPBI 1",
  "PBRL 0",

  "PSIM 0",
  "BRNE 169",
  "PPBI 1",
  "JMPI 180",

  "POPI 1",//179: remove spurious 0


  "STOP 0",//180:



};


#endif //ADAM_H