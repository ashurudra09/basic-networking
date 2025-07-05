#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdbool.h>

#define MAX_CLIENTS 2

// (rps_compare function is identical to TCP version, can be included here)
void rps_compare(char buffer[MAX_CLIENTS][1024]){
    if(strcmp(buffer[0], "r") == 0){
        if(strcmp(buffer[1], "r") == 0){ strcpy(buffer[0], "DRAW"); strcpy(buffer[1], "DRAW"); }
        else if(strcmp(buffer[1], "p") == 0){ strcpy(buffer[0], "LOSE"); strcpy(buffer[1], "WIN"); }
        else if(strcmp(buffer[1], "s") == 0){ strcpy(buffer[0], "WIN"); strcpy(buffer[1], "LOSE"); }
        else{ strcpy(buffer[0], "WIN"); strcpy(buffer[1], "invalid response: LOSE"); }
    }
    else if(strcmp(buffer[0], "p") == 0){
        if(strcmp(buffer[1], "p") == 0){ strcpy(buffer[0], "DRAW"); strcpy(buffer[1], "DRAW"); }
        else if(strcmp(buffer[1], "s") == 0){ strcpy(buffer[0], "LOSE"); strcpy(buffer[1], "WIN"); }
        else if(strcmp(buffer[1], "r") == 0){ strcpy(buffer[0], "WIN"); strcpy(buffer[1], "LOSE"); }
        else{ strcpy(buffer[0], "WIN"); strcpy(buffer[1], "invalid response: LOSE"); }
    }
    else if(strcmp(buffer[0], "s") == 0){
        if(strcmp(buffer[1], "s") == 0){ strcpy(buffer[0], "DRAW"); strcpy(buffer[1], "DRAW"); }
        else if(strcmp(buffer[1], "r") == 0){ strcpy(buffer[0], "LOSE"); strcpy(buffer[1], "WIN"); }
        else if(strcmp(buffer[1], "p") == 0){ strcpy(buffer[0], "WIN"); strcpy(buffer[1], "LOSE"); }
        else{ strcpy(buffer[0], "WIN"); strcpy(buffer[1], "invalid response: LOSE"); }
    }
    else if(strcmp(buffer[1], "s") == 0 || strcmp(buffer[1], "p") == 0 || strcmp(buffer[1], "r") == 0){
        strcpy(buffer[0], "invalid response: LOSE");
        strcpy(buffer[1], "WIN");
    }
    else{
        strcpy(buffer[0], "invalid response: LOSE");
        strcpy(buffer[1], "invalid response: LOSE");
    }
}

// Function to check if two sockaddr_in are the same
bool is_same_addr(struct sockaddr_in *addr1, struct sockaddr_in *addr2) {
    return addr1->sin_addr.s_addr == addr2->sin_addr.s_addr &&
           addr1->sin_port == addr2->sin_port;
}

int main(int argc, char **argv){
    if (argc != 2){
        printf("Usage: %s <port>\n", argv[0]);
        exit(0);
    }

    char *ip = "127.0.0.1";
    int port = atoi(argv[1]);

    int sockfd;
    struct sockaddr_in server_addr, client_addrs[MAX_CLIENTS];
    struct sockaddr_in temp_addr; // To receive incoming packet's address
    char client_buffers[MAX_CLIENTS][1024];
    bool client_has_moved[MAX_CLIENTS] = {false, false};
    int client_count = 0;
    bool play_again_phase = false;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0){
        perror("[-]socket error");
        exit(1);
    }

    memset(&server_addr, '\0', sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip);

    if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("[-]bind error");
        exit(1);
    }
    printf("UDP Server is listening on port %d...\n", port);

    while(1){
        // Wait until we have two clients and both have sent a move
        while (!(client_count == MAX_CLIENTS && client_has_moved[0] && client_has_moved[1])) {
            char temp_buffer[1024];
            bzero(temp_buffer, 1024);
            socklen_t addr_size = sizeof(temp_addr);
            
            int bytes_received = recvfrom(sockfd, temp_buffer, 1023, 0, (struct sockaddr *)&temp_addr, &addr_size);
            if (bytes_received < 0) {
                perror("recvfrom error");
                continue;
            }

            int client_index = -1;
            // Check if this client is already known
            for (int i = 0; i < client_count; i++) {
                if (is_same_addr(&client_addrs[i], &temp_addr)) {
                    client_index = i;
                    break;
                }
            }

            // If it's a new client and we have space
            if (client_index == -1 && client_count < MAX_CLIENTS) {
                client_index = client_count;
                client_addrs[client_index] = temp_addr;
                client_count++;
                printf("[+] New client %d registered.\n", client_count);
            }

            // If we know the client, process their move
            if (client_index != -1) {
                strcpy(client_buffers[client_index], temp_buffer);
                client_has_moved[client_index] = true;
                printf("[+] Received from client %d: %s\n", client_index + 1, client_buffers[client_index]);
            }
        }

        // --- Game Logic Processing ---
        if (play_again_phase) {
            if (strcmp(client_buffers[0], "y") == 0 && strcmp(client_buffers[1], "y") == 0) {
                char *msg = "Next round! Choose r/p/s:";
                sendto(sockfd, msg, strlen(msg), 0, (struct sockaddr *)&client_addrs[0], sizeof(client_addrs[0]));
                sendto(sockfd, msg, strlen(msg), 0, (struct sockaddr *)&client_addrs[1], sizeof(client_addrs[1]));
            } else {
                char *msg = "Okay then, goodbye!";
                sendto(sockfd, msg, strlen(msg), 0, (struct sockaddr *)&client_addrs[0], sizeof(client_addrs[0]));
                sendto(sockfd, msg, strlen(msg), 0, (struct sockaddr *)&client_addrs[1], sizeof(client_addrs[1]));
                // Reset for new players
                client_count = 0;
            }
            play_again_phase = false;
        } else {
            rps_compare(client_buffers);
            sendto(sockfd, client_buffers[0], strlen(client_buffers[0]), 0, (struct sockaddr *)&client_addrs[0], sizeof(client_addrs[0]));
            sendto(sockfd, client_buffers[1], strlen(client_buffers[1]), 0, (struct sockaddr *)&client_addrs[1], sizeof(client_addrs[1]));
            
            char *msg = "Play again? (y/n)";
            sendto(sockfd, msg, strlen(msg), 0, (struct sockaddr *)&client_addrs[0], sizeof(client_addrs[0]));
            sendto(sockfd, msg, strlen(msg), 0, (struct sockaddr *)&client_addrs[1], sizeof(client_addrs[1]));
            play_again_phase = true;
        }

        // Reset move flags for the next round
        client_has_moved[0] = false;
        client_has_moved[1] = false;
    }
    return 0;
}