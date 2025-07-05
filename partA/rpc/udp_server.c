#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

void rps_compare(char buffer[3][1024]){
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

int main(int argc, char **argv){

    if (argc != 2){
        printf("Usage: %s <port>\n", argv[0]);
        exit(0);
    }

    char *ip = "127.0.0.1";
    int port = atoi(argv[1]);
    int rps = 0;

    int sockfd;
    struct sockaddr_in server_addr, client_addr[2];
    char buffer[3][1024];
    socklen_t addr_size;
    int n;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0){
        perror("[-]socket error");
        exit(1);
    }

    memset(&server_addr, '\0', sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip);

    n = bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (n < 0) {
        perror("[-]bind error");
        exit(1);
    }

    while(1){
        for(int i = 0; i < 2; i++){
            bzero(buffer[i], 1024);
            addr_size = sizeof(client_addr[i]);
            if (recvfrom(sockfd, buffer[i], 1024, 0, (struct sockaddr *) &client_addr[i], &addr_size) < 0) {
                perror("recvfrom");
                exit(1);
            }
            printf("[+]Data recv %d: %s\n", i+1, buffer[i]);
        }

        if(rps == 1){
            rps_compare(buffer);
            rps = 0;
        }
        else if (strcmp(buffer[0], "Hello!") == 0 && strcmp(buffer[1], "Hello!") == 0) {
            bzero(buffer[0], 1024);
            strcpy(buffer[0], "Hello! lets begin. choose r/p/s");
            bzero(buffer[1], 1024);
            strcpy(buffer[1], "Hello! lets begin. choose r/p/s");
            rps = 1;
        }
        else{
            if(strcmp(buffer[0], "y") == 0 && strcmp(buffer[1], "y") == 0){
                bzero(buffer[0], 1024);
                strcpy(buffer[0], "Okay then, Lets play again!");
                bzero(buffer[1], 1024);
                strcpy(buffer[1], "Okay then, Lets play again!");
            }
            else {
                bzero(buffer[0], 1024);
                strcpy(buffer[0], "Okay then, Goodbye!");
                bzero(buffer[1], 1024);
                strcpy(buffer[1], "Okay then, Goodbye!");
            }
            rps = 1;
        }

        for(int i = 0; i < 2; i++){
//            bzero(buffer[i], 1024);
//            strcpy(buffer[i], "Welcome to the UDP Server.");
            if (sendto(sockfd, buffer[i], 1024, 0, (struct sockaddr *) &client_addr[i], sizeof(client_addr[i])) < 0) {
                perror("sendto");
                exit(1);
            }
            printf("[+]Data send%d: %s\n", i+1, buffer[i]);
        }
    }
    return 0;
}