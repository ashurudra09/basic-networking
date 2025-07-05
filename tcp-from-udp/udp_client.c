#include "tcp_scratch.h"
#include <stdbool.h>

// Helper function to remove trailing newline from fgets
void remove_newline(char *str) {
    str[strcspn(str, "\n")] = 0;
}

// Extended data structure to track acknowledgement status
typedef struct {
    Data packet;
    bool acknowledged;
} Segment;

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
    
    while(1){
        int option;
        printf("1 for send, 0 for receive: ");
        scanf("%d", &option);
        getchar(); // Consume the newline character left by scanf

        // SENDER LOGIC
        if(option == 1) {
            char *ip = "127.0.0.1";
            int port = atoi(argv[1]);

            int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
            struct sockaddr_in addr;
            memset(&addr, '\0', sizeof(addr));
            addr.sin_family = AF_INET;
            addr.sin_port = htons(port);
            addr.sin_addr.s_addr = inet_addr(ip);

            char full_message[4096];
            printf("Enter message to send: ");
            fgets(full_message, sizeof(full_message), stdin);
            remove_newline(full_message);

            int total_len = strlen(full_message);
            int total_chunks = (total_len + buffsize - 1) / buffsize;
            Segment segments[total_chunks];
            int current_seq = 0;

            // 1. Chunk the message and prepare all segments
            for (int i = 0; i < total_chunks; i++) {
                strncpy(segments[i].packet.msg, full_message + i * buffsize, buffsize);
                current_seq += strlen(segments[i].packet.msg);
                segments[i].packet.seq = current_seq;
                segments[i].packet.ack = 0; // Will be updated later
                segments[i].acknowledged = false;
            }

            printf("Message chunked into %d segments.\n", total_chunks);
            int chunks_acked = 0;

            // 2. Send loop with retransmission
            while (chunks_acked < total_chunks) {
                // Send all unacknowledged packets
                for (int i = 0; i < total_chunks; i++) {
                    if (!segments[i].acknowledged) {
                        char packet_str[1024];
                        snprintf(packet_str, 1024, "%s\t%d\t%d", segments[i].packet.msg, segments[i].packet.seq, segments[i].packet.ack);
                        sendto(sockfd, packet_str, strlen(packet_str), 0, (struct sockaddr *) &addr, sizeof(addr));
                        printf("[+] Sent: %s\n", packet_str);
                    }
                }

                // Wait for an ACK with a timeout
                struct pollfd fds[1];
                fds[0].fd = sockfd;
                fds[0].events = POLLIN;
                int poll_res = poll(fds, 1, timeout);

                if (poll_res > 0) { // We received something
                    char ack_buffer[1024];
                    socklen_t addr_size = sizeof(addr);
                    recvfrom(sockfd, ack_buffer, sizeof(ack_buffer) - 1, 0, (struct sockaddr *) &addr, &addr_size);
                    printf("[+] Recv ACK: %s\n", ack_buffer);

                    // Parse the ACK
                    Data ack_packet;
                    Initialize(&ack_packet);
                    sscanf(ack_buffer, "%*s\t%d\t%d", &ack_packet.seq, &ack_packet.ack);

                    // Update acknowledged status for all packets confirmed by this ACK
                    int newly_acked = 0;
                    for (int i = 0; i < total_chunks; i++) {
                        if (!segments[i].acknowledged && segments[i].packet.seq <= ack_packet.ack) {
                            segments[i].acknowledged = true;
                            newly_acked++;
                        }
                    }
                    if (newly_acked > 0) {
                        chunks_acked += newly_acked;
                        printf("--- %d chunks acknowledged. Total acked: %d/%d ---\n", newly_acked, chunks_acked, total_chunks);
                    }
                } else { // Timeout
                    printf("--- Timeout! Retransmitting unacknowledged packets. ---\n");
                }
            }

            // 3. Send termination message
            char final_msg[1024];
            snprintf(final_msg, 1024, "N=%d\t%d\t%d", total_chunks, current_seq + 1, 0);
            for (int i=0; i<3; i++) { // Send it a few times to ensure delivery
                sendto(sockfd, final_msg, strlen(final_msg), 0, (struct sockaddr *) &addr, sizeof(addr));
                usleep(10000); // 10ms delay
            }
            printf("[+] Sent termination message.\n");
            close(sockfd);
        }
        // RECEIVER LOGIC
        else if(option == 0){
            int port = atoi(argv[1]);
            int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
            struct sockaddr_in server_addr, client_addr;

            memset(&server_addr, '\0', sizeof(server_addr));
            server_addr.sin_family = AF_INET;
            server_addr.sin_port = htons(port);
            server_addr.sin_addr.s_addr = INADDR_ANY;
            bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr));
            printf("Receiver is listening on port %d...\n", port);

            Data received_chunks[2048] = {0};
            bool chunk_received[2048] = {false};
            int total_chunks = -1;
            int chunks_count = 0;

            while(total_chunks == -1 || chunks_count < total_chunks) {
                char input[1024];
                bzero(input, 1024);
                socklen_t addr_size = sizeof(client_addr);
                recvfrom(sockfd, input, sizeof(input) - 1, 0, (struct sockaddr *) &client_addr, &addr_size);
                
                Data rec_packet;
                Initialize(&rec_packet);
                char temp_msg[buffsize + 1] = {0};
                
                // Parse the received packet
                char* token = strtok(input, "\t");
                if (token) strncpy(temp_msg, token, buffsize);
                token = strtok(NULL, "\t");
                if (token) rec_packet.seq = atoi(token);
                
                // Check for termination message
                if (strncmp(temp_msg, "N=", 2) == 0) {
                    total_chunks = atoi(temp_msg + 2);
                    printf("[+] Termination message received. Expecting %d total chunks.\n", total_chunks);
                } else {
                    strcpy(rec_packet.msg, temp_msg);
                    int chunk_index = (rec_packet.seq - 1) / buffsize;
                    
                    if (chunk_index >= 0 && chunk_index < 2048 && !chunk_received[chunk_index]) {
                        received_chunks[chunk_index] = rec_packet;
                        chunk_received[chunk_index] = true;
                        chunks_count++;
                        printf("[+] Recv: %s (seq: %d). Stored at index %d. Total: %d\n", rec_packet.msg, rec_packet.seq, chunk_index, chunks_count);
                    } else {
                        printf("[+] Recv Duplicate: %s (seq: %d). Ignoring.\n", rec_packet.msg, rec_packet.seq);
                    }
                }

                // Send ACK for the highest contiguous sequence number received
                int ack_seq = 0;
                for (int i = 0; i < 2048; i++) {
                    if (chunk_received[i]) {
                        ack_seq = received_chunks[i].seq;
                    } else {
                        break; // Found a gap, so we can't ACK beyond this
                    }
                }
                
                if (RETRANSMISSION_CHECK) { usleep(500000); } // Artificial delay

                char ack_str[1024];
                snprintf(ack_str, 1024, "Ack\t%d\t%d", 0, ack_seq);
                sendto(sockfd, ack_str, strlen(ack_str), 0, (struct sockaddr *) &client_addr, sizeof(client_addr));
                printf("[+] Sent ACK for seq up to %d\n", ack_seq);
            }

            printf("\n--- Transmission Complete ---\nFinal output: ");
            for(int i = 0; i < total_chunks; i++){
                printf("%s", received_chunks[i].msg);
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