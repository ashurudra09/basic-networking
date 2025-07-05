#include "tcp_scratch.h"

void Initialize(Data* d){
    bzero(d->msg, buffsize);
    d->seq = 0;
    d->ack = 0;
}

// rameshdhallebhaisaabsundarsusheel

int main(int argc, char **argv){
    if (argc != 2){
        printf("Usage: %s <port>\n", argv[0]);
        exit(0);
    }
    int option;
    while(1){
        Data d_msg[1000], d_rec;

        Initialize(&d_rec);
        for (int i = 0; i < 1000; i++) {
            Initialize(&d_msg[i]);
        }

        printf("1 for send, 0 for receive: ");
        scanf("%d", &option);
        if(option == 1) {
            char *ip = "127.0.0.1";
            int port = atoi(argv[1]);

            int sockfd;
            struct sockaddr_in addr;
            char buffer[1024];
            socklen_t addr_size;

            sockfd = socket(AF_INET, SOCK_DGRAM, 0);
            memset(&addr, '\0', sizeof(addr));
            addr.sin_family = AF_INET;
            addr.sin_port = htons(port);
            addr.sin_addr.s_addr = inet_addr(ip);

            bzero(buffer, 1024);
            scanf("%s", buffer);
            char *copy = buffer;
            char msg[1024] = {};
            int unack_update = 0, unack_index = 0, msg_index = 0, done = 0;
            int total_chunks = (int) strlen(buffer)/buffsize;
            if(strlen(buffer) > total_chunks*buffsize)
                total_chunks++;
            int counter = 0;
            while (done != 2) {
                if (unack_update == 1) {                           // update unackowledged chunk
                    unack_index++;
                    unack_update = 0;
                }
                if(strlen(copy) == 0){
                    done = 1;
                    if(unack_index == msg_index) {
                        strcpy(d_msg[msg_index].msg, "N=");   // DONE message, after transmission ends
                        done = 2;
                    }
                    else{
                        copy = d_msg[unack_index].msg;
                    }
                }
                else{
                    strncpy(d_msg[msg_index].msg, copy, buffsize);   // get chunk of message
                    copy += (int) strlen(d_msg[msg_index].msg);       // making copy point to next chunk
                }
                if(done != 1){
                    if (msg_index > 0) {
                        d_msg[msg_index].seq = d_msg[msg_index - 1].seq + (int) strlen(d_msg[msg_index].msg);
                        d_msg[msg_index].ack = d_rec.seq;
                    } else {
                        d_msg[msg_index].seq = d_msg[999].seq + (int) strlen(d_msg[msg_index].msg);
                        d_msg[msg_index].ack = d_rec.seq;
                    }
                    if(done == 2){
                        snprintf(msg, 1024, "%s%d\t%d\t%d", d_msg[msg_index].msg, total_chunks,
                                 d_msg[msg_index].seq, d_msg[msg_index].ack);     // copying formatted string to msg
                    }
                    else{
                        snprintf(msg, 1024, "%s\t%d\t%d", d_msg[msg_index].msg,
                                 d_msg[msg_index].seq, d_msg[msg_index].ack);     // copying formatted string to msg
                    }
                }
                else if(counter*10 == timeout){
                    if (unack_index > 0) {
                        d_msg[unack_index].ack = d_rec.seq;
                    } else {
                        d_msg[unack_index].ack = d_rec.seq;
                    }
                    counter = 0;
                    snprintf(msg, 1024, "%s\t%d\t%d", d_msg[unack_index].msg,
                             d_msg[unack_index].seq, d_msg[unack_index].ack);     // copying formatted string to msg
                }

                if(done == 0 || counter == 0 || done == 2){
                    if (sendto(sockfd, msg, 1024, 0, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
                        perror("sendto");
                        exit(1);
                    }
                    printf("[+]Data send: %s\n", msg);
                }
                if(done == 0){
                    msg_index++;
                    if (msg_index == 1000)
                        msg_index = 0;
                }

                char input[1024];
                bzero(input, 1024);
                addr_size = sizeof(addr);
                struct pollfd fds[1];
                fds[0].fd = sockfd;
                fds[0].events = POLLIN;

                poll(fds, 1, 10);
                counter++;
                if(fds[0].revents & POLLIN) {
                    if (recvfrom(sockfd, input, 1024, 0, (struct sockaddr *) &addr, &addr_size) < 0) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) {}
                        else {
                            perror("recvfrom");
                            exit(1);
                        }
                    }
                    unack_update = 1;
                }
                if (strlen(input) > 0)
                    printf("[+]Data recv: %s\n", input);
                else {
                    bzero(d_rec.msg, buffsize);
                    continue;
                }
                char *t = strtok(input, "\t");
                if (t && strlen(t) > 0)
                    strncpy(d_rec.msg, t, buffsize);
                t = strtok(NULL, "\t");
                if (t && strlen(t) > 0)
                    d_rec.seq = atoi(t);
                t = strtok(NULL, "\t");
                if (t && strlen(t) > 0)
                    d_rec.ack = atoi(t);
            }
            close(sockfd);
        }
        else if(option == 0){

            char *ip = "127.0.0.1";
            int port = atoi(argv[1]);

            int sockfd;
            struct sockaddr_in server_addr, client_addr;
            char buffer[1024];
            socklen_t addr_size;
            int n;

            sockfd = socket(AF_INET, SOCK_DGRAM, 0);
            if (sockfd < 0){
                perror("[-]socket error");
                exit(1);
            }

            memset(&server_addr, '\0', sizeof(server_addr));
            server_addr.sin_family = AF_INET;
            server_addr.sin_port = htons(port);
            server_addr.sin_addr.s_addr = inet_addr(ip);

            n = bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr));
            if (n < 0) {
                perror("[-]bind error");
                exit(1);
            }

            char msg[1024] = {};
            int msg_index = 0, done = 0, chunks = 0;
            while (done != 1) {
                char input[1024];
                bzero(input, 1024);
                addr_size = sizeof(client_addr);
                if (recvfrom(sockfd, input, 1024, 0, (struct sockaddr *) &client_addr, &addr_size) < 0) {
                    perror("recvfrom");
                    exit(1);
                }
                if (strlen(input) > 0)
                    printf("[+]Data recv: %s\n", input);
                else {
                    continue;
                }
                char *t = strtok(input, "\t");
                if (t && strlen(t) > 0) {
                    strncpy(d_msg[msg_index].msg, t, buffsize);
                    if(strcmp(d_msg[msg_index-1].msg, d_msg[msg_index].msg) == 0){
                        continue;
                    }
                }
                t = strtok(NULL, "\t");
                if (t && strlen(t) > 0)
                    d_msg[msg_index].seq = atoi(t);
                t = strtok(NULL, "\t");
                if (t && strlen(t) > 0)
                    d_msg[msg_index].ack = atoi(t);

                if(strncmp(d_msg[msg_index].msg, "N=", 2) == 0){
                    done = 1;
                    chunks = atoi(d_msg[msg_index].msg+2);
                }

                d_rec.seq += 3;
                d_rec.ack = (d_msg[msg_index].seq);
                snprintf(msg, 1024, "Ack\t%d\t%d",d_rec.seq, d_rec.ack);     // copying formatted string to msg
                if (sendto(sockfd, msg, 1024, 0, (struct sockaddr *) &client_addr, sizeof(client_addr)) < 0) {
                    perror("sendto");
                    exit(1);
                }
                printf("[+]Data send: %s\n", msg);
                msg_index++;
                if (msg_index == 1000)
                    msg_index = 0;
                if(RETRANSMISSION_CHECK){ usleep(1000000); }
            }
            printf("Final output: ");
            for(int i = 0; i < chunks; i++){
                printf("%s", d_msg[i].msg);
            }
            printf("\n");
            close(sockfd);
        }
        else{
            printf("Invalid option!\n");
        }

    }
    return 0;
}