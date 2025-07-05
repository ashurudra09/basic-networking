#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <poll.h>
#include <errno.h>
#include <stdbool.h>

#define MAX_CLIENTS 2

// Function to compare the moves of two players and determine the outcome
void rps_compare(char buffer[MAX_CLIENTS][1024]){
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


// Function to reset the game state
void reset_game(int *client_sockets, int *client_count, bool *client_has_moved) {
    for (int i = 0; i < *client_count; i++) {
        if (client_sockets[i] != -1) {
            close(client_sockets[i]);
            printf("[+]Client on socket %d disconnected.\n", client_sockets[i]);
        }
        client_sockets[i] = -1;
    }
    *client_count = 0;
    client_has_moved[0] = false;
    client_has_moved[1] = false;
    printf("\n--- Game Reset: Waiting for 2 new players ---\n");
}

int main(){
    char *ip = "127.0.0.1";
    int port = 5566;

    int server_sock;
    int client_sockets[MAX_CLIENTS] = {-1, -1};
    int client_count = 0;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_size;
    char client_buffers[MAX_CLIENTS][1024];
    bool client_has_moved[MAX_CLIENTS] = {false, false};
    bool play_again_phase = false;

    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0){
        perror("[-]Socket error");
        exit(1);
    }
    printf("[+]TCP server socket created.\n");

    memset(&server_addr, '\0', sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip);

    if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
        perror("[-]Bind error");
        exit(1);
    }
    printf("[+]Bind to the port number: %d\n", port);

    listen(server_sock, 5);
    printf("Listening for connections...\n");

    struct pollfd fds[MAX_CLIENTS + 1];

    while(1){
        // Setup pollfd array
        fds[0].fd = server_sock;
        fds[0].events = POLLIN;
        int active_fds = 1;
        for (int i = 0; i < client_count; i++) {
            if (client_sockets[i] != -1) {
                fds[active_fds].fd = client_sockets[i];
                fds[active_fds].events = POLLIN;
                active_fds++;
            }
        }

        // Wait for activity on any socket
        int poll_count = poll(fds, active_fds, -1); // -1 for infinite timeout
        if (poll_count < 0) {
            perror("[-]Poll error");
            break;
        }

        // Check for new connections on the listening socket
        if (fds[0].revents & POLLIN) {
            if (client_count < MAX_CLIENTS) {
                addr_size = sizeof(client_addr);
                int new_sock = accept(server_sock, (struct sockaddr*)&client_addr, &addr_size);
                if (new_sock < 0) {
                    perror("[-]Accept error");
                } else {
                    client_sockets[client_count] = new_sock;
                    client_count++;
                    printf("[+]Client %d connected on socket %d.\n", client_count, new_sock);
                    if (client_count == MAX_CLIENTS) {
                        printf("--- Two players connected! Starting game. ---\n");
                        char *start_msg = "Game start! Choose rock (r), paper (p), or scissors (s):";
                        send(client_sockets[0], start_msg, strlen(start_msg), 0);
                        send(client_sockets[1], start_msg, strlen(start_msg), 0);
                    }
                }
            }
        }

        // Check for data from connected clients
        for (int i = 1; i < active_fds; i++) {
            if (fds[i].revents & POLLIN) {
                int client_index = -1;
                for (int j = 0; j < client_count; j++) {
                    if (client_sockets[j] == fds[i].fd) {
                        client_index = j;
                        break;
                    }
                }

                if (client_index != -1) {
                    bzero(client_buffers[client_index], 1024);
                    int bytes_received = recv(client_sockets[client_index], client_buffers[client_index], 1023, 0);

                    if (bytes_received <= 0) { // Client disconnected or error
                        printf("[-]Client %d on socket %d disconnected or had an error.\n", client_index + 1, client_sockets[client_index]);
                        reset_game(client_sockets, &client_count, client_has_moved);
                        play_again_phase = false;
                        break; // Restart poll loop
                    }

                    printf("Client %d: %s\n", client_index + 1, client_buffers[client_index]);
                    client_has_moved[client_index] = true;
                }
            }
        }

        // If we have 2 clients and both have made a move, process the game logic
        if (client_count == MAX_CLIENTS && client_has_moved[0] && client_has_moved[1]) {
            if (play_again_phase) {
                if (strcmp(client_buffers[0], "y") == 0 && strcmp(client_buffers[1], "y") == 0) {
                    char *next_round_msg = "Next round! Choose rock (r), paper (p), or scissors (s):";
                    send(client_sockets[0], next_round_msg, strlen(next_round_msg), 0);
                    send(client_sockets[1], next_round_msg, strlen(next_round_msg), 0);
                } else {
                    char *goodbye_msg = "Okay then, goodbye!";
                    send(client_sockets[0], goodbye_msg, strlen(goodbye_msg), 0);
                    send(client_sockets[1], goodbye_msg, strlen(goodbye_msg), 0);
                    reset_game(client_sockets, &client_count, client_has_moved);
                }
                play_again_phase = false;
            } else {
                rps_compare(client_buffers);
                send(client_sockets[0], client_buffers[0], strlen(client_buffers[0]), 0);
                send(client_sockets[1], client_buffers[1], strlen(client_buffers[1]), 0);

                char *play_again_msg = "Play again? (y/n)";
                send(client_sockets[0], play_again_msg, strlen(play_again_msg), 0);
                send(client_sockets[1], play_again_msg, strlen(play_again_msg), 0);
                play_again_phase = true;
            }

            // Reset move flags for the next round
            client_has_moved[0] = false;
            client_has_moved[1] = false;
        }
    }
    close(server_sock);
    return 0;
}