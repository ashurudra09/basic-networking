#include "tcp_scratch.h"

// Helper function to initialize a Data struct
void Initialize(Data* d){
    bzero(d->msg, buffsize);
    d->seq = 0;
    d->ack = 0;
}

int main(int argc, char **argv){
    if (argc != 2){
        printf("Usage: %s <port>\n", argv[0]);
        exit(0);
    }
    
    // Main loop allows for multiple send/receive sessions
    while(1){
        // Arrays to store sent message chunks and received data
        Data d_msg[1000], d_rec;

        // Initialize all data structures
        Initialize(&d_rec);
        for (int i = 0; i < 1000; i++) {
            Initialize(&d_msg[i]);
        }

        // User chooses to be a sender or a receiver
        int option;
        printf("1 for send, 0 for receive: ");
        scanf("%d", &option);

        // SENDER LOGIC
        if(option == 1) {
            char *ip = "127.0.0.1";
            int port = atoi(argv[1]);

            int sockfd;
            struct sockaddr_in addr;
            char buffer[1024]; // Buffer for the full message to be sent
            socklen_t addr_size;

            // Setup UDP socket
            sockfd = socket(AF_INET, SOCK_DGRAM, 0);
            memset(&addr, '\0', sizeof(addr));
            addr.sin_family = AF_INET;
            addr.sin_port = htons(port);
            addr.sin_addr.s_addr = inet_addr(ip);

            printf("Enter message to send: ");
            scanf("%s", buffer);
            char *copy = buffer; // Pointer to traverse the original message
            char msg[1024] = {}; // Formatted packet string to be sent
            
            // State variables for sending logic
            int unack_update = 0; // Flag to indicate a new ACK was received
            int unack_index = 0;  // Index of the oldest unacknowledged chunk
            int msg_index = 0;    // Index for the next new chunk to be sent
            int done = 0;         // State flag: 0=sending, 1=all sent, waiting for final ACKs, 2=finished
            int total_chunks = (int) strlen(buffer)/buffsize;
            if(strlen(buffer) % buffsize != 0)
                total_chunks++;
            int counter = 0; // Counter for timeout logic

            // Main sending loop
            while (done != 2) {
                // If an ACK was received, move to the next unacknowledged chunk
                if (unack_update == 1) {
                    unack_index++;
                    unack_update = 0;
                }
                
                // Check if the entire message has been chunked and sent at least once
                if(strlen(copy) == 0){
                    done = 1; // All chunks have been sent once
                    // If all sent chunks have been acknowledged
                    if(unack_index == msg_index) {
                        // Prepare the final termination message
                        strcpy(d_msg[msg_index].msg, "N=");
                        done = 2; // Mark as completely finished
                    }
                    else{
                        // If not all ACKs received, point 'copy' to the unacked chunk for retransmission
                        copy = d_msg[unack_index].msg;
                    }
                }
                else{
                    // Create a new chunk from the original message
                    strncpy(d_msg[msg_index].msg, copy, buffsize);
                    copy += (int) strlen(d_msg[msg_index].msg);
                }
                
                // If not in retransmission-only mode, prepare the packet data
                if(done != 1){
                    // Calculate sequence number (cumulative bytes sent)
                    d_msg[msg_index].seq = (msg_index > 0 ? d_msg[msg_index - 1].seq : 0) + (int) strlen(d_msg[msg_index].msg);
                    d_msg[msg_index].ack = d_rec.seq; // Piggyback the last received sequence number as our ACK
                    
                    if(done == 2){ // Format the final termination packet
                        snprintf(msg, 1024, "%s%d\t%d\t%d", d_msg[msg_index].msg, total_chunks, d_msg[msg_index].seq, d_msg[msg_index].ack);
                    }
                    else{ // Format a regular data packet
                        snprintf(msg, 1024, "%s\t%d\t%d", d_msg[msg_index].msg, d_msg[msg_index].seq, d_msg[msg_index].ack);
                    }
                }
                // Retransmission logic: if timeout is reached
                else if(counter*10 >= timeout){
                    d_msg[unack_index].ack = d_rec.seq;
                    counter = 0; // Reset timeout counter
                    // Format the retransmission packet
                    snprintf(msg, 1024, "%s\t%d\t%d", d_msg[unack_index].msg, d_msg[unack_index].seq, d_msg[unack_index].ack);
                }

                // Send the packet if it's a new chunk, a retransmission, or the final packet
                if(done == 0 || counter == 0 || done == 2){
                    sendto(sockfd, msg, 1024, 0, (struct sockaddr *) &addr, sizeof(addr));
                    printf("[+]Data send: %s\n", msg);
                }
                
                if(done == 0){ msg_index++; } // Move to the next message index for the next new chunk

                // Use poll() to check for incoming ACKs without blocking indefinitely
                char input[1024];
                bzero(input, 1024);
                addr_size = sizeof(addr);
                struct pollfd fds[1];
                fds[0].fd = sockfd;
                fds[0].events = POLLIN;

                poll(fds, 1, 10); // Wait for 10ms
                counter++;
                
                // If an ACK arrived
                if(fds[0].revents & POLLIN) {
                    recvfrom(sockfd, input, 1024, 0, (struct sockaddr *) &addr, &addr_size);
                    unack_update = 1; // Set flag to update the unacknowledged index
                }
                
                if (strlen(input) > 0) {
                    printf("[+]Data recv: %s\n", input);
                    // Parse the received ACK packet
                    char *t = strtok(input, "\t");
                    if (t) strncpy(d_rec.msg, t, buffsize);
                    t = strtok(NULL, "\t");
                    if (t) d_rec.seq = atoi(t);
                    t = strtok(NULL, "\t");
                    if (t) d_rec.ack = atoi(t);
                }
            }
            close(sockfd);
        }
        // RECEIVER LOGIC
        else if(option == 0){
            char *ip = "127.0.0.1";
            int port = atoi(argv[1]);

            int sockfd;
            struct sockaddr_in server_addr, client_addr;
            int n;

            // Setup UDP socket and bind it to the port
            sockfd = socket(AF_INET, SOCK_DGRAM, 0);
            memset(&server_addr, '\0', sizeof(server_addr));
            server_addr.sin_family = AF_INET;
            server_addr.sin_port = htons(port);
            server_addr.sin_addr.s_addr = inet_addr(ip);
            n = bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr));
            if (n < 0) { perror("[-]bind error"); exit(1); }
            printf("Receiver is listening on port %d...\n", port);

            char msg[1024] = {}; // Buffer for outgoing ACK packets
            int msg_index = 0;   // Index to store received chunks
            int done = 0;        // Flag to indicate when transmission is complete
            int chunks = 0;      // Total number of chunks to expect

            // Main receiving loop
            while (done != 1) {
                char input[1024];
                bzero(input, 1024);
                socklen_t addr_size = sizeof(client_addr);
                // Wait for an incoming packet (blocking)
                recvfrom(sockfd, input, 1024, 0, (struct sockaddr *) &client_addr, &addr_size);
                printf("[+]Data recv: %s\n", input);
                
                // Parse the incoming packet using strtok
                char *t = strtok(input, "\t");
                if (t) {
                    // Basic duplicate detection: if the message is the same as the last one, ignore.
                    // A more robust implementation would use sequence numbers.
                    if(msg_index > 0 && strcmp(d_msg[msg_index-1].msg, t) == 0){
                        continue;
                    }
                    strncpy(d_msg[msg_index].msg, t, buffsize);
                }
                t = strtok(NULL, "\t");
                if (t) d_msg[msg_index].seq = atoi(t);
                t = strtok(NULL, "\t");
                if (t) d_msg[msg_index].ack = atoi(t);

                // Check for the termination message "N="
                if(strncmp(d_msg[msg_index].msg, "N=", 2) == 0){
                    done = 1; // Mark transmission as complete
                    chunks = atoi(d_msg[msg_index].msg+2); // Extract total number of chunks
                }

                // Prepare and send an ACK
                d_rec.seq += 3; // Arbitrary increment for ACK packet's own sequence number
                d_rec.ack = d_msg[msg_index].seq; // Acknowledge the sequence number we just received
                snprintf(msg, 1024, "Ack\t%d\t%d",d_rec.seq, d_rec.ack);
                
                // Artificially delay ACK if RETRANSMISSION_CHECK is enabled
                if(RETRANSMISSION_CHECK && !done){ usleep(1000000); } // sleep for 1 sec
                
                sendto(sockfd, msg, 1024, 0, (struct sockaddr *) &client_addr, sizeof(client_addr));
                printf("[+]Data send: %s\n", msg);
                
                msg_index++;
            }
            
            // Reassemble and print the final message
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