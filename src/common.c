#include "common.h"
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>

void fifo_exit(FILE *log, int fifo_id)
{
   struct timespec thread_time;
   clock_gettime(CLOCK_REALTIME, &thread_time);

   /* Close FIFO */
   close( fifo_id );
     
   fprintf(stdout, "\nPID [%d] = %ld s - %ld nsec\nCaught SIGINT signal!\n",
         getpid(), thread_time.tv_sec, thread_time.tv_nsec);
   fprintf(log, "PID [%d] = %ld s - %ld nsec\nCaught SIGINT signal!\n",
         getpid(), thread_time.tv_sec, thread_time.tv_nsec);

   fclose( log );
   exit(0);
}
