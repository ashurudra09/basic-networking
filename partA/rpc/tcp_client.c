#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int main(){

    char *ip = "127.0.0.1";
    int port = 5566;

    int sock;
    struct sockaddr_in addr;
    socklen_t addr_size;
    char buffer[1024];
    int n;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0){
        perror("[-]Socket error");
        exit(1);
    }
    printf("[+]TCP server socket created.\n");

    memset(&addr, '\0', sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = port;
    addr.sin_addr.s_addr = inet_addr(ip);

    if(connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("connection");
        exit(1);
    }
    printf("Connected to the server.\n");

    while(1){
//        printf("l\n");
        if(strcmp(buffer, "Okay then, goodbye!") == 0){
            break;
        }
        bzero(buffer, 1024);
        scanf("%s", buffer);
        printf("Client: %s\n", buffer);

        if ((send(sock, buffer, strlen(buffer), 0 | MSG_NOSIGNAL)) < 0) {
            perror("send");
            exit(1);
        }

        bzero(buffer, 1024);
        if (recv(sock, buffer, sizeof(buffer), 0 | MSG_NOSIGNAL) < 0) {
            perror("recv");
            exit(1);
        }
        printf("Server: %s\n", buffer);
        if(strcmp(buffer, "q") == 0){
            break;
        }
        else if(strncmp(buffer, "WIN", 3) == 0 || strncmp(buffer, "LOSE", 4) == 0 || strncmp(buffer, "DRAW", 4) == 0
                || strncmp(buffer, "invalid response: LOSE", 22) == 0){
            bzero(buffer, 1024);
            printf("Server: Play again??\n");
        }
    }
    close(sock);
    printf("Disconnected from the server.\n");

    return 0;

}