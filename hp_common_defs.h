#ifndef __hp_common_defs_h__
#define __hp_common_defs_h__

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <stdint.h>
#include <errno.h>

#define DEBUG 1
#define printd(...) if(DEBUG) printf(__VA_ARGS__)

#define ERROR 1
#define printe(...) if(DEBUG) printf(__VA_ARGS__)

#define handle_error(msg) \
   do { perror(msg); return 0; } while (0)

#define handle_error_exit(msg) \
   do { perror(msg); _exit(1); } while (0)


const unsigned int MAX_RETRIES = 100;
const unsigned int RETRY_INTERVAL_SEC = 1;
#endif
