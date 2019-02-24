#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
/* /sys includes */
#include <sys/types.h>
#include <sys/stat.h>

/* Local Inlcudes */
#include "common.h"

#define MESSAGE_SIZE 50
#define HEADER_SIZE  30

#define FIFO_SIZE    MESSAGE_SIZE + HEADER_SIZE

static FILE *log;
static int fifo_id;
static struct timespec thread_time;

static void fifo_exit(void)
{
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
   log = fopen("second_pid.log", "w");

   fprintf(log, "PID %d - Named FIFO:\n",
         getpid());
   
   int fid = open(path_to_fifo, O_RDWR);
   fprintf(log, "Path to FIFO: %s File Descriptor: %d\n",
         path_to_fifo, fid);

   /* Header for message */
   char header[HEADER_SIZE];
   sprintf(header, "From PID %d - Length %d\n",
         getpid(), FIFO_SIZE);

   /* Buffer for message */
   char buffer[MESSAGE_SIZE];

   /* Buffer for entired message to write to pipe:
    * header + buffer */
   char fifo[FIFO_SIZE];   
   while( 1 )
   {
      read(fid, fifo, sizeof(fifo));
      clock_gettime(CLOCK_REALTIME, &thread_time);
      fprintf(log, "%ld s - %ld nsec - Received: %s",
            thread_time.tv_sec, thread_time.tv_nsec, fifo);

      /* Write to buffer */
      sprintf(buffer, "Hello Friend!\n");

      /* Complete message to write to pipe */
      sprintf(fifo, "%s %s", header, buffer);

      /* Write buffer to pipe */
      write(fid, fifo, strlen(fifo)+1);
      clock_gettime(CLOCK_REALTIME, &thread_time);
      
      fprintf(log, "%ld s - %ld nsec - Sending: %s",
            thread_time.tv_sec, thread_time.tv_nsec, fifo);   
   }
   return 0;
}
