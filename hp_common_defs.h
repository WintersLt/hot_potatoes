#ifndef __hp_common_defs_h__
#define __hp_common_defs_h__

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <stdint.h>
#include <errno.h>

#define DEBUG 0
#define printd(...) if(DEBUG) printf(__VA_ARGS__)

#define ERROR 0
#define printe(...) if(DEBUG) printf(__VA_ARGS__)

#define handle_error(msg) \
   do { perror(msg); return 0; } while (0)

#define handle_error_exit(msg) \
   do { perror(msg); _exit(1); } while (0)

enum MasterCommands_e
{
	CMD_INVALID = 0,
	CMD_POTATO,
	CMD_TERMINATE
};

const unsigned int MAX_RETRIES = 100;
const unsigned int RETRY_INTERVAL_SEC = 1;
#endif
