#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <poll.h>
#include <errno.h>
#include <stdbool.h>

void rps_compare(char buffer[100][1024]){
    if(strcmp(buffer[0], "r") == 0){
        if(strcmp(buffer[1], "r") == 0){
            bzero(buffer[0], 1024);
            bzero(buffer[1], 1024);
            strcpy(buffer[0], "DRAW");
            strcpy(buffer[1], "DRAW");
        }
        else if(strcmp(buffer[1], "p") == 0){
            bzero(buffer[0], 1024);
            bzero(buffer[1], 1024);
            strcpy(buffer[0], "LOSE");
            strcpy(buffer[1], "WIN");
        }
        else if(strcmp(buffer[1], "s") == 0){
            bzero(buffer[0], 1024);
            bzero(buffer[1], 1024);
            strcpy(buffer[0], "WIN");
            strcpy(buffer[1], "LOSE");
        }
        else{
            bzero(buffer[0], 1024);
            bzero(buffer[1], 1024);
            strcpy(buffer[1], "invalid response: LOSE");
            strcpy(buffer[0], "WIN");
        }
    }
    else if(strcmp(buffer[0], "p") == 0){
        if(strcmp(buffer[1], "p") == 0){
            bzero(buffer[0], 1024);
            bzero(buffer[1], 1024);
            strcpy(buffer[0], "DRAW");
            strcpy(buffer[1], "DRAW");
        }
        else if(strcmp(buffer[1], "s") == 0){
            bzero(buffer[0], 1024);
            bzero(buffer[1], 1024);
            strcpy(buffer[0], "LOSE");
            strcpy(buffer[1], "WIN");
        }
        else if(strcmp(buffer[1], "r") == 0){
            bzero(buffer[0], 1024);
            bzero(buffer[1], 1024);
            strcpy(buffer[0], "WIN");
            strcpy(buffer[1], "LOSE");
        }
        else{
            bzero(buffer[0], 1024);
            bzero(buffer[1], 1024);
            strcpy(buffer[1], "invalid response: LOSE");
            strcpy(buffer[0], "WIN");
        }
    }
    else if(strcmp(buffer[0], "s") == 0){
        if(strcmp(buffer[1], "s") == 0){
            bzero(buffer[0], 1024);
            bzero(buffer[1], 1024);
            strcpy(buffer[0], "DRAW");
            strcpy(buffer[1], "DRAW");
        }
        else if(strcmp(buffer[1], "r") == 0){
            bzero(buffer[0], 1024);
            bzero(buffer[1], 1024);
            strcpy(buffer[0], "LOSE");
            strcpy(buffer[1], "WIN");
        }
        else if(strcmp(buffer[1], "p") == 0){
            bzero(buffer[0], 1024);
            bzero(buffer[1], 1024);
            strcpy(buffer[0], "WIN");
            strcpy(buffer[1], "LOSE");
        }
        else{
            bzero(buffer[0], 1024);
            bzero(buffer[1], 1024);
            strcpy(buffer[1], "invalid response: LOSE");
            strcpy(buffer[0], "WIN");
        }
    }
    else if(strcmp(buffer[1], "s") == 0 || strcmp(buffer[1], "p") == 0 || strcmp(buffer[1], "r") == 0){
            bzero(buffer[0], 1024);
            bzero(buffer[1], 1024);
            strcpy(buffer[0], "invalid response: LOSE");
            strcpy(buffer[1], "WIN");
    }
    else{
//        printf("%s\t\t\t%s\n", buffer[0], buffer[1]);
        bzero(buffer[0], 1024);
        bzero(buffer[1], 1024);
        strcpy(buffer[0], "invalid response: LOSE");
        strcpy(buffer[1], "invalid response: LOSE");
    }
}

int main(){
    char *ip = "127.0.0.1";
    int port = 5566;

    int server_sock;
    int client_sock[100];
    int client_num = 0;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_size;
    char buffer[100][1024];
    int n;

    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0){
        perror("[-]Socket error");
        exit(1);
    }
    printf("[+]TCP server socket created.\n");

    memset(&server_addr, '\0', sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = port;
    server_addr.sin_addr.s_addr = inet_addr(ip);

    n = bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (n < 0){
        perror("[-]Bind error");
        exit(1);
    }
    printf("[+]Bind to the port number: %d\n", port);

    listen(server_sock, 5);
    printf("Listening...\n");

    bool another = false;
    while(1){
//        if(client_num >= 2){ printf("l\n"); }
        addr_size = sizeof(client_addr);
        struct pollfd fds[1];
        fds[0].fd = server_sock;
        fds[0].events = POLLIN;

        poll(fds, 1, 20);
        if(fds[0].revents & POLLIN) {
            client_sock[client_num++] = accept(server_sock, (struct sockaddr *) &client_addr, &addr_size);
            printf("[+]Client%d connected.\n", client_num);
            if(client_num == 2){
                printf("Ready! Lets play!");
            }
            continue;
        }
        if(client_num < 2){
            continue;
        }
        for(int i = 0; i < client_num; i++) {
            bzero(buffer[i], 1024);
            int c = recv(client_sock[i], buffer[i], sizeof(buffer[i]), MSG_NOSIGNAL);
            if (c < 0) {
                perror("recv");
                exit(1);
            }
            else
                printf("Client%d: %s\n", i + 1, buffer[i]);
        }
        if(another) {
            if (strcmp(buffer[0], buffer[1]) == 0 && strcmp(buffer[0], "y") == 0){
                another = false;
                strcpy(buffer[99], "Cool! lets play another!");
                printf("Server: %s\n", buffer[99]);
                if (send(client_sock[0], buffer[99], strlen(buffer[99]), 0 | MSG_NOSIGNAL) < 0) {
                    perror("send");
                    exit(1);
                }
                if (send(client_sock[1], buffer[99], strlen(buffer[99]), 0 | MSG_NOSIGNAL) < 0) {
                    perror("send");
                    exit(1);
                }
                continue;
            }
            else{
                printf("Server: Okay then, goodbye!\n");
                if (send(client_sock[0], "Okay then, goodbye!", 20, 0 | MSG_NOSIGNAL) < 0) {
                    perror("send");
                    exit(1);
                }
                if (send(client_sock[1], "Okay then, goodbye!", 20, 0 | MSG_NOSIGNAL) < 0) {
                    perror("send");
                    exit(1);
                }
                close(client_sock[0]);
                printf("[+]Client1 disconnected.\n\n");
                close(client_sock[1]);
                printf("[+]Client2 disconnected.\n\n");
                client_num = 0;
                another = false;
                continue;
            }
        }

        for(int i = 0 ; i < client_num; i++) {
            if (strcmp(buffer[i], "q") == 0) {
                bzero(buffer[i], 1024);
                strcpy(buffer[i], "q");
                printf("Server: %s\n", buffer[i]);
                if (send(client_sock[i], buffer[i], strlen(buffer[i]), 0 | MSG_NOSIGNAL) < 0) {
                    perror("send");
                    exit(1);
                }
                close(client_sock[i]);
                printf("[+]Client%d disconnected.\n\n", i + 1);
                client_num--;
            }
        }

        bzero(buffer[99], 1024);
        if(client_num > 1){
            rps_compare(buffer);
            strcpy(buffer[99], "Play again??");
            another = true;
            printf("Server: %s\n", buffer[99]);
            if (send(client_sock[0], buffer[0], strlen(buffer[0]), 0 | MSG_NOSIGNAL) < 0) {
                perror("send");
                exit(1);
            }
            if (send(client_sock[1], buffer[1], strlen(buffer[1]), 0 | MSG_NOSIGNAL) < 0) {
                perror("send");
                exit(1);
            }
        }
    }
    return 0;
}
