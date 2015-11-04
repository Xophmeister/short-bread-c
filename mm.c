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
typedef void*(*malloc_t)(size_t);
typedef void*(*realloc_t)(void*, size_t);
typedef void*(*free_t)(void*);

static malloc_t  _malloc;
static realloc_t _realloc;
static free_t    _free;

int mem_init(size_t bytes) {
  int ret = 0;

  /* Assign libc functions */
  _malloc  = (malloc_t)dlsym(RTLD_NEXT, "malloc");
  _realloc = (realloc_t)dlsym(RTLD_NEXT, "realloc");
  _free    = (free_t)dlsym(RTLD_NEXT, "free");

  /* Allocate memory on the heap */
  _mem = _malloc(bytes);

  if (_mem) {
    _size = bytes;
    _top  = 0;
  
    /* Initialise the spin lock */
    (void)pthread_spin_init(&_lock, PTHREAD_PROCESS_SHARED);
  } else {
    ret = -1;
  }

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
    /* Resize the heap: Accommodate overflow and double */
    size_t new_size = (_top + bytes) * 2;
    void* old_mem = _mem;
    _mem = _realloc(_mem, new_size);
    if (_mem == old_mem) {
      _size = new_size;
      (void)pthread_spin_unlock(&_lock);
      return malloc(bytes);
    } else {
      /* Fail if realloc moves memory around or fails itself */
      _size = 0;
      _top  = 0;
      (void)pthread_spin_unlock(&_lock);
      return NULL;
    }
  }
}

void* calloc(size_t n, size_t each) {
  size_t bytes = n * each;
  void* mem = malloc(bytes);
  if (mem) {
    return memset(mem, 0, bytes);
  } else {
    return NULL;
  }
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
