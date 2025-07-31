#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>


int main()
{
    int sock = socket(AF_INET, SOCK_STREAM, 0); //int socket(int domain, int type, int protocol);
    struct sockaddr_in server = {.sin_family = AF_INET, .sin_addr.s_addr = inet_addr("127.0.0.1"), .sin_port = htons(12345)};
    bind(sock, (struct sockaddr*)&server, sizeof(server));//int bind(int __fd, const struct sockaddr *__addr, socklen_t __len)
    listen(sock, 1);
    int client = accept(sock, NULL, NULL);   
    char buff[1024];
    int n = read(client, buff, 1023); 
    buff[n] = '\0';
    printf("Received: %s \n", buff);
    
    char *message = "HELLO PIDOR";
    send(client, message, strlen(message), 0);
    close(client); close(sock);
    
    return 0;

}