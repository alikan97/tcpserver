#ifndef __GLOBALS_H__
#define __GLOBALS_H__

#include "types.h"

#define MAX_CLIENTS 10 
#define MAX_MESSAGE_LEN 1024
#define CHANNELS_LEN 64
#define BACKLOG 1024 // Total messages capacity per channel
#define BUFFER_LEN 156

sharedMemory_t *globalStorage;

#endif /*__GLOBALS_H__*/