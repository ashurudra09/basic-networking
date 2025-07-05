#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// Function to compare moves, identical to the TCP server's version.
void rps_compare(char buffer[3][1024]){
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

int main(int argc, char **argv){
    if (argc != 2){
        printf("Usage: %s <port>\n", argv[0]);
        exit(0);
    }

    char *ip = "127.0.0.1";
    int port = atoi(argv[1]);
    int rps = 0; // State flag: 0 for initial contact/play again, 1 for RPS move

    int sockfd;
    struct sockaddr_in server_addr, client_addr[2]; // Store addresses for two clients
    char buffer[3][1024]; // Buffers for messages from two clients
    socklen_t addr_size;

    // Create UDP socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0){
        perror("[-]socket error");
        exit(1);
    }

    // Configure server address
    memset(&server_addr, '\0', sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip);

    // Bind socket to the address and port
    if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("[-]bind error");
        exit(1);
    }
    printf("UDP Server is listening on port %d...\n", port);

    while(1){
        // Sequentially wait for two packets. This is a blocking operation.
        // The server assumes two different clients will send packets one after another.
        for(int i = 0; i < 2; i++){
            bzero(buffer[i], 1024);
            addr_size = sizeof(client_addr[i]);
            if (recvfrom(sockfd, buffer[i], 1024, 0, (struct sockaddr *) &client_addr[i], &addr_size) < 0) {
                perror("recvfrom");
                exit(1);
            }
            printf("[+]Data recv from client %d: %s\n", i+1, buffer[i]);
        }

        // If the state flag 'rps' is 1, it means the received packets are game moves
        if(rps == 1){
            rps_compare(buffer); // Determine the winner
            rps = 0; // Reset state to wait for "play again" response
        }
        // If this is the first contact from both clients
        else if (strcmp(buffer[0], "Hello!") == 0 && strcmp(buffer[1], "Hello!") == 0) {
            strcpy(buffer[0], "Hello! lets begin. choose r/p/s");
            strcpy(buffer[1], "Hello! lets begin. choose r/p/s");
            rps = 1; // Set state to expect game moves next
        }
        // Otherwise, the packets are responses to "play again?"
        else{
            // If both want to play again
            if(strcmp(buffer[0], "y") == 0 && strcmp(buffer[1], "y") == 0){
                strcpy(buffer[0], "Okay then, Lets play again!");
                strcpy(buffer[1], "Okay then, Lets play again!");
            }
            // If at least one does not want to play again
            else {
                strcpy(buffer[0], "Okay then, Goodbye!");
                strcpy(buffer[1], "Okay then, Goodbye!");
            }
            rps = 1; // Set state to expect game moves next (or clients will exit)
        }

        // Send the appropriate response back to each client using their stored address
        for(int i = 0; i < 2; i++){
            if (sendto(sockfd, buffer[i], 1024, 0, (struct sockaddr *) &client_addr[i], sizeof(client_addr[i])) < 0) {
                perror("sendto");
                exit(1);
            }
            printf("[+]Data send to client %d: %s\n", i+1, buffer[i]);
        }
    }
    return 0;
}