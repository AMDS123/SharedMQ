#ifndef __ERRORS_HEADER__
#define __ERRORS_HEADER__

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdarg.h>
#include <string.h>

#define QUEUE_ERR -1000
#define QUEUE_ERR_CHECKHT  (QUEUE_ERR - 1) //head, tail mess up
#define QUEUE_ERR_FULL     (QUEUE_ERR - 2) //full, can't enqueue more
#define QUEUE_ERR_EMPTY    (QUEUE_ERR - 3) //empty, can't dequeue more
#define QUEUE_ERR_CHECKSEN (QUEUE_ERR - 4) //sentinel check wrong.
#define QUEUE_ERR_MEMESS   (QUEUE_ERR - 5) //mem is fuck up
#define QUEUE_ERR_OTFBUFF  (QUEUE_ERR - 6) //user buffer overflow

#define SYS_ERR -2000
#define NODATA_ERR (SYS_ERR - 1)
#define ARG_ERR (SYS_ERR - 2)

#define TELL_ERROR(format, ...) fprintf(stderr, "ERROR: " format "\n", ## __VA_ARGS__)

#define TELL_SYS_ERROR fprintf(stderr, "ERROR: %s\n", strerror(errno))

extern void exit_if(int condition, const char *format, ...);

#endif