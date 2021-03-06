#ifndef COMMON_H
#define COMMON_H

#define SHM_SEGMENT_SIZE 65536
#define SHM_SEGMENT_NAME "/shm"
#define SEMA_NAME        "/da_semaphore"

/* Sockets and FIFO */
#define PORT 8080
#define BODY_SIZE    50
#define HEADER_SIZE  30

/* Max Number of Messages in Queue */
#define NUM_MESSAGES 10
#define QUEUE_ONE_NAME   "/queue-one"
#define QUEUE_TWO_NAME   "/queue-two"

/* Enumerator with reasons for exit */
typedef enum reasons {
   REASON_BEGIN   = 0,
   REASON_SIGINT,
   REASON_SIGPIPE,
   REASON_CLEAN,
   REASON_MAX
} reason_e;

/* Defines type of message */
typedef enum message {
   MESSAGE_BEGIN = 0,
   MESSAGE_PRINT,       /* Sring for printing */
   MESSAGE_CMD,         /* Command for LED Control */
   MESSAGE_MAX
} message_e;

/* Struct used to contain message information */
typedef struct packet {
   message_e type;            /* String or Command */
   char header[HEADER_SIZE];  /* Message header - contains PID of sender */
   char body[BODY_SIZE];      /* Actual contents for message */
} packet_t;

char *sequence_a[] = {
   "First message in the sequence\n",
   NULL,   /* Command */
   NULL,   /* Command */
   "Second string to send, but fourth message so far!\n",
   "Two strings back to back\n",
   NULL,   /* Command */
   "Let's do three in a row!\n",
   "Almost done...\n",
   "This is the ninth message so far!\n",
   NULL    /* Command */
};

char *sequence_b[] = {
   "First message in the sequence\n",
   "This is the second message so far!\n",
   NULL,   /* Command */
   NULL,   /* Command */
   "Third string to send, but the fifth message so far!\n",
   NULL,   /* Command */
   "Finish up with only strings, I guess...\n",
   "Filler, filler, filler, filler!\n",
   "Second to last!\n",
   "Wooo! Last message!\n"
};




#endif /* COMMON_H */
