#ifndef _MM_H
#define _MM_H
#include <stddef.h>

int   mem_init(size_t);
void  mem_nuke(void);

#ifdef DEBUG
#include <stdio.h>
void  mem_dump(FILE*);
#endif

void* malloc(size_t);
void* calloc(size_t, size_t);
void* realloc(void*, size_t);
void  free(void*);

#endif
