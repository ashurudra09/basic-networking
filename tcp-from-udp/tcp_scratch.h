#ifndef MINI_PROJECT_2_ASHURUDRA09_TCP_SCRATCH_H
#define MINI_PROJECT_2_ASHURUDRA09_TCP_SCRATCH_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <poll.h>

// MACROS for configuring the protocol
#define buffsize 5              // The size of the data payload in each packet/chunk.
#define timeout 400             // The time in milliseconds the sender waits for an ACK before retransmitting.
#define RETRANSMISSION_CHECK 1  // If set to 1, the receiver will artificially delay ACKs to test the sender's retransmission logic. Set to 0 for normal operation.

// Structure to represent a data packet.
// It contains the message payload, a sequence number, and an acknowledgment number.
typedef struct data Data;
struct data{
    char msg[buffsize]; // Data chunk
    int seq;            // Sequence number: total bytes sent so far
    int ack;            // Acknowledgment number: sequence number of the last packet received from the other peer
};

#endif //MINI_PROJECT_2_ASHURUDRA09_TCP_SCRATCH_H