#ifndef _SPINLOCK_OSX_H
#define _SPINLOCK_OSX_H
#ifdef __APPLE__
typedef int pthread_spinlock_t;

#ifndef PTHREAD_PROCESS_SHARED
#define PTHREAD_PROCESS_SHARED 1
#endif

#ifndef PTHREAD_PROCESS_PRIVATE
#define PTHREAD_PROCESS_PRIVATE 2
#endif

int pthread_spin_init(pthread_spinlock_t*, int);
int pthread_spin_destroy(pthread_spinlock_t*);
int pthread_spin_lock(pthread_spinlock_t*);
int pthread_spin_trylock(pthread_spinlock_t*);
int pthread_spin_unlock(pthread_spinlock_t*);
#endif
#endif
