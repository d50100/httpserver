#ifndef __i_utils_h__
#define __i_utils_h__

#include <fcntl.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include "dbg.h"

void itimeofday(long* sec, long* usec);

int64_t iclock64(void);

uint32_t iclock();

uint32_t generate_conv();


#endif  // __i_utils_h__