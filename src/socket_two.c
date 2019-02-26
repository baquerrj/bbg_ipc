#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
/* /sys includes */
//#include <sys/types.h>
//#include <sys/stat.h>
#include <sys/socket.h>
//#include <netinet/in.h>
#include <arpa/inet.h>
/* Local Inlcudes */
#include "common.h"

static FILE     *log;
static packet_t *msg_in;
static packet_t *msg_out;

static int sock_fd;
static struct timespec thread_time;

typedef enum reasons {
   REASON_BEGIN,
   REASON_SIGINT,
   REASON_SIGPIPE,
   REASON_MAX
} reason_e;

static void socket_exit( reason_e reason )
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
      default:
         break;
   }
   fclose( log );
   free( msg_in );
   free( msg_out );
   close( sock_fd );
   exit(0);
}

void sig_handler(int signo)
{
   if( signo == SIGINT )
   {
      socket_exit( REASON_SIGINT );
   }
   else if( signo == SIGPIPE )
   {
      socket_exit( REASON_SIGPIPE );
   }
   return;
}

int main(void)
{
   struct sockaddr_in serv_addr;

   /* Set up signal handling */
   signal( SIGINT, sig_handler );
   signal( SIGPIPE, sig_handler );
   /* Log file */
   log = fopen( "client.log", "w" );

   fprintf( log, "PID %d - Sockets:\n",
            getpid());

   if( 0 > (sock_fd = socket( AF_INET, SOCK_STREAM, 0 )) )
   {
      perror( "Encountered error creating socket" );
      return 1;
   }

   memset( &serv_addr, '0', sizeof(serv_addr) );

   serv_addr.sin_family = AF_INET;
   serv_addr.sin_port = htons ( PORT );

   if( 0 >= inet_pton( AF_INET, "127.0.0.1", &serv_addr.sin_addr ) )
   {
      perror( "Error" );
      return 1;
   }

   if( 0 > connect(sock_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr) ) )
   {
      perror( "Encountered error accepting new connection" );
      return 1;
   }

   fprintf( log, "Socket ID: %d\nPORT: %d\nFamily: %d\nSource Address: %d\n",
            sock_fd, 8080, AF_INET, INADDR_ANY );

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
      /* Read from socket */
      if( 0 > read( sock_fd, msg_in, sizeof(*msg_in) ) )
      {
         perror("Encountered error while atttempting to read socket" );
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

      /* Write full message to socket */
      if( 0 > send( sock_fd, msg_out, sizeof(*msg_out), 0) )
      {
         perror( "Encountered error while attempting to write to socket" );
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
      i++;
      if( 10 == i )
      {
         i = 0;
      }
   }
   return 0;
}
