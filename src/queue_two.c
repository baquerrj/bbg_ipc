/* https://www.softprayog.in/programming/interprocess-communication-using-posix-message-queues-in-linux */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <semaphore.h>
#include <errno.h>
#include <mqueue.h>
/* /sys includes */
#include <sys/types.h>
#include <sys/stat.h>
/* Local Inlcudes */
#include "common.h"

/* File Descriptors for named queues:
 * fin is the input FIFO for process
 * fout is output FIFO for process */
static mqd_t    queue_in;
static mqd_t    queue_out;

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
static void queue_exit( reason_e reason )
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

   mq_close( queue_in );
   mq_close( queue_out );
   mq_unlink( QUEUE_ONE_NAME );
   mq_unlink( QUEUE_TWO_NAME );
   exit(0);
}

void sig_handler(int signo)
{
   if( signo == SIGINT )
   {
      queue_exit( REASON_SIGINT );
   }
   else if( signo == SIGPIPE )
   {
      queue_exit( REASON_SIGPIPE );
   }
   return;
}

int main(void)
{
   /* Set up signal handling */
   signal( SIGINT, sig_handler );
   signal( SIGPIPE, sig_handler );

   /* Log file */
   log = fopen( "queue_two.log", "w" );

   fprintf( log, "PID %d - POSIX QUEUE:\n",
            getpid() );

   struct mq_attr attr;
   attr.mq_flags = 0;
   attr.mq_maxmsg = NUM_MESSAGES;
   attr.mq_msgsize = sizeof( *msg_in );
   attr.mq_curmsgs = 0;


   queue_in = mq_open( QUEUE_TWO_NAME, O_CREAT | O_EXCL | O_RDWR, 0666, &attr );
   if( queue_in == -1 && EEXIST == errno )
   {
      queue_in = mq_open( QUEUE_TWO_NAME, O_RDWR );
   }
   else if( queue_in == -1 )
   {
		perror( "mq_open " QUEUE_TWO_NAME );
		exit(1);
	}

   queue_out = mq_open( QUEUE_ONE_NAME, O_CREAT | O_EXCL | O_RDWR, 0666, &attr );
   if( queue_out == -1 && EEXIST == errno )
   {
      queue_out = mq_open( QUEUE_ONE_NAME, O_RDWR );
   }
   else if( queue_out == -1 )
   {
		perror( "mq_open " QUEUE_ONE_NAME );
		exit(1);
	}
   fprintf( log, "Input Queue Name: %s\nMaximum Number of Messages: %lu\nMaximum Message Size: %lu\n",
            QUEUE_TWO_NAME, attr.mq_maxmsg, attr.mq_msgsize );

   mq_getattr( queue_out, &attr );
   fprintf( log, "Output Queue Name: %s\nMaximum Number of Messages: %lu\nMaximum Message Size: %lu\n",
            QUEUE_TWO_NAME, attr.mq_maxmsg, attr.mq_msgsize );

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
      /* Read from input queue */
      if( -1 == mq_receive( queue_in, (char *)msg_in, sizeof(*msg_in), NULL ) )
      {
         perror( "Error receiving" );
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
         fprintf( log, "\n( %ld.%ld secs ) - Received: COMMAND",
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

      /* Write full message to output queue */
      if( -1 == mq_send( queue_out, (const char *)msg_out, sizeof(*msg_out), 0 ) )
      {
         perror( "Error sending" );
      }

      clock_gettime( CLOCK_REALTIME, &thread_time );

      if( MESSAGE_PRINT == msg_out->type )
      {
         fprintf( log, "\n( %ld.%ld secs ) - Sending: %s",
                  thread_time.tv_sec, thread_time.tv_nsec, msg_out->body );
      }
      else
      {
         /* This is a command, so we don't want to print it - just handle it */
         fprintf( log, "\n( %ld.%ld secs ) - Sending: COMMAND",
                  thread_time.tv_sec, thread_time.tv_nsec );
      }

      i++;
   }
   queue_exit( REASON_CLEAN );
   return 0;
}
