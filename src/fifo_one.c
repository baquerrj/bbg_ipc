#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
/* /sys includes */
#include <sys/types.h>
#include <sys/stat.h>

/* Local Inlcudes */
#include "common.h"

static FILE     *log;
static int      fifo_id;
static packet_t *fifo;

static struct timespec thread_time;

static void fifo_exit(void)
{
   clock_gettime(CLOCK_REALTIME, &thread_time);
   /* Close FIFO */
   close( fifo_id );

   fprintf(stdout, "\nPID [%d] ( %ld.%ld secs )\nCaught SIGINT signal!\n",
         getpid(), thread_time.tv_sec, thread_time.tv_nsec);
   fprintf(log, "\nPID [%d] ( %ld.%ld secs )\nCaught SIGINT signal!\n",
         getpid(), thread_time.tv_sec, thread_time.tv_nsec);

   fclose( log );
   exit(0);
}



void sig_handler(int signo)
{
   if( signo == SIGINT )
   {
      fifo_exit();
   }
   else
   {
      fprintf(stdout, "PID %d caught unknown signal!\n",
            getpid());
      fprintf(log, "PID %d caught unknown signal!\n",
            getpid());
   }
   return;
}

int main(void)
{
   /* Set up signal handling */
   signal(SIGINT, sig_handler);

   /* FIFO File Path */
   char *path_to_fifo = "/tmp/fifo";

   /* Create FIFO */
   mkfifo(path_to_fifo, 0666);

   /* Log file */
   log = fopen("first_pid.log", "w");

   fprintf(log, "PID %d - Named FIFO:\n",
         getpid());

   int fifo_id = open(path_to_fifo, O_RDWR);
   fprintf(log, "Path to FIFO: %s - File Descriptor: %d\n",
         path_to_fifo, fifo_id);

   /* Allocate memory for packet struct */
   fifo = malloc( sizeof(packet_t) );
   if( NULL == fifo )
   {
      fprintf( stderr, "Encountered error allocating memory for packet struct!\n" );
      return 1;
   }

   /* Header for message */
   sprintf(fifo->header, "From PID %d - Length %d\n",
         getpid(), FIFO_SIZE);

   /* Set message type as MESSAGE_PRINT for now */
   fifo->type = MESSAGE_PRINT;

   while( 1 )
   {
      /* Write to buffer */
      sprintf(fifo->body, "Hello Friend!\n");

      /* Write full message to pipe */
      write(fifo_id, fifo, sizeof(fifo));

      clock_gettime(CLOCK_REALTIME, &thread_time);
      fprintf(log, "( %ld. %ld secs ) - Sending: %s",
            thread_time.tv_sec, thread_time.tv_nsec, fifo->body);

      /* Read from FIFO */
      read(fifo_id, fifo, sizeof(fifo));

      clock_gettime(CLOCK_REALTIME, &thread_time);
      fprintf(log, "( %ld. %ld secs ) - Received: %s",
            thread_time.tv_sec, thread_time.tv_nsec, fifo->body);
   }
   return 0;
}
