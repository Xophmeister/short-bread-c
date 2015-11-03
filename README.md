# Premature Optimisation Playground

Short of writing a unikernel to solve the problem, we do a lot of the
management work ourselves in a (potentially misguided) effort to eke the
most performance out of our implementation.

* Managed memory allocation: Rather than constantly calling out to
  `malloc` and friends, we allocate a big chunk of memory at start up
  and do our own management (just with a simple pointer to the next
  chunk, with no freeing). This is clearly wasteful, but we are not
  optimising for space.

* Thread pooling: Everything that can be distributed, will be
  distributed!
  * TODO: Analyse overhead against parallelisation gains (n.b., with the
    above shared memory, threads are just used for compute so shouldn't
    need much memory of their own)
  * TODO: Lock free?
  * TODO: How to signal workers (spin lock/signal/semaphore...)

* Memory mapping: We don't `read` the dictionary, we `mmap` it, with
  multiple workers scanning over it to feed it into the dictionary hash
  (hash keys will be pointers to the map, rather than `memcpy`'d).
  (NOTE: Will serial IO limit the workers, or will the OS do magic
  things? The dictionary is relatively small, so it could be loaded into
  a couple of pages...)

* Markov model: To reduce the search space when finding potential words
  with one letter difference, we create a Markov model from the
  dictionary as we read it in. Then we can generate all the potential
  nodes above some threshold (e.g., "short" will never generate "xhort"
  and it's unlikely to generate "shorg")
