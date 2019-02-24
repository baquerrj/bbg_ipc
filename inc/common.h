#ifndef COMMON_H
#define COMMON_H

#include <pthread.h>
#include <stdio.h>
#include <sys/syscall.h>

void fifo_exit(FILE *log, int fifo_id, int signo);

extern pthread_mutex_t mutex;

#endif /* COMMON_H */
