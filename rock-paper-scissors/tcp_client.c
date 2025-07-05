#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

// Helper function to remove trailing newline from fgets
void remove_newline(char *str) {
    str[strcspn(str, "\n")] = 0;
}

int main(){
    char *ip = "127.0.0.1";
    int port = 5566;

    int sock;
    struct sockaddr_in addr;
    char buffer[1024];

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0){
        perror("[-]Socket error");
        exit(1);
    }
    printf("[+]TCP client socket created.\n");

    memset(&addr, '\0', sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip);

    if(connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("[-]Connection error");
        exit(1);
    }
    printf("Connected to the server.\n");

    // Start a separate thread or process for receiving messages to avoid blocking
    // For simplicity, this client will alternate sending and receiving.
    while(1){
        // Receive message from server
        bzero(buffer, 1024);
        int bytes_received = recv(sock, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received <= 0) {
            printf("Server disconnected or error occurred.\n");
            break;
        }
        printf("Server: %s\n", buffer);

        // Check for terminal messages from server
        if(strcmp(buffer, "Okay then, goodbye!") == 0){
            break;
        }
        
        // Get user input safely
        bzero(buffer, 1024);
        printf("Your move > ");
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            // Handle Ctrl+D or input error
            break;
        }
        remove_newline(buffer); // Remove trailing newline from fgets

        // Send the user's input to the server
        if (send(sock, buffer, strlen(buffer), 0) < 0) {
            perror("[-]Send error");
            break;
        }
    }
    
    close(sock);
    printf("Disconnected from the server.\n");
    return 0;
}