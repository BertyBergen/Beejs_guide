#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>

int main()
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serv =  { .sin_family = AF_INET, .sin_addr.s_addr = INADDR_ANY, .sin_port = htons(8080)};
    bind(sock, (struct sockaddr*)&serv, sizeof(serv));// Функция bind() привязывает сокет к определённому адресу (IP + порт). Ее прототип
    //int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
    //sockfd - descriptor
    // addr — указатель на структуру с адресом, к которому хочешь привязать сокет.
    // addrlen - size of struct
    listen(sock, 1); // server side only Перевести в режим "Жду клиентов"
    int client = accept(sock, NULL, NULL); // снимает 1 клиента с очереди. Принять подключение 
    char buf[1024];
    int n = read(client, buf, 1023);
    buf[n] = '\0';
    printf("Received: %s \n", buf);
    close(client); close(sock);
    return 0;

}