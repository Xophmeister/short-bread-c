#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include "options.h"

/* States for state machine */
enum {
  DECIDE,
  READ_NOW,
  READ_NEXT,

  TYPE_FLAG,
  TYPE_INTEGER,
  TYPE_DOUBLE,
  TYPE_STRING
};

void usage(char* bin) {
  (void)fprintf(stderr,
    "Usage: %s [OPTIONS] SOURCE TARGET\n"
    "\n"
    "Options:\n"
    "  -n THREADS     Size of thread pool (defaults to cores minus one)\n"
    "  -d DICTIONARY  Path to dictionary (defaults to /usr/share/dict/words)\n"
    "  -m THRESHOLD   Markov model threshold in [0, 1] (defaults to 0.9)\n"
#ifdef DEBUG
    "  --log          Print logging information\n"
#endif
    "  SOURCE         Source word\n"
    "  TARGET         Target word\n"
    "\n"
    "Note that the source and target words must be of the same length.\n",
  bin);
}

int opt_parse(opt_t* options, int argc, char** argv) {
  int status = 0;

  /* Set defaults */
  options->threads = (size_t)(sysconf(_SC_NPROCESSORS_ONLN) - 1);
  if (options->threads == 0) { options->threads = 1; }
  options->dict_path = "/usr/share/dict/words";
  options->markov_threshold = 0.9;
#ifdef DEBUG
  options->log = 0;
#endif

  if (argc >= 3) {
    int    state = DECIDE;
    int    type;
    void** element;

    for (int i = 1; i < argc; ++i) {
      char** option = argv + i;

      /* Determine read state */
      if (state == DECIDE) {
        switch (argc - i) {
          case 1: /* Last element */
            state   = READ_NOW;
            type    = TYPE_STRING;
            element = (void**)&(options->word_target);
            break;

          case 2: /* Penultimate element */
            state   = READ_NOW;
            type    = TYPE_STRING;
            element = (void**)&(options->word_source);
            break;

          default:
            /* Thread count flag */
            if (strcmp(*option, "-n") == 0) {
              state   = READ_NEXT;
              type    = TYPE_INTEGER;
              element = (void**)&(options->threads);
            }

            /* Markov model threshold flag */
            if (strcmp(*option, "-m") == 0) {
              state   = READ_NEXT;
              type    = TYPE_DOUBLE;
              element = (void**)&(options->markov_threshold);
            }

            /* Dictionary flag */
            if (strcmp(*option, "-d") == 0) {
              state   = READ_NEXT;
              type    = TYPE_STRING;
              element = (void**)&(options->dict_path);
            }

#ifdef DEBUG
            /* Logging flag */
            if (strcmp(*option, "--log") == 0) {
              state   = READ_NOW;
              type    = TYPE_FLAG;
              element = (void**)&(options->log);
            }
#endif

            break;
        }
      }
      
      /* Do something about it */
      switch (state) {
        case READ_NOW:
          switch (type) {
            case TYPE_STRING:
              *element = *option;
              break;

            case TYPE_INTEGER:
              *(size_t*)element = (size_t)strtoul(*option, NULL, 10);
              break;

            case TYPE_DOUBLE:
              *(double*)element = (double)strtod(*option, NULL);
              break;

            case TYPE_FLAG:
              *(int*)element = 1;
              break;

            default:
              break;
          }
          state = DECIDE;
          break;

        case READ_NEXT:
          state = READ_NOW;
          break;

        default:
          break;
      }
    }
  }

  /* Sanity check */
  if (options->threads          == 0    ||
      options->markov_threshold  < 0    ||
      options->markov_threshold  > 1    ||
      options->dict_path        == NULL ||
      options->word_source      == NULL ||
      options->word_target      == NULL ||
      (options->word_length = strlen(options->word_source)) != strlen(options->word_target)
  ) {
    status = -1;
  }

  return status;
}
