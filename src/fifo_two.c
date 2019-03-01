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

/* File Descriptors for named pipes:
 * fin is the input FIFO for process
 * fout is output FIFO for process */
static int      fin;
static int      fout;

/* Log File for process */
static FILE     *log;

/* Structs for messages:
 * msg_in for the incoming messages from other processes
 * msg_out for outgoing messages */
static packet_t *msg_in;
static packet_t *msg_out;


static struct timespec thread_time;

/* Function to shut down process cleanly
 * Logs reason for exit, frees allocated memory, closes
 * file decriptors */
static void fifo_exit( reason_e reason )
{
   clock_gettime(CLOCK_REALTIME, &thread_time);

   switch( reason )
   {
      case( REASON_SIGINT ):
         fprintf( stdout, "\nPID [%d] ( %ld.%ld secs )\nCaught SIGINT signal!\n",
                  getpid(), thread_time.tv_sec, thread_time.tv_nsec );
         fprintf( log, "\nPID [%d] ( %ld.%ld secs )\nCaught SIGINT signal!\n",
                  getpid(), thread_time.tv_sec, thread_time.tv_nsec );
         break;
      case( REASON_SIGPIPE ):
         fprintf( stdout, "\nPID [%d] ( %ld.%ld secs )\nConnection closed!\n",
                  getpid(), thread_time.tv_sec, thread_time.tv_nsec );
         fprintf( log, "\nPID [%d] ( %ld.%ld secs )\nConnection closed!\n",
                  getpid(), thread_time.tv_sec, thread_time.tv_nsec );
         break;
      case( REASON_CLEAN ):
         fprintf( stdout, "\nPID [%d] ( %ld.%ld secs )\nExiting!\n",
                  getpid(), thread_time.tv_sec, thread_time.tv_nsec );
         fprintf( log, "\nPID [%d] ( %ld.%ld secs )\nExiting!\n",
                  getpid(), thread_time.tv_sec, thread_time.tv_nsec ); 
         break;
      default:
         break;
   }
   fclose( log );
   free( msg_in );
   free( msg_out );
   close( fin );
   close( fout );
   exit(0);
}

/* Signal Handler
 * Handles SIGINT (Cntrl+C) and SIGPIPE - calling
 * fifo_exit() specifying which signal was caught */
void sig_handler( int signo )
{
   if( signo == SIGINT )
   {
      fifo_exit( REASON_SIGINT );
   }
   else if( signo == SIGPIPE )
   {
      fifo_exit( REASON_SIGPIPE );
   }
   return;
}

int main(void)
{
   /* Set up signal handling */
   signal( SIGINT, sig_handler );
   signal( SIGPIPE, sig_handler );

   /* FIFO File Path */
   char *path_to_fin  = "/tmp/fifo-two";
   char *path_to_fout = "/tmp/fifo-one";

   /* Create FIFO */
   mkfifo(path_to_fin, 0666);
   mkfifo(path_to_fout, 0666);

   /* Log file */
   log = fopen( "second_pid.log", "w" );

   fprintf( log, "PID %d - Named FIFO:\n",
            getpid() );


   if( 0 > (fout = open( path_to_fout, O_WRONLY )) )
   {
      perror( "Encountered error opening output FIFO!\n" );
      exit(1);
   }
   if( 0 > (fin = open( path_to_fin, O_RDONLY )) )
   {
      perror( "Encountered error opening input FIFO!\n" );
      exit(1);
   }
   fprintf( log, "Path to Input FIFO: %s\nFile Descriptor: %d\n",
            path_to_fin, fin );
   fprintf( log, "Path to Output FIFO: %s\nFile Descriptor: %d\n",
            path_to_fout, fout );

   /* Allocate memory for packet struct */
   msg_in = malloc( sizeof(packet_t) );
   if( NULL == msg_in )
   {
      fprintf( stderr, "Encountered error allocating memory for packet struct!\n" );
      exit(1);
   }

   msg_out = malloc( sizeof(packet_t) );
   if( NULL == msg_out )
   {
      fprintf( stderr, "Encountered error allocating memory for packet struct!\n" );
      exit(1);
   }


   int i = 0;
   while( 10 != i )
   {
      /* Read from FIFO */
      if( 0 > read( fin, msg_in, sizeof(*msg_in) ) )
      {
         perror("Encountered error while atttempting to read FIFO!\n" );
      }

      clock_gettime( CLOCK_REALTIME, &thread_time );

      if( MESSAGE_PRINT == msg_in->type )
      {
         fprintf( log, "\n( %ld.%ld secs ) - Received:\nHeader: %sBody: %s",
                  thread_time.tv_sec, thread_time.tv_nsec, msg_in->header, msg_in->body );
      }
      else
      {
         /* This is a command, so we don't want to print it - just handle it */
         fprintf( log, "\n( %ld.%ld secs ) - Received:   COMMAND",
                  thread_time.tv_sec, thread_time.tv_nsec );
      }

      if( NULL != sequence_b[i] )
      {
         /* Write string to body of message */
         sprintf( msg_out->body, "%s", sequence_b[i] );
         msg_out->type = MESSAGE_PRINT;

         sprintf( msg_out->header, "From PID %d - Length %ld\n",
                  getpid(), strlen(sequence_b[i]) );
      }
      else
      {
         /* This is a command */
         msg_out->type = MESSAGE_CMD;
      }

      /* Write full message to pipe */
      if( 0 > write( fout, msg_out, sizeof(*msg_out) ) )
      {
         perror( "Encountered error while attempting to write to FIFO!\n" );
      }

      clock_gettime( CLOCK_REALTIME, &thread_time );

      if( MESSAGE_PRINT == msg_out->type )
      {
         fprintf( log, "\n( %ld.%ld secs ) - Sending:    %s",
                  thread_time.tv_sec, thread_time.tv_nsec, msg_out->body );
      }
      else
      {
         /* This is a command, so we don't want to print it - just handle it */
         fprintf( log, "\n( %ld.%ld secs ) - Sending:    COMMAND",
                  thread_time.tv_sec, thread_time.tv_nsec );
      }

      i++;
   }
   fifo_exit( REASON_CLEAN );
   return 0;
}
