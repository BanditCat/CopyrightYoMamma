
typedef struct{
  GLuint* shaders;
  u32* usages;
  u32* names;
  u32 size; // read only
//private
  u32 bufsize;
} shaders;


void initShaders( shaders* s );
// This should only be called on program exit.  The names and usage strings are not released until program exit.
void destroyShaders( shaders* s );
void addShaderOrDie( shaders* s, const u8* src, u32 srcsz, const u8* name, int type );
void addShaderFromResourceOrDie( shaders* s, u32 rsc, const u8* name, int type );
void addShaderFromArchiveOrDie( shaders* s, varchive* varc, const u8* name, int type );
GLuint getShaderByName( shaders* s, const u8* name );

typedef struct{
  GLuint* programs;
  u32* usages;
  u32* names;
  u32 size; // read only
// private
  u32 bufsize;
} programs;


void initPrograms( programs* p );
// This should only be called on program exit.  The names and usage strings are not released until program exit.
void destroyPrograms( programs* p );
void createProgramOrDie( programs* p, shaders* s, u32 cnt, const u8** shds, const u8* name );
GLuint getProgramByName( programs* p, const u8* name );
void useProgramByName( programs* p, const u8* name );
void useProgramByIndex( programs* p, u32 ind );