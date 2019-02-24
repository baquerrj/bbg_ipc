#ifndef COMMON_H
#define COMMON_H

#define BODY_SIZE    50
#define HEADER_SIZE  30

#define FIFO_SIZE    BODY_SIZE + HEADER_SIZE


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



#endif /* COMMON_H */
