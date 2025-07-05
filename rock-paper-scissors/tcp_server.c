#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <poll.h>
#include <errno.h>
#include <stdbool.h>

// Function to compare the moves of two players and determine the outcome
void rps_compare(char buffer[100][1024]){
    // Check if player 1 chose 'r' (rock)
    if(strcmp(buffer[0], "r") == 0){
        if(strcmp(buffer[1], "r") == 0){ strcpy(buffer[0], "DRAW"); strcpy(buffer[1], "DRAW"); }
        else if(strcmp(buffer[1], "p") == 0){ strcpy(buffer[0], "LOSE"); strcpy(buffer[1], "WIN"); }
        else if(strcmp(buffer[1], "s") == 0){ strcpy(buffer[0], "WIN"); strcpy(buffer[1], "LOSE"); }
        else{ strcpy(buffer[0], "WIN"); strcpy(buffer[1], "invalid response: LOSE"); } // Player 2 invalid move
    }
    // Check if player 1 chose 'p' (paper)
    else if(strcmp(buffer[0], "p") == 0){
        if(strcmp(buffer[1], "p") == 0){ strcpy(buffer[0], "DRAW"); strcpy(buffer[1], "DRAW"); }
        else if(strcmp(buffer[1], "s") == 0){ strcpy(buffer[0], "LOSE"); strcpy(buffer[1], "WIN"); }
        else if(strcmp(buffer[1], "r") == 0){ strcpy(buffer[0], "WIN"); strcpy(buffer[1], "LOSE"); }
        else{ strcpy(buffer[0], "WIN"); strcpy(buffer[1], "invalid response: LOSE"); } // Player 2 invalid move
    }
    // Check if player 1 chose 's' (scissors)
    else if(strcmp(buffer[0], "s") == 0){
        if(strcmp(buffer[1], "s") == 0){ strcpy(buffer[0], "DRAW"); strcpy(buffer[1], "DRAW"); }
        else if(strcmp(buffer[1], "r") == 0){ strcpy(buffer[0], "LOSE"); strcpy(buffer[1], "WIN"); }
        else if(strcmp(buffer[1], "p") == 0){ strcpy(buffer[0], "WIN"); strcpy(buffer[1], "LOSE"); }
        else{ strcpy(buffer[0], "WIN"); strcpy(buffer[1], "invalid response: LOSE"); } // Player 2 invalid move
    }
    // Handle cases where player 1's move is invalid but player 2's is valid
    else if(strcmp(buffer[1], "s") == 0 || strcmp(buffer[1], "p") == 0 || strcmp(buffer[1], "r") == 0){
            strcpy(buffer[0], "invalid response: LOSE");
            strcpy(buffer[1], "WIN");
    }
    // Handle case where both players' moves are invalid
    else{
        strcpy(buffer[0], "invalid response: LOSE");
        strcpy(buffer[1], "invalid response: LOSE");
    }
}

int main(){
    char *ip = "127.0.0.1";
    int port = 5566;

    int server_sock;
    int client_sock[100]; // Array to hold client socket descriptors
    int client_num = 0;   // Counter for connected clients
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_size;
    char buffer[100][1024]; // Buffer to hold messages from clients

    // Create TCP socket
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0){
        perror("[-]Socket error");
        exit(1);
    }
    printf("[+]TCP server socket created.\n");

    // Configure server address
    memset(&server_addr, '\0', sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = port;
    server_addr.sin_addr.s_addr = inet_addr(ip);

    // Bind socket to the specified IP and port
    if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
        perror("[-]Bind error");
        exit(1);
    }
    printf("[+]Bind to the port number: %d\n", port);

    // Listen for incoming connections
    listen(server_sock, 5);
    printf("Listening...\n");

    bool another = false; // Flag to check if we are in the "play again?" phase
    while(1){
        // Use poll to check for new connections without blocking the main loop
        struct pollfd fds[1];
        fds[0].fd = server_sock;
        fds[0].events = POLLIN; // Check for incoming data (i.e., new connections)

        poll(fds, 1, 20); // Wait for 20ms for an event

        // If a new connection is pending, accept it
        if(fds[0].revents & POLLIN) {
            client_sock[client_num++] = accept(server_sock, (struct sockaddr *) &client_addr, &addr_size);
            printf("[+]Client%d connected.\n", client_num);
            if(client_num == 2){
                printf("Ready! Lets play!\n");
            }
            continue; // Go back to the start of the loop to check for more connections or start the game
        }
        
        // Do not proceed with the game until two clients are connected
        if(client_num < 2){
            continue;
        }
        
        // Receive messages from both clients (this part is blocking)
        for(int i = 0; i < client_num; i++) {
            bzero(buffer[i], 1024);
            int c = recv(client_sock[i], buffer[i], sizeof(buffer[i]), MSG_NOSIGNAL);
            if (c <= 0) { // If recv returns 0 or less, client has disconnected
                printf("[-]Client%d disconnected unexpectedly.\n", i + 1);
                close(client_sock[i]);
                // Simple reset logic: disconnect the other client and wait for new players
                close(client_sock[1-i]);
                client_num = 0;
                another = false;
                printf("Resetting game. Waiting for 2 new players.\n");
                break;
            }
            printf("Client%d: %s\n", i + 1, buffer[i]);
        }
        if (client_num < 2) continue; // If a client disconnected, restart the loop

        // Handle the "play again?" logic
        if(another) {
            // If both clients respond with 'y'
            if (strcmp(buffer[0], "y") == 0 && strcmp(buffer[1], "y") == 0){
                another = false; // Reset the flag for the next round
                char* next_round_msg = "Cool! lets play another!";
                printf("Server: %s\n", next_round_msg);
                send(client_sock[0], next_round_msg, strlen(next_round_msg), MSG_NOSIGNAL);
                send(client_sock[1], next_round_msg, strlen(next_round_msg), MSG_NOSIGNAL);
                continue; // Start the next round
            }
            // If either client does not respond with 'y'
            else{
                printf("Server: Okay then, goodbye!\n");
                send(client_sock[0], "Okay then, goodbye!", 20, MSG_NOSIGNAL);
                send(client_sock[1], "Okay then, goodbye!", 20, MSG_NOSIGNAL);
                // Disconnect both clients
                close(client_sock[0]);
                printf("[+]Client1 disconnected.\n\n");
                close(client_sock[1]);
                printf("[+]Client2 disconnected.\n\n");
                // Reset server state to wait for new players
                client_num = 0;
                another = false;
                continue;
            }
        }

        // Main game logic for a round
        if(client_num > 1){
            rps_compare(buffer); // Determine the winner
            
            // Send the results to each client
            send(client_sock[0], buffer[0], strlen(buffer[0]), MSG_NOSIGNAL);
            send(client_sock[1], buffer[1], strlen(buffer[1]), MSG_NOSIGNAL);
            
            // Set the flag to indicate the next input will be for "play again?"
            another = true;
            printf("Server: Round over. Asking players to play again.\n");
        }
    }
    return 0;
}