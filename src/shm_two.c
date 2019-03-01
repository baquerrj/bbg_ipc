#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <semaphore.h>
#include <errno.h>
/* /sys includes */
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
/* Local Inlcudes */
#include "common.h"

static packet_t *shm;
static sem_t    *sem;
static int      shm_fd;
static FILE     *log;

/* Structs for messages:
 * msg_in for the incoming messages from other processes
 * msg_out for outgoing messages */
static packet_t *msg_in;
static packet_t *msg_out;

static struct timespec thread_time;


static void *get_shared_memory(void)
{
   void *shm_p;

   shm_fd = shm_open( SHM_SEGMENT_NAME, O_CREAT | O_EXCL | O_RDWR, 0666 );
   if( 0 < shm_fd )
   {
      fprintf( stdout, "Creating shared memroy and setting size to %d bytes\n",
               SHM_SEGMENT_SIZE );
      fprintf( log, "Creating shared memroy and setting size to %d bytes\n",
               SHM_SEGMENT_SIZE );

      if( 0 > ftruncate( shm_fd, SHM_SEGMENT_SIZE) )
      {
         perror( "Encountered error setting size of shared memroy" );
         exit(1);
      }

      sem = sem_open( SEMA_NAME, O_RDWR | O_CREAT | O_EXCL, 0666, 1 );
      if( EEXIST == errno )
      {
         perror( "Semaphore already exists" );
         sem = sem_open( SEMA_NAME, O_RDWR, 1 );
      }
		if( SEM_FAILED == sem )
      {
			perror( "Encountered error creating semapahore for shared memory" );
         exit(1);
      }
	}
   else if( -1 == shm_fd && EEXIST == errno )
   {
		/* Already exists: open again without O_CREAT */
		shm_fd = shm_open( SHM_SEGMENT_NAME, O_RDWR, 0 );
		sem = sem_open(SEMA_NAME, O_RDWR);

		if( SEM_FAILED == sem )
      {
			perror( "Encountered error creating semapahore for shared memory" );
         exit(1);
      }
	}
   else if( -1  == shm_fd )
   {
		perror( "shm_open " SHM_SEGMENT_NAME );
		exit(1);
	}

	/* Map the shared memory */
	shm_p = mmap( NULL, SHM_SEGMENT_SIZE, PROT_READ | PROT_WRITE,
		     MAP_SHARED, shm_fd, 0 );

	if( NULL == shm_p )
   {
		perror( "Encountered error memory mapping shared memory" );
		exit(1);
	}
	return shm_p;
}

static void shm_exit( reason_e reason )
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

   /* Close Memory */
   munmap( shm, SHM_SEGMENT_SIZE );
   shm_unlink( SHM_SEGMENT_NAME );
   sem_unlink( SEMA_NAME );
   exit(0);
}

void sig_handler(int signo)
{
   if( signo == SIGINT )
   {
      shm_exit( REASON_SIGINT );
   }
   else if( signo == SIGPIPE )
   {
      shm_exit( REASON_SIGPIPE );
   }
   return;
}

int main(void)
{
   /* Set up signal handling */
   signal( SIGINT, sig_handler );
   signal( SIGPIPE, sig_handler );

   /* Log file */
   log = fopen( "shm_two.log", "w" );

   fprintf( log, "PID %d - Shared Memory:\n",
            getpid() );

   shm = get_shared_memory();
   
   fprintf( log, "File Descriptor: %d\nShared Memory Name: %s\nSize: %d bytes\nSemaphore Name: %s",
            shm_fd, SHM_SEGMENT_NAME, SHM_SEGMENT_SIZE, SEMA_NAME );
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

   sem_post( sem );
   int i = 0;
   while( 10 != i )
   {
      /* Read from Shared Memory */
      sem_wait( sem );
      memcpy( msg_in, shm, sizeof(*shm) );
      sem_post( sem );

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

      /* Write full message to memory */
      sem_wait( sem );
      memcpy( shm, msg_out, sizeof(*msg_out));
      sem_post( sem );

      clock_gettime( CLOCK_REALTIME, &thread_time );

      if( MESSAGE_PRINT == msg_out->type )
      {
         fprintf( log, "\n( %ld.%ld secs ) - Sending: %s",
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
   shm_exit( REASON_CLEAN );
   return 0;
}
