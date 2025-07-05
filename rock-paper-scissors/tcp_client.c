#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int main(){

    // Server IP address and port
    char *ip = "127.0.0.1";
    int port = 5566;

    int sock;
    struct sockaddr_in addr;
    char buffer[1024];

    // Create a TCP socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0){
        perror("[-]Socket error");
        exit(1);
    }
    printf("[+]TCP server socket created.\n");

    // Zero out the server address structure
    memset(&addr, '\0', sizeof(addr));
    // Set address family, port, and IP
    addr.sin_family = AF_INET;
    addr.sin_port = port;
    addr.sin_addr.s_addr = inet_addr(ip);

    // Connect to the server
    if(connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("[-]Connection error");
        exit(1);
    }
    printf("Connected to the server.\n");

    // Main loop for game interaction
    while(1){
        // Check if the server sent the goodbye message
        if(strcmp(buffer, "Okay then, goodbye!") == 0){
            break; // Exit the loop if the game is over
        }
        
        // Clear the buffer and get user input (r, p, s, or y)
        bzero(buffer, 1024);
        scanf("%s", buffer);
        printf("Client: %s\n", buffer);

        // Send the user's input to the server
        // MSG_NOSIGNAL prevents the program from crashing if the server disconnects
        if ((send(sock, buffer, strlen(buffer), 0 | MSG_NOSIGNAL)) < 0) {
            perror("[-]Send error");
            exit(1);
        }

        // Clear the buffer and wait for the server's response
        bzero(buffer, 1024);
        if (recv(sock, buffer, sizeof(buffer), 0 | MSG_NOSIGNAL) < 0) {
            perror("[-]Recv error");
            exit(1);
        }
        printf("Server: %s\n", buffer);
        
        // Check for a quit signal from the server (not used in current server logic but good practice)
        if(strcmp(buffer, "q") == 0){
            break;
        }
        // If the response is a game result, prompt the user to play again
        else if(strncmp(buffer, "WIN", 3) == 0 || strncmp(buffer, "LOSE", 4) == 0 || strncmp(buffer, "DRAW", 4) == 0
                || strncmp(buffer, "invalid response: LOSE", 22) == 0){
            bzero(buffer, 1024);
            printf("Server: Play again?? (y/n)\n");
        }
    }
    
    // Close the socket
    close(sock);
    printf("Disconnected from the server.\n");

    return 0;
}