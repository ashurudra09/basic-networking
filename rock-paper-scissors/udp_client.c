#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(int argc, char **argv){
    if (argc != 2) {
        printf("Usage: %s <port>\n", argv[0]);
        exit(0);
    }

    char *ip = "127.0.0.1";
    int port = atoi(argv[1]);

    int sockfd;
    struct sockaddr_in addr; // Server address structure
    char buffer[1024];
    socklen_t addr_size;

    // Create UDP socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    
    // Configure server address
    memset(&addr, '\0', sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip);

    // Send an initial "Hello!" message to register with the server
    bzero(buffer, 1024);
    strcpy(buffer, "Hello!");
    if (sendto(sockfd, buffer, 1024, 0, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
        perror("sendto");
        exit(1);
    }
    printf("[+]Data send: %s\n", buffer);

    // Main game loop
    while(1){
        // Wait for a response from the server (blocking)
        bzero(buffer, 1024);
        addr_size = sizeof(addr);
        if (recvfrom(sockfd, buffer, 1024, 0, (struct sockaddr *) &addr, &addr_size) < 0) {
            perror("recvfrom");
            exit(1);
        }
        printf("[+]Data recv: %s\n", buffer);

        // If server sends goodbye message, exit
        if(strcmp(buffer, "Okay then, Goodbye!") == 0){
            printf("Server ended the game. Exiting.\n");
            exit(0);
        }
        // If the server sent a game result, print the "Play again?" prompt
        else if(strcmp(buffer, "WIN") == 0 || strcmp(buffer, "LOSE") == 0 || strcmp(buffer, "DRAW") == 0
        || strcmp(buffer, "invalid response: LOSE") == 0){
            printf("[+]Server asks: Play again? (y/n)\n");
        }

        // Get user input (move or play again response)
        bzero(buffer, 1024);
        scanf("%s", buffer);
        
        // Send the input to the server
        if (sendto(sockfd, buffer, 1024, 0, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
            perror("sendto");
            exit(1);
        }
        printf("[+]Data send: %s\n", buffer);
    }

    return 0;
}