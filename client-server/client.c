/*
** client.c -- демонстрация клиента с потоковым сокетом
*/

#include <stdio.h>      // для printf(), fprintf()
#include <stdlib.h>     // для exit()
#include <unistd.h>     // для close()
#include <errno.h>      // для errno и perror()
#include <string.h>     // для memset()
#include <netdb.h>      // для getaddrinfo()
#include <sys/types.h>  // для типов сокетов
#include <netinet/in.h> // для sockaddr_in
#include <sys/socket.h> // для socket(), connect()
#include <arpa/inet.h>  // для inet_ntop()

#define PORT "3490"          // порт, к которому подключается клиент
#define MAXDATASIZE 100      // макс. кол-во байт за раз, которые можно получить

// Получает указатель на IP-адрес (IPv4 или IPv6)
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        // IPv4
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    // IPv6
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[])
{
    int sockfd, numbytes;                    // файловый дескриптор сокета, кол-во полученных байт
    char buf[MAXDATASIZE];                  // буфер для приёма данных
    struct addrinfo hints, *servinfo, *p;   // hints — настройки, servinfo — список адресов, p — текущий элемент
    int rv;
    char s[INET6_ADDRSTRLEN];               // строка для IP-адреса

    if (argc != 2) {
        fprintf(stderr,"usage: client hostname\n");
        exit(1);
    }

    // Подготовка структуры hints
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;      // IPv4 или IPv6
    hints.ai_socktype = SOCK_STREAM;  // потоковый сокет (TCP)

    // Получение списка адресов сервера
    if ((rv = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // Перебираем адреса и пытаемся подключиться к первому подходящему
    for(p = servinfo; p != NULL; p = p->ai_next) {
        // Создаем сокет
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }

        // Преобразуем IP-адрес в строку и выводим
        inet_ntop(p->ai_family,
            get_in_addr((struct sockaddr *)p->ai_addr),
            s, sizeof s);
        printf("client: attempting connection to %s\n", s);

        // Пытаемся подключиться
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            perror("client: connect");
            close(sockfd);
            continue;
        }

        break;
    }

    // Если не удалось подключиться
    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }

    // Подтверждение успешного подключения
    inet_ntop(p->ai_family,
            get_in_addr((struct sockaddr *)p->ai_addr),
            s, sizeof s);
    printf("client: connected to %s\n", s);

    freeaddrinfo(servinfo); // Освобождаем память, выделенную getaddrinfo

    // Получение данных от сервера
    if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
        perror("recv");
        exit(1);
    }

    buf[numbytes] = '\0'; // Завершаем строку нулём

    printf("client: received '%s'\n", buf); // Вывод полученного сообщения

    close(sockfd); // Закрываем сокет

    return 0;
}
