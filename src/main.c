#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "fifo_one.h"
#include "fifo_two.h"
#include "common.h"

static pthread_t t1;
static pthread_t t2;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

#if 0
static void print_header(FILE *file)
{
   struct timespec time;
   clock_gettime(CLOCK_REALTIME, &time);
   fprintf( file, "=====================================================\n" );
   fprintf( file, "Master Thread [%d]: %ld s - %ld ns\n",
            (pid_t)syscall(SYS_gettid), time.tv_sec, time.tv_nsec );
   return;
}
#endif

int main( int argc, char *argv[] )
{
   /* Attempting to spawn child threads */
   pthread_create(&t1, NULL, fifo_one, NULL);
   pthread_create(&t2, NULL, fifo_two, NULL);
   pthread_join(t1, NULL);
   pthread_join(t2, NULL);

   return 0;
}
