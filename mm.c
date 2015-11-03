#include <errno.h>
#include <dlfcn.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include "mm.h"
#include "spinlock-osx.h"

static void*              _mem;
static size_t             _size;
static size_t             _top;
static pthread_spinlock_t _lock;

/* libc memory management function types and pointers thereto */
typedef void*(*calloc_t)(size_t,size_t);
typedef void*(*realloc_t)(void*, size_t);
typedef void*(*free_t)(void*);

static calloc_t  _calloc;
static realloc_t _realloc;
static free_t    _free;

int mem_init(size_t bytes) {
  int ret = 0;

  /* Assign libc functions */
  _calloc  = (calloc_t)dlsym(RTLD_NEXT, "calloc");
  _realloc = (realloc_t)dlsym(RTLD_NEXT, "realloc");
  _free    = (free_t)dlsym(RTLD_NEXT, "free");

  /* Allocate memory on the heap, all zero'd */
  _mem = _calloc(bytes, 1);
  if (_mem) {
    _size = bytes;
    _top  = 0;
  } else {
    ret = -1;
  }

  /* Initialise the spin lock */
  (void)pthread_spin_init(&_lock, PTHREAD_PROCESS_SHARED);

  return ret;
}

void mem_nuke(void) {
  _free(_mem);
  _size = 0;
  _top  = 0;
  (void)pthread_spin_destroy(&_lock);
}

void* malloc(size_t bytes) {
  (void)pthread_spin_lock(&_lock);
  if (_top + bytes <= _size) {
    /* We've still got space on our heap */
    void* ptr = (char *)_mem + _top;
    _top += bytes;
    (void)pthread_spin_unlock(&_lock);
    return ptr;
  } else {
    /* Double the heap */
    _mem = _realloc(_mem, _size * 2);
    if (_mem) {
      _size *= 2;
      (void)pthread_spin_unlock(&_lock);
      return malloc(bytes);
    } else {
      _size = 0;
      _top  = 0;
      (void)pthread_spin_unlock(&_lock);
      return NULL;
    }
  }
}

void* calloc(size_t n, size_t bytes) {
  return malloc(n * bytes);
}

void* realloc(void* p, size_t bytes) {
  void* new = malloc(bytes);
  if (new) {
    return memcpy(new, p, bytes);
  } else {
    return NULL;
  }
}

void free(void* p) {
  /* Do nothing */
  (void)p;
}
