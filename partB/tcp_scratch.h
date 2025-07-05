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

#define buffsize 5              // size of chunk
#define timeout 400             // timeout in ms
#define RETRANSMISSION_CHECK 1  // if this is set to 1, you can see retransmission

typedef struct data Data;
struct data{
    char msg[buffsize];
    int seq;
    int ack;
};

#endif //MINI_PROJECT_2_ASHURUDRA09_TCP_SCRATCH_H
