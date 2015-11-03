#ifndef _MM_H
#define _MM_H

int   mem_init(size_t);
void  mem_nuke(void);

void* malloc(size_t);
void* calloc(size_t, size_t);
void* realloc(void*, size_t);
void  free(void*);

#endif
