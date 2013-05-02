#include "stabai.h"
#include <sstream>
#include <iomanip>
#include <map>
#include <cmath>

using namespace std;





static const char* opnames[] = {

  "STOP",// 0
  "NULL",// 1
  "PSIS",// 2
  "PBIS",// 3
  "PSSI",// 4
  "PBSI",// 5
  "PSIM",// 6
  "PBIM",// 7
  "PSRL",// 8
  "PBRL",// 9
  "PSRD",// 10
  "PBRD",// 11
  "PSSS",// 12
  "PBSS",// 13
  "PSNI",// 14
  "PBNI",// 15
  "PSII",// 16
  "PBII",// 17
  "PSIN",// 18
  "PBIN",// 19
  "DUPL",// 20
  "SWAP",// 21
  "SETR",// 22
  "POPI",// 23
  "PPBI",// 24
  "POPR",// 25
  "PPBR",// 26
  "JMPP",// 27
  "JMPR",// 28
  "JMPI",// 29 start immediate jumps
  "BREQ",// 30
  "BRNE",// 31
  "BRGR",// 32
  "BRGE",// 33
  "BRLS",// 34
  "BRLE",// 35
  "LOPI",// 36
  "LOPR",// 37 
  "CALL",// 38 end immediate
  "RETR",// 39
  "ADDI",// 40
  "ADDR",// 41
  "SUBI",// 42
  "SUBR",// 43
  "MULI",// 44
  "MULR",// 45
  "DIVI",// 46
  "DIVR",// 47
  "MODI",// 48
  "MODR",// 49
  "ANDI",// 50
  "ANDR",// 51
  "BORI",// 52
  "BORR",// 53
  "XORI",// 54
  "XORR",// 55
  "SHLI",// 56
  "SHLR",// 57
  "SHRI",// 58
  "SHRR",// 59

  "I058",// 058
  "I059",// 059
  "I060",// 060
  "I061",// 061
  "I062",// 062
  "I063",// 063
  "I064",// 064
  "I065",// 065
  "I066",// 066
  "I067",// 067
  "I068",// 068
  "I069",// 069
  "I070",// 070
  "I071",// 071
  "I072",// 072
  "I073",// 073
  "I074",// 074
  "I075",// 075
  "I076",// 076
  "I077",// 077
  "I078",// 078
  "I079",// 079
  "I080",// 080
  "I081",// 081
  "I082",// 082
  "I083",// 083
  "I084",// 084
  "I085",// 085
  "I086",// 086
  "I087",// 087
  "I088",// 088
  "I089",// 089
  "I090",// 090
  "I091",// 091
  "I092",// 092
  "I093",// 093
  "I094",// 094
  "I095",// 095
  "I096",// 096
  "I097",// 097
  "I098",// 098
  "I099",// 099
  "I100",// 100
  "I101",// 101
  "I102",// 102
  "I103",// 103
  "I104",// 104
  "I105",// 105
  "I106",// 106
  "I107",// 107
  "I108",// 108
  "I109",// 109
  "I110",// 110
  "I111",// 111
  "I112",// 112
  "I113",// 113
  "I114",// 114
  "I115",// 115
  "I116",// 116
  "I117",// 117
  "I118",// 118
  "I119",// 119
  "I120",// 120
  "I121",// 121
  "I122",// 122
  "I123",// 123
  "I124",// 124
  "I125",// 125
  "I126",// 126
  "I127",// 127
  "I128",// 128
  "I129",// 129
  "I130",// 130
  "I131",// 131
  "I132",// 132
  "I133",// 133
  "I134",// 134
  "I135",// 135
  "I136",// 136
  "I137",// 137
  "I138",// 138
  "I139",// 139
  "I140",// 140
  "I141",// 141
  "I142",// 142
  "I143",// 143
  "I144",// 144
  "I145",// 145
  "I146",// 146
  "I147",// 147
  "I148",// 148
  "I149",// 149
  "I150",// 150
  "I151",// 151
  "I152",// 152
  "I153",// 153
  "I154",// 154
  "I155",// 155
  "I156",// 156
  "I157",// 157
  "I158",// 158
  "I159",// 159
  "I160",// 160
  "I161",// 161
  "I162",// 162
  "I163",// 163
  "I164",// 164
  "I165",// 165
  "I166",// 166
  "I167",// 167
  "I168",// 168
  "I169",// 169
  "I170",// 170
  "I171",// 171
  "I172",// 172
  "I173",// 173
  "I174",// 174
  "I175",// 175
  "I176",// 176
  "I177",// 177
  "I178",// 178
  "I179",// 179
  "I180",// 180
  "I181",// 181
  "I182",// 182
  "I183",// 183
  "I184",// 184
  "I185",// 185
  "I186",// 186
  "I187",// 187
  "I188",// 188
  "I189",// 189
  "I190",// 190
  "I191",// 191
  "I192",// 192
  "I193",// 193
  "I194",// 194
  "I195",// 195
  "I196",// 196
  "I197",// 197
  "I198",// 198
  "I199",// 199
  "I200",// 200
  "I201",// 201
  "I202",// 202
  "I203",// 203
  "I204",// 204
  "I205",// 205
  "I206",// 206
  "I207",// 207
  "I208",// 208
  "I209",// 209
  "I210",// 210
  "I211",// 211
  "I212",// 212
  "I213",// 213
  "I214",// 214
  "I215",// 215
  "I216",// 216
  "I217",// 217
  "I218",// 218
  "I219",// 219
  "I220",// 220
  "I221",// 221
  "I222",// 222
  "I223",// 223
  "I224",// 224
  "I225",// 225
  "I226",// 226
  "I227",// 227
  "I228",// 228
  "I229",// 229
  "I230",// 230
  "I231",// 231
  "I232",// 232
  "I233",// 233
  "I234",// 234
  "I235",// 235
  "I236",// 236
  "I237",// 237
  "I238",// 238
  "I239",// 239
  "I240",// 240
  "I241",// 241
  "I242",// 242
  "I243",// 243
  "I244",// 244
  "I245",// 245
  "I246",// 246
  "I247",// 247
  "I248",// 248
  "I249",// 249
  "I250",// 250
  "I251",// 251
  "I252",// 252
  "I253",// 253
  "I254",// 254
  "I255",// 255

};





bool Program::addInstruction( const string& instruction ){
  if( instruction.size() < 6 || instruction[ 4 ] != ' ' )
    return false;
  string inst = instruction.substr( 0, 4 );
  u32 opc = 0;
  u32 data = 0;
  bool found = false;
  for( u32 i = 0; i < ( sizeof( opnames ) / sizeof( const char* ) ); ++i ){
    if( !inst.compare( opnames[ i ] ) ){
      found = true;
      opc = i;
      break;
    }
  }
  if( !found )
    return false;
  for( u32 i = 5; i < instruction.size(); ++i ){
    if( instruction[ i ] < '0' || instruction[ i ] > '9' || data >= ( 1 << 24 ) )
      return false;
    data = ( data * 10 ) + ( instruction[ i ] - '0' );
  }
  insts->push_back( ( opc << 24 ) + data );
  return true;
}

string Program::instructionToString( u32 instruction ){
  ostringstream ans;
  ans << opnames[ instruction >> 24 ];
  ans << " ";
  ans << ( instruction & ( ( 1 << 24 ) - 1 ) );
  return ans.str();
}







Program::Program( deque< u32 >& ins, deque< const deque< u32 >* >& inp, deque< u32 >& st, u64 nonce) : insts( &ins ), inps( &inp ), stk( &st ), ip( 0 ), rp( 0 ){
  rand = new AESRand( nonce, 1024 );
}
Program::~Program(){
  delete rand;
}


bool Program::tick( void ){
  if( !insts->size() || ip >= insts->size() )
    return false;
  u8 opcode = ( (*insts)[ ip ] >> 24 );
  u32 data = (*insts)[ ip ] & ( ( 1 << 24 ) - 1 );

  switch( opcode ){
    u32 stackSel;
    u32 idx;
    // STOP pop and stop
  case 0:
    for( u32 i = 0; i < data && stk->size(); ++i )
      stk->pop_front();    
    return false;
    // NULL null
  case 1:
    break;
    // PSIS pushInputSize
    // pushes the size of input number (*stk)[ data ]
    // Input 1 (random noise) always returns 0 size.  
    // if data is zero it also pops the stack before adding the new value.
  case 2:
    if( data >= stk->size() || (*stk)[ data ] >= ( inps->size() + 2 ) )
      return false;
    stackSel = (*stk)[ data ];
    if( !data )
      stk->pop_front();
    if( stackSel < 2 ){
      if( stackSel )
        stk->push_front( 0 );
      else
        stk->push_front( insts->size() );
    } else
      stk->push_front( (*inps)[ stackSel - 2 ]->size() );
    break;
    // PBIS pushbackInputSize
  case 3:
    if( data >= stk->size() || (*stk)[ data ] >= ( inps->size() + 2 ) )
      return false;
    stackSel = (*stk)[ data ];
    if( !data )
      stk->pop_front();
    if( stackSel < 2 ){
      if( stackSel )
        stk->push_back( 0 );
      else
        stk->push_back( insts->size() );
    } else
      stk->push_back( (*inps)[ stackSel - 2 ]->size() );
    break;
    // PSSI pushInputSizeImmediate
    // pushes the size of input number (*stk)[ data ]
    // Input 1 (random noise) always returns 0 size
  case 4:
    if( data >= ( inps->size() + 2 ) )
      return false;
    stackSel = data;
    if( stackSel < 2 ){
      if( stackSel )
        stk->push_front( 0 );
      else
        stk->push_front( insts->size() );
    } else
      stk->push_front( (*inps)[ stackSel - 2 ]->size() );
    break;
    // PBSI pushbackInputSizeImmediate
  case 5:
    if( data >= ( inps->size() + 2 ) )
      return false;
    stackSel = data;
    if( stackSel < 2 ){
      if( stackSel )
        stk->push_back( 0 );
      else
        stk->push_back( insts->size() );
    } else
      stk->push_back( (*inps)[ stackSel - 2 ]->size() );
    break;
    // PSIM pushImmediate
  case 6:
    stk->push_front( data );
    break;
    // PBIM pushbackImmediate
  case 7:
    stk->push_back( data );
    break;
    // PSRL pushRelative
  case 8:
    if( data >= stk->size() )
      return false;
    stk->push_front( (*stk)[ data ] );
    break;
    // PBRL pushbackRelative
  case 9:
    if( data >= stk->size() )
      return false;
    stk->push_back( (*stk)[ data ] );
    break;
    // PSRD pushRelativeDereference
    // if data is zero it also pops the stack before adding the new element.
  case 10:
    if( data >= stk->size() || (*stk)[ data ] >= stk->size() )
      return false;
    {
      u32 dt = (*stk)[ (*stk)[ data ] ];
      if( !data )
        stk->pop_front();
      stk->push_front( dt );
    }
    break;
    // PBRD pushbackRelativeDereference
  case 11:
    if( data >= stk->size() || (*stk)[ data ] >= stk->size() )
      return false;
    {
      u32 dt = (*stk)[ (*stk)[ data ] ];
      if( !data )
        stk->pop_front();
      stk->push_back( dt );
    }
    break;
    // PSSS pushStackSize
  case 12:
    stk->push_front( stk->size() );
    break;
    // PBSS pushbackStackSize
  case 13:
    stk->push_back( stk->size() );
    break;
    // PSNI push number of inputs ( not including self and random )
  case 14:
    stk->push_front( inps->size() );
    break;
    // PBNI pushback number of inputs ( not including self and random )
  case 15:
    stk->push_back( inps->size() );
    break;
    // PSII pushInputImmediate
    // pushes the value of input[ data >> 12 ] ][ data & ( ( 1 << 12 ) - 1 ) ]
    // the value of input[ 1 ][ anything ] is non-deterministic
  case 16:
    stackSel = ( data >> 12 );
    idx = data & ( ( 1 << 12 ) - 1 );
    if( stackSel >= inps->size() + 2 )
      return false;
    if( stackSel < 2 ){
      if( stackSel )
        stk->push_front( (*rand)[ rp++ ] );
      else{
        if( idx >= insts->size() )
          return false;
        stk->push_front( (*insts)[ idx ] );
      }
    } else{
      if( idx >= (*inps)[ stackSel - 2 ]->size() )
        return false;
      stk->push_front( (*(*inps)[ stackSel - 2 ])[ idx ] );
    }
    break;  
    // PBII pushbackInputImmediate
  case 17:
    stackSel = ( data >> 12 );
    idx = data & ( ( 1 << 12 ) - 1 );
    if( stackSel >= inps->size() + 2 )
      return false;
    if( stackSel < 2 ){
      if( stackSel )
        stk->push_back( (*rand)[ rp++ ] );
      else{
        if( idx >= insts->size() )
          return false;
        stk->push_back( (*insts)[ idx ] );
      }
    } else{
      if( idx >= (*inps)[ stackSel - 2 ]->size() )
        return false;
      stk->push_back( (*(*inps)[ stackSel - 2 ])[ idx ] );
    }
    break;  
    // PSIN pushInput
    // pushes the value of input[ (*stk)[ data ] ][ (*stk)[ data + 1 ] ]
    // the value of input[ 1 ][ anything ] is non-deterministic
    // if data is 0 it also douple pops before adding the new value.
  case 18:
    if( data + 1 >= stk->size() || (*stk)[ data ] >= ( inps->size() + 2 ) )
      return false;
    stackSel = (*stk)[ data ];
    idx = (*stk)[ data + 1 ];
    if( !data ){
      stk->pop_front();
      stk->pop_front();
    }
    if( stackSel < 2 ){
      if( stackSel )
        stk->push_front( (*rand)[ rp++ ] );
      else{
        if( idx >= insts->size() )
          return false;
        stk->push_front( (*insts)[ idx ] );
      }
    } else{
      if( idx >= (*inps)[ stackSel - 2 ]->size() )
        return false;
      stk->push_front( (*(*inps)[ stackSel - 2 ])[ idx ] );
    }
    break;
    // PBIN pushbackInput
  case 19:
    if( data + 1 >= stk->size() || (*stk)[ data ] >= ( inps->size() + 2 ) )
      return false;
    stackSel = (*stk)[ data ];
    idx = (*stk)[ data + 1 ];
    if( !data ){
      stk->pop_front();
      stk->pop_front();
    }
    if( stackSel < 2 ){
      if( stackSel )
        stk->push_back( (*rand)[ rp++ ] );
      else{
        if( idx >= insts->size() )
          return false;
        stk->push_back( (*insts)[ idx ] );
      }
    } else{
      if( idx >= (*inps)[ stackSel - 2 ]->size() )
        return false;
      stk->push_back( (*(*inps)[ stackSel - 2 ])[ idx ] );
    }
    break;
    // DUPL duplicate, pop data elements off the stack, then copy top element if there is one.
  case 20:
    for( u32 i = 0; stk->size() && i < data; ++i )
      stk->pop_front();
    if( stk->size() )
      stk->push_front( (*stk)[ 0 ] ); 
    break;
    // SWAP swap
    // swaps (*stk)[ 0 ] and (*stk)[ data ]
  case 21:
    if( data >= stk->size() )
      return false;
    {
      u32 t = (*stk)[ 0 ];
      (*stk)[ 0 ] = (*stk)[ data ];
      (*stk)[ data ] = t;
    }
    break;
    // SETR relative set
    // sets (*stk)[ (*stk)[ 0 ] ] to (*stk)[ data ]
  case 22:
    if( data >= stk->size() || (*stk)[ 0 ] >= stk->size() )
      return false;
    (*stk)[ (*stk)[ 0 ] ] = (*stk)[ data ];
    break;
    // POPI pop multiple values immediate
  case 23:    for( u32 i = 0; i < data && stk->size(); ++i )
                stk->pop_front();

    break;
    // PPBI pop multiple values immediate off back
  case 24:
    for( u32 i = 0; i < data && stk->size(); ++i )
      stk->pop_back();
    break;
    // POPR pop multiple values relative
  case 25:
    if( data >= stk->size() )
      return false;
    for( u32 i = 0, c = (*stk)[ data ]; i < c && stk->size(); ++i )
      stk->pop_front();
    break;
    // PPBR pop multiple values relative off back
  case 26:
    if( data >= stk->size() )
      return false;
    for( u32 i = 0, c = (*stk)[ data ]; i < c && stk->size(); ++i )
      stk->pop_back();
    break;
    // JMPP relative jump and pop.  Jumps to stack[ 0 ], also pops data elements off the stack.
  case 27:
    if( !stk->size() )
      break;
    ip = (*stk)[ 0 ];
    for( u32 i = 0; i < data && stk->size(); ++i )
      stk->pop_front();
    return true;
    // JMPR relative jump, if data is 0, pop before jumping
  case 28:
    if( data >= stk->size() )
      return false;
    ip = (*stk)[ data ];
    if( !data )
      stk->pop_front();
    return true;
    // JMPI immediate jump 
  case 29:
    ip = data;
    return true;
    // BREQ braches to data if stack[ 0 ] == stack[ 1 ].  Also pops 2 elements off the stack.
  case 30:
    if( stk->size() < 2 )
      break;
    if( (*stk)[ 0 ] == (*stk)[ 1 ] )
      ip = data;
    else
      ip += 1;
    stk->pop_front(); stk->pop_front();
    return true;
    // BRNE braches to data if stack[ 0 ] != stack[ 1 ].  Also pops 2 elements off the stack.
  case 31:
    if( stk->size() < 2 )
      break;
    if( (*stk)[ 0 ] != (*stk)[ 1 ] )
      ip = data;
    else
      ip += 1;
    stk->pop_front(); stk->pop_front();
    return true;
    // BRGR braches to data if stack[ 0 ] > stack[ 1 ].  Also pops 2 elements off the stack.
  case 32:
    if( stk->size() < 2 )
      break;
    if( (*stk)[ 0 ] > (*stk)[ 1 ] )
      ip = data;
    else
      ip += 1;
    stk->pop_front(); stk->pop_front();
    return true;
    // BRGE braches to data if stack[ 0 ] >= stack[ 1 ].  Also pops 2 elements off the stack.
  case 33:
    if( stk->size() < 2 )
      break;
    if( (*stk)[ 0 ] >= (*stk)[ 1 ] )
      ip = data;
    else
      ip += 1;
    stk->pop_front(); stk->pop_front();
    return true;
    // BRLS braches to data if stack[ 0 ] < stack[ 1 ].  Also pops 2 elements off the stack.
  case 34:
    if( stk->size() < 2 )
      break;
    if( (*stk)[ 0 ] < (*stk)[ 1 ] )
      ip = data;
    else
      ip += 1;
    stk->pop_front(); stk->pop_front();
    return true;
    // BRLE braches to data if stack[ 0 ] <= stack[ 1 ].  Also pops 2 elements off the stack.
  case 35:
    if( stk->size() < 2 )
      break;
    if( (*stk)[ 0 ] <= (*stk)[ 1 ] )
      ip = data;
    else
      ip += 1;
    stk->pop_front(); stk->pop_front();
    return true;
    // LOPI loop immediate, if stack[ 0 ] is 0, pop it, otherwise decrement stack[ 0 ] and branch to data
  case 36:
    if( !stk->size() )
      break;
    if( (*stk)[ 0 ] ){
      ip = data;
      --(*stk)[ 0 ];
      return true;
    } else
      stk->pop_front();
    break;
    // LOPR loop relative, if stack[ 0 ] is 0, pop it, otherwise decrement stack[ 0 ] and branch to stack[ data ]
  case 37:
    if( data >= stk->size() )
      return false;
    if( (*stk)[ 0 ] ){
      ip = (*stk)[ data ];
      --(*stk)[ 0 ];
      return true;
    } else
      stk->pop_front();
    break;
    // CALL push ip onto the back of the stack and branch to data.
  case 38:
    stk->push_back( ip );
    ip = data;
    return true;
    // RETR load the back of the stack + 1 into ip and pop the back of the stack.  Also pop data elements off the front of the stack.
  case 39:
    if( !stk->size() )
      break;
    ip = (*stk)[ stk->size() - 1 ] + 1;
    stk->pop_back();
    for( u32 i = 0; i < data && stk->size(); ++i )
      stk->pop_front();
    return true;
    // ADDI add immediate
  case 40:
    if( !stk->size() )
      break;
    (*stk)[ 0 ] += data;
    break;
    // for all XXXR arathmetic: if data is 1, then remove it, leaving just the result on top.
    // ADDR add relative
  case 41:
    if( data >= stk->size() )
      break;
    if( data == 1 && stk->size() > 1 ){
      (*stk)[ 1 ] = (*stk)[ 0 ] + (*stk)[ 1 ];
      stk->pop_front();
    }else
      (*stk)[ 0 ] += (*stk)[ data ];
    break;
    // SUBI subtract immediate
  case 42:
    if( !stk->size() )
      break;
    (*stk)[ 0 ] -= data;
    break;
    // SUBR subtract relative
  case 43:
    if( data >= stk->size() )
      break;
    if( data == 1 && stk->size() > 1 ){
      (*stk)[ 1 ] = (*stk)[ 0 ] - (*stk)[ 1 ];
      stk->pop_front();
    }else
      (*stk)[ 0 ] -= (*stk)[ data ];
    break;
    // MULI multiply immediate
  case 44:
    if( !stk->size() )
      break;
    (*stk)[ 0 ] *= data;
    break;
    // MULR multiply relative
  case 45:
    if( data >= stk->size() )
      break;
    if( data == 1 && stk->size() > 1 ){
      (*stk)[ 1 ] = (*stk)[ 0 ] * (*stk)[ 1 ];
      stk->pop_front();
    }else
      (*stk)[ 0 ] *= (*stk)[ data ];
    break;
    // DIVI divide immediate
  case 46:
    if( !stk->size() )
      break;
    if( data )
      (*stk)[ 0 ] /= data;
    else
      (*stk)[ 0 ] = 0;
    break;
    // DIVR divide relative
  case 47:
    if( data >= stk->size() )
      break;
    {
      u32 res = 0;
      if( (*stk)[ data ] )
        res = (*stk)[ 0 ] / (*stk)[ data ];
      if( data == 1 && stk->size() > 1 ){
        (*stk)[ 1 ] = res;
        stk->pop_front();
      }else
        (*stk)[ 0 ] = res;
    }
    break;
    // MODI modulus immediate
  case 48:
    if( !stk->size() )
      break;
    if( data )
      (*stk)[ 0 ] %= data;
    else 
      (*stk)[ 0 ] = 0;
    break;
    // MODR modulus relative
  case 49:
    if( data >= stk->size() )
      break;
    {
      u32 res = 0;
      if( (*stk)[ data ] )
        res = (*stk)[ 0 ] % (*stk)[ data ];
      if( data == 1 && stk->size() > 1 ){
        (*stk)[ 1 ] = res;
        stk->pop_front();
      }else
        (*stk)[ 0 ] = res;
    }
    break;
    // ANDI bitwise and immediate
  case 50:
    if( !stk->size() )
      break;
    (*stk)[ 0 ] &= data;
    break;
    // ANDR bitwise and relative
  case 51:
    if( data >= stk->size() )
      break;
    if( data == 1 && stk->size() > 1 ){
      (*stk)[ 1 ] = (*stk)[ 0 ] & (*stk)[ 1 ];
      stk->pop_front();
    }else
      (*stk)[ 0 ] &= (*stk)[ data ];
    break;
    // BORI bitwise or immediate
  case 52:
    if( !stk->size() )
      break;
    (*stk)[ 0 ] |= data;
    break;
    // BORR bitwise or relative
  case 53:
    if( data >= stk->size() )
      break;
    if( data == 1 && stk->size() > 1 ){
      (*stk)[ 1 ] = (*stk)[ 0 ] | (*stk)[ 1 ];
      stk->pop_front();
    }else
      (*stk)[ 0 ] |= (*stk)[ data ];   
    break;
    // XORI bitwise xor immediate
  case 54:
    if( !stk->size() )
      break;
    (*stk)[ 0 ] ^= data;
    break;
    // XORR bitwise xor relative
  case 55:
    if( data >= stk->size() )
      break;
    if( data == 1 && stk->size() > 1 ){
      (*stk)[ 1 ] = (*stk)[ 0 ] ^ (*stk)[ 1 ];
      stk->pop_front();
    }else
      (*stk)[ 0 ] ^= (*stk)[ data ];    
    break;
    // SHLI bitwise shift left immediate
  case 56:
    if( !stk->size() )
      break;
    (*stk)[ 0 ] <<= data;
    break;
    // SHLR bitwise shift left relative
  case 57:
    if( data >= stk->size() )
      break;
    if( data == 1 && stk->size() > 1 ){
      (*stk)[ 1 ] = (*stk)[ 0 ] << (*stk)[ 1 ];
      stk->pop_front();
    }else
      (*stk)[ 0 ] <<= (*stk)[ data ];
    break;
    // SHRI bitwise shift right immediate
  case 58:
    if( !stk->size() )
      break;
    (*stk)[ 0 ] >>= data;
    break;
    // SHRR bitwise shift right relative
  case 59:
    if( data >= stk->size() )
      break;
    if( data == 1 && stk->size() > 1 ){
      (*stk)[ 1 ] = (*stk)[ 0 ] >> (*stk)[ 1 ];
      stk->pop_front();
    }else
      (*stk)[ 0 ] >>= (*stk)[ data ];    
    break;
  default:
    return false;
  }    
  ip += 1;
  return true;
}
string Program::trace( u32 steps, const set< u32 >& breakpoints ){
  ostringstream ans;
  bool stop = false;
  ans << programToString() << "\n\n\n";
  for( u32 i = 0; i < steps; ++i ){
    bool bp = breakpoints.find( ip ) != breakpoints.end();
    if( bp )
      ans << ip << ": " << Program::instructionToString( (*insts)[ ip ] ) << "\n";
    stop = !tick();
    if( bp )
      ans << stackToString() << "\n\n";
    if( stop )
      break;
  }
  if( stop )
    ans << "Finished\n";
  else
    ans << "Not finished...\n";
  return ans.str();
}
void Program::run( ProgramRunInfo& pri, u64 maxic, u32 maxss ){
  pri.instructionCount = 0;
  pri.maxStackSize = 0;
  pri.randomDataConsumed = rp;

  while( pri.maxStackSize < maxss && pri.instructionCount < maxic ){
    if( stk->size() > pri.maxStackSize )
      pri.maxStackSize = stk->size();
    ++pri.instructionCount;
    if( !tick() )
      break;
  }
  pri.randomDataConsumed = rp - pri.randomDataConsumed;
}
string Program::programToString( void ){
  ostringstream ans;
  ans << "Program (ip:" << ip << ")<\n";
  for( u32 i = 0; i < insts->size(); ++i ){
    if( ip == i )
      ans << "* " ;
    else
      ans << "  ";
    ans << setw( 11 ) << i << ": " << Program::instructionToString( (*insts)[ i ] ) << "\n";
  }
  ans << ">\n";
  return ans.str();
}
string Program::stackToString( void ){
  ostringstream ans;
  ans << "Stack<\n";
  for( u32 i = 0; i < stk->size(); ++i )
    ans << setw( 11 ) << i << ": " << setw( 11 ) << (*stk)[ i ] << "  " << Program::instructionToString( (*stk)[ i ] ) << "\n";
  ans << ">\n";
  return ans.str();
}
void Program::renonce( u64 nonce ){
  delete rand;
  rp = 0;
  rand = new AESRand( nonce, 1024 );
}
void Program::reset( void ){
  ip = 0;
  stk->clear();
}
void Program::clear( void ){
  reset();
  insts->clear();
  inps->clear();
}


u32 Program::instructionPointer( void ) const{
  return ip;
}
void Program::addInput( const deque< u32 >& input ){
  inps->push_back( &input );
}

ProgramPool::ProgramPool( const Program& adam, u32 averagePoolSize, f64 cullScalar, u32 slidingAverageCount, u64 nonce ) : aps( averagePoolSize ), rp( 0 ), inps(), progs(), slidingCnt( slidingAverageCount ), cullSclr( cullScalar ), tcs(), mss(){
  rand = new AESRand( nonce, 1024 );
  progs.push_front( new PPMember() );
  progs[ 0 ]->insts = adam.instructions();
  progs[ 0 ]->parent = 0;
  progs[ 0 ]->parentGenerationGap = 0;
  progs[ 0 ]->scoreDenom = 0.0;
  progs[ 0 ]->score = 0.0;
  inps.push_back( &progs[ 0 ]->insts );
}

ProgramPool::~ProgramPool( void ){
  while( progs.size() ){
    delete progs[ 0 ];
    progs.pop_front();
  }
  delete rand;
}
void ProgramPool::populate( void ){
  if( progs.size() > (u32)( aps * cullSclr ) )
    while( progs.size() > (u32)( aps / cullSclr ) )
      cull();
  
  u64 maxTickCount = 0;
  u32 maxStackSize = 0;
  if( tcs.size() ){
    for( u32 i = 0; i < tcs.size(); ++i ){
      maxTickCount += tcs[ i ] * 2;
      maxStackSize += mss[ i ] * 2;
    }
    maxTickCount /= tcs.size();
    maxStackSize /= tcs.size();
  } else
    maxTickCount = maxStackSize = (u32)-1;

  if( maxTickCount < ADAM_MIN_TICKS )
    maxTickCount = ADAM_MIN_TICKS; 
  if( maxStackSize < ADAM_MIN_MAX_STACK_SIZE )
    maxStackSize = ADAM_MIN_MAX_STACK_SIZE;

  u32 whch = (*rand)[ rp++ ] % progs.size();
  //{
  //  u32 lowestRunCount = (u32)-1;
  //  deque< u32 > indcs;

  //  for( u32 i = 0; i < progs.size(); ++i ){
  //    if( progs[ i ]->scores.size() < lowestRunCount ){
  //      lowestRunCount = progs[ i ]->scores.size();
  //      indcs.clear();
  //    }
  //    if( progs[ i ]->scores.size() == lowestRunCount ||  progs[ i ]->scores.size() == lowestRunCount )
  //      indcs.push_front( i );
  //  }

  //  whch = indcs[ (*rand)[ rp++ ] % indcs.size() ];
  //}

  {
    f64 sc, br;

    progs.push_back( new PPMember() );
    progs.back()->parent = progs[ whch ];
    progs.back()->parentGenerationGap = 1;
    progs.back()->score = 0.0;
    progs.back()->scoreDenom = 0.0;

    {
      ProgramRunInfo pri;
      //BUGBUG crashes when rp wraps to zero?
      Program prg( progs[ whch ]->insts, inps, progs.back()->insts, ( ( (u64)(*rand)[ rp ] ) << 32 ) + ( (u64)(*rand)[ rp + 1 ] ) );
      rp += 2;
      
      breedPrep();
      prg.run( pri, maxTickCount, maxStackSize );
      while( tcs.size() >= slidingCnt ){ tcs.pop_front(); mss.pop_front(); } tcs.push_back( pri.instructionCount ); mss.push_back( pri.maxStackSize );
      br = breed( pri, prg.instructions(), prg.stack() );

      deque< u32 > stk;
      prg.setStack( stk );
      prg.reset();
      scorePrep();
      prg.run( pri, maxTickCount, maxStackSize );
      while( tcs.size() >= slidingCnt ){ tcs.pop_front(); mss.pop_front(); } tcs.push_back( pri.instructionCount ); mss.push_back( pri.maxStackSize );
      sc = score( pri, prg.stack(), progs.back()->insts );
    
    
      // Run start tests to esablish score.
      deque< u32 > breedStk;
      for( u32 i = 0; i < START_TEST_COUNT; ++i ){
        f64 ssc = 0.0;
        f64 sbr = 0.0;
        prg.setStack( breedStk );
        prg.reset();
        breedPrep();
        prg.run( pri, maxTickCount, maxStackSize );
        sbr = breed( pri, prg.instructions(), prg.stack() );
        while( tcs.size() >= slidingCnt ){ tcs.pop_front(); mss.pop_front(); } tcs.push_back( pri.instructionCount ); mss.push_back( pri.maxStackSize );
        prg.setStack( stk );
        prg.reset();
        scorePrep();
        prg.run( pri, maxTickCount, maxStackSize );
        ssc = score( pri, prg.stack(), breedStk );
        while( tcs.size() >= slidingCnt ){ tcs.pop_front(); mss.pop_front(); } tcs.push_back( pri.instructionCount ); mss.push_back( pri.maxStackSize );
        u32 g = 0;
        for( PPMember* c = progs.back(); c; g += c->parentGenerationGap, c = c->parent ){
          c->score += scaleScore( g ) * ( sbr + ssc );
          c->scoreDenom += scaleScore( g );
        }
      }
    

    }



    u32 g = 0;
    for( PPMember* c = progs[ whch ]; c; g += c->parentGenerationGap, c = c->parent ){
      //BUGBUG
      // TODO: make it run new prog once and discard output so all progs have scores.
      // TODO: make it compare breed and test and return 0.0 if the stacks are equal.
      c->score += scaleScore( g ) * ( br + sc );
      c->scoreDenom += scaleScore( g );
    }

    inps.push_back( &progs.back()->insts );
  }
}
string ProgramPool::programPoolToString( void ){
  ostringstream ans;
  map< PPMember*, u32 > indcs;
  for( u32 i = 0; i < progs.size(); ++i )
    indcs.insert( pair< PPMember*, u32 >( progs[ i ], i ) );

  ans << "ProgramPool (avg. ticks " << averageTicks() << ", avg. max stack size " << averageMaxStackSize() << ", rp " << rp << ")<\n";
  for( u32 i = 0; i < progs.size(); ++i ){
    ans << "  Program " << i << " <size " << progs[ i ]->insts.size() << ", parent ";
    if( progs[ i ]->parent )
      ans << indcs[ progs[ i ]->parent ] << ", gen gap ";
    else
      ans << "none, gen gap ";
    ans << progs[ i ]->parentGenerationGap;
    ans << ", avg score " << ( progs[ i ]->scoreDenom ? progs[ i ]->score / progs[ i ]->scoreDenom : 0.0 );
    ans << ", runs " << progs[ i ]->scoreDenom;
    ans << "<\n";
  }
  ans << ">\n";
  return ans.str();
}

string ProgramPool::highestScore( void ){
  ostringstream sans;
  if( !progs.size() )
    return "";
  f64 ans = -1.0; 
  u32 idx = 0;
  for( u32 i = 0; i < progs.size(); ++i ){
    f64 cs = ( progs[ i ]->scoreDenom ? progs[ i ]->score / progs[ i ]->scoreDenom : 0.0 ); 
    if( cs > ans ) {
      idx = i;
      ans = cs; 
    }
  }
  sans << "highest score< index: " << idx << ", score: " << ans <<", runs: " << progs[ idx ]->scoreDenom << " >";
  return sans.str(); 
}
void ProgramPool::breedPrep( void ){
  deque< u32 >* ni = new deque< u32 >();
  ni->push_front( 0 );
  inps.push_front( ni );
}
f64 ProgramPool::breed( const ProgramRunInfo& pri, const deque< u32 >& prog, const deque< u32 >& newProg ){
  delete inps[ 0 ];
  inps.pop_front();
  f64 scr = 0;

  u32 ld = abs( (int)( (int)prog.size() - (int)newProg.size() ) );
  if( prog == newProg || pri.instructionCount < 100 || prog.size() < 50 || newProg.size() < 50 )
    return 0;

  u32 exptrt = (u32)( prog.size() * 0.15 );
  if( !exptrt )
    exptrt = 1;

  u32 lds = ld > exptrt ? ld - exptrt : exptrt - ld;
  if( lds < exptrt )
    scr += ( (f64)( exptrt - lds ) ) * ( (f64)( exptrt - lds ) );
  exptrt /= 3;
  if( !exptrt )
    exptrt = 1;
  ld = pri.randomDataConsumed;
  lds = ld > exptrt ? ld - exptrt : exptrt - ld;
  if( lds < exptrt )
    scr += ( (f64)( exptrt - lds ) ) * ( (f64)( exptrt - lds ) );

  return ADAM_BREED_SCALAR * scr / ( ( (f64)pri.instructionCount ) + ( (f64)pri.maxStackSize ) );
}

void ProgramPool::cull( void ){
  if( !progs.size() )
    return;
  f64 ls = -1.0;
  u32 tr = 1;
  for( u32 i = 1; i < progs.size(); ++i ){
    f64 cs;
    if( progs[ i ]->scoreDenom != 0.0 ) 
      cs = progs[ i ]->score / progs[ i ]->scoreDenom;
    else
      cs = 0;
    if( ls == -1.0 || cs < ls ){
      tr = i;
      ls = cs;
    }
  }
  
  PPMember* trp = progs[ tr ];
  for( u32 i = 0; i < progs.size() - 1; ++i ){
    PPMember* nc = ( i < tr ? progs[ i ] : progs[ i + 1 ] );
    if( nc->parent == trp ){
      nc->parent = trp->parent;
      if( trp->parent )
        nc->parentGenerationGap += trp->parentGenerationGap;
      else
        nc->parentGenerationGap = 0;
    }
    progs[ i ] = nc;
    if( i >= tr )
      inps[ i ] = inps[ i + 1 ];
  }

  inps.pop_back();
  progs.pop_back();
  delete trp;
}

FibProgramPool::FibProgramPool( const Program& adam, u32 averagePoolSize, f64 cullScalar, u32 slidingAverageCount, u64 nonce ) : ProgramPool( adam, averagePoolSize, cullScalar, slidingAverageCount, nonce ){}
FibProgramPool::~FibProgramPool( void ){}
void FibProgramPool::scorePrep( void ){
  deque< u32 >* ni = new deque< u32 >();
  ni->push_front( 1 );
  inps.push_front( ni );
}
f64 FibProgramPool::score( const ProgramRunInfo& pri, const deque< u32 >& stack, const std::deque< u32 >& newProg ){
  delete inps[ 0 ];
  inps.pop_front();
  
  if( stack.size() > 1024 || !pri.instructionCount || !pri.maxStackSize || abs( (int)( (int)stack.size() - (int)newProg.size() ) ) < 20 )
    return 0.0;
  f64 scr = 0.0;
  {
    u32 c = 0;
    u32 n = 1;
    u32 t;
    for( u32 i = 0; i < stack.size(); ++i, t = n, n += c, c = t ){
      u32 df = stack[ i ] < c ? c - stack[ i ] : stack[ i ] - c;
      if( df < 256 )
        scr += pow( (f64)( 256 - df ), 2.0 )  / ( 256.0 * 256.0 );
      else
        scr -= pow( (f64)( df - 256 ), 2.0 ) / ( 256.0 * 256.0 );
    }
  }

  if( scr > 0.0 )
    return ( scr * 10000000000.0 ) / ( ( (f64)pri.instructionCount ) + ( (f64)pri.maxStackSize ) );
  else
    return 0.0;
}

