/*****************************************************************
 * outline.c
 *
 * Copyright 1999, Clark Cooper
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the license contained in the
 * COPYING file that comes with the expat distribution.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Read an XML document from standard input and print an element
 * outline on standard output.
 * Must be used with Expat compiled for UTF-8 output.
 */


#include <stdio.h>
#include <expat.h>
#include <windows.h>

#define BUFFSIZE        8192

static char Buff[BUFFSIZE];

int Depth;
int mc = 0;
XML_Memory_Handling_Suite mh;
HANDLE heap;



void* vlMalloc( size_t sz ){ 
  ++mc;
  return HeapAlloc( heap, 0, sz );
}
void vlFree( void* d ){ 
  if( d != NULL )--mc;
  HeapFree( heap, 0, d );
}
void* vlRealloc( void* d, size_t sz ){ 
  if( d == NULL )
    return vlMalloc( sz );
  if( !sz ){
    vlFree( d );
    return NULL;
  }
  return HeapReAlloc( heap, 0, d, sz );
}




static void start( void *data, const char *el, const char **attr ){
  int i;

  for( i = 0; i < Depth; i++ ) printf("  ");
  printf("<%s", el);

  for (i = 0; attr[i]; i += 2) {
    printf(" %s='%s'", attr[i], attr[i + 1]);
  }
  printf(">\n");
  Depth++;
}

static void end( void *data, const char *el ){
  int i;
  printf( "\n" );
  for( i = 0; i < Depth; i++ ) printf("  ");
  printf("</%s>\n", el);
  Depth--;
}

static void chars( void *data, const char *txt, int sz ){
  char* buf = vlMalloc( sz + 1 );
  memcpy( buf, txt, sz );
  buf[ sz ] = '\0';
  printf( "%s", buf );
  vlFree( buf );
}
static void ents( void *data, const char *ent, int ispar ){
  printf( "&%s;", ent );
}


int main(int argc, char *argv[])
{
  XML_Parser p;
  mh.malloc_fcn = vlMalloc;
mh.free_fcn = vlFree;
mh.realloc_fcn = vlRealloc;

  heap = HeapCreate( HEAP_NO_SERIALIZE | HEAP_GENERATE_EXCEPTIONS, 0, 0 );
  p = XML_ParserCreate_MM(NULL,&mh,NULL);
  if (! p) {
    fprintf(stderr, "Couldn't allocate memory for parser\n");
    exit(-1);
  }

  XML_SetElementHandler( p, start, end );
  XML_SetCharacterDataHandler( p, chars );
  XML_SetSkippedEntityHandler( p, ents );
  for (;;) {
    int done;
    int len;

    len = (int)fread(Buff, 1, BUFFSIZE, stdin);
    if (ferror(stdin)) {
      fprintf(stderr, "Read error\n");
      exit(-1);
    }
    done = feof(stdin);

    if (XML_Parse(p, Buff, len, done) == XML_STATUS_ERROR) {
      fprintf(stderr, "Parse error at line %u:\n%s\n",
              XML_GetCurrentLineNumber(p),
              XML_ErrorString(XML_GetErrorCode(p)));
      exit(-1);
    }

    if (done)
      break;
  }
  XML_ParserFree(p);
  printf( "\n\n\n\nMemcount: %i", mc );
  HeapDestroy( heap );
  return 0;
}
