#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
/* /sys includes */
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>

/* Local Inlcudes */
#include "common.h"

static FILE     *log;
static packet_t *msg_in;
static packet_t *msg_out;

static struct timespec thread_time;

static void socket_exit(void)
{
   clock_gettime(CLOCK_REALTIME, &thread_time);

   fprintf( stdout, "\nPID [%d] ( %ld.%ld secs )\nCaught SIGINT signal!\n",
            getpid(), thread_time.tv_sec, thread_time.tv_nsec );
   fprintf( log, "\nPID [%d] ( %ld.%ld secs )\nCaught SIGINT signal!\n",
            getpid(), thread_time.tv_sec, thread_time.tv_nsec );

   fclose( log );
   free( msg_in );
   free( msg_out );
   exit(0);
}

void sig_handler(int signo)
{
   if( signo == SIGINT )
   {
      socket_exit();
   }
   else
   {
      fprintf( stdout, "PID %d caught unknown signal!\n",
               getpid() );
      fprintf( log, "PID %d caught unknown signal!\n",
               getpid() );
   }
   return;
}

int main(void)
{
   int sock, new_socket;
   struct sockaddr_in my_addr;
   int addrlen = sizeof(my_addr);

   char *msg = sequence_a[0];

   int opt = 1;

   /* Set up signal handling */
   signal( SIGINT, sig_handler );

   /* Log file */
   log = fopen( "first_pid.log", "w" );

   fprintf( log, "PID %d - Sockets:\n",
            getpid());

   if( 0 > (sock = socket( AF_INET, SOCK_STREAM, 0 )) )
   {
      perror( "Encountered error creating socket" );
      return 1;
   }

   if( setsockopt( sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt) ))
   {
      perror( "Encountered error setting sock option" );
      return 1;
   }
   my_addr.sin_family = AF_INET;
   my_addr.sin_addr.s_addr = INADDR_ANY;
   my_addr.sin_port = htons ( 8080 );

   // Bind socket
   if( 0 > bind( sock, (struct sockaddr *)&my_addr, sizeof(my_addr) ) )
   {
      perror( "Encountered error binding socket" );
      return 1;
   }

   // Listen
   if( 0 > listen( sock, 3 ) )
   {
      perror( "Encountered error listening on socket" );
      return 1;
   }

   if( 0 > (new_socket = accept(sock, (struct sockaddr *)&my_addr, (socklen_t*)&addrlen)) )
   {
      perror( "Encountered error accepting new connection" );
      return 1;
   }

   fprintf( log, "Socket ID: %d\nSocket Option: %d\nListening on port: %d\nFamily: %d\nSource Address: %d\n",
            sock, opt, 8080, AF_INET, INADDR_ANY );

   /* Allocate memory for packet struct */
   msg_in = malloc( sizeof(packet_t) );
   if( NULL == msg_in )
   {
      fprintf( stderr, "Encountered error allocating memory for packet struct!\n" );
      return 1;
   }

   msg_out = malloc( sizeof(packet_t) );
   if( NULL == msg_out )
   {
      fprintf( stderr, "Encountered error allocating memory for packet struct!\n" );
      return 1;
   }

   int i = 0;
   while( 10 != i )
   {
      read( new_socket, msg_in, sizeof(*msg_in) );

      send( new_socket, msg, sizeof(*msg), 0 );
      i++;

      fprintf( log, "\n( %ld. %ld secs ) - Received:\nHeader: %sBody: %s",
               thread_time.tv_sec, thread_time.tv_nsec, msg_in->header, msg_in->body );
#if 0
      if( NULL != sequence_a[i] )
      {
         /* Write string to body of message */
         sprintf( msg_out->body, "%s", sequence_a[i] );
         msg_out->type = MESSAGE_PRINT;

         sprintf( msg_out->header, "From PID %d - Length %ld\n",
               getpid(), strlen(sequence_a[i]) );
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
         fprintf( log, "\n( %ld. %ld secs ) - Sending: %s",
                  thread_time.tv_sec, thread_time.tv_nsec, msg_out->body );
      }
      else
      {
         /* This is a command, so we don't want to print it - just handle it */
         fprintf( log, "\n( %ld. %ld secs ) - Sending: COMMAND",
                  thread_time.tv_sec, thread_time.tv_nsec );
      }

      /* Read from Input FIFO */
      if( 0 > read( fin, msg_in, sizeof(*msg_in) ) )
      {
         perror("Encountered error while atttempting to read FIFO!\n" );
      }

      clock_gettime( CLOCK_REALTIME, &thread_time );

      if( MESSAGE_PRINT == msg_in->type )
      {
         fprintf( log, "\n( %ld. %ld secs ) - Received:\nHeader: %sBody: %s",
                  thread_time.tv_sec, thread_time.tv_nsec, msg_in->header, msg_in->body );
      }
      else
      {
         /* This is a command, so we don't want to print it - just handle it */
         fprintf( log, "\n( %ld. %ld secs ) - Received: COMMAND",
                  thread_time.tv_sec, thread_time.tv_nsec );
      }
      i++;
      if( 10 == i )
      {
         i = 0;
      }
#endif
   }
   return 0;
}
