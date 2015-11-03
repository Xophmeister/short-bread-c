#ifndef _OPTIONS_H
#define _OPTIONS_H

typedef struct _options {
  size_t      threads;

  const char* dict_path;

  double      markov_threshold;

  size_t      word_length;
  const char* word_source;
  const char* word_target;

#ifdef DEBUG
  int         log;
#endif
} opt_t;

void usage(char*);
int  opt_parse(opt_t*, int, char**);

#endif
