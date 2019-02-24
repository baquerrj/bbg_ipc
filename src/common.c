#include "common.h"
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <signal.h>

void fifo_exit(FILE *log, int fifo_id, int signo)
{
   struct timespec thread_time;
   clock_gettime(CLOCK_REALTIME, &thread_time);

   /* Close FIFO */
   close( fifo_id );

   switch( signo )
   {
      case SIGINT:
         fprintf(stdout, "\nPID [%d] (%ld s - %ld nsec)\nCaught SIGINT signal!\n",
               (pid_t)syscall(SYS_gettid), thread_time.tv_sec, thread_time.tv_nsec);
         fprintf(log, "PID [%d] (%ld s - %ld nsec)\nCaught SIGINT signal!\n",
               (pid_t)syscall(SYS_gettid), thread_time.tv_sec, thread_time.tv_nsec);
         break;
      default:
         fprintf(stdout, "\nPID [%d] (%ld s - %ld nsec)\nExiting!\n",
               (pid_t)syscall(SYS_gettid), thread_time.tv_sec, thread_time.tv_nsec);
         fprintf(log, "PID [%d] (%ld s - %ld nsec)\nExiting!\n",
               (pid_t)syscall(SYS_gettid), thread_time.tv_sec, thread_time.tv_nsec);
         break;
   }
   fclose( log );
   exit(0);
}
