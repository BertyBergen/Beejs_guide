/*
** talker.c -- a datagram "client" demo
*/

#include <stdio.h>              // стандартный ввод/вывод
#include <stdlib.h>             // для exit(), malloc() и прочего
#include <unistd.h>             // для close()
#include <errno.h>              // для errno и perror()
#include <string.h>             // для memset(), strlen()
#include <sys/types.h>          // общие определения типов
#include <sys/socket.h>         // функции и структуры для сокетов
#include <netinet/in.h>         // для sockaddr_in и прочего (IPv4)
#include <arpa/inet.h>          // для inet_ntop() и других
#include <netdb.h>              // для getaddrinfo()

#define SERVERPORT "4950"       // порт, на который отправляем данные

int main(int argc, char *argv[])
{
    int sockfd;                 // файловый дескриптор сокета
    struct addrinfo hints;      // настройки для getaddrinfo()
    struct addrinfo *servinfo;  // список полученных адресов
    struct addrinfo *p;         // итератор по списку адресов
    int rv;                     // для хранения кода возврата функций
    int numbytes;               // количество отправленных байт

    if (argc != 3) {            // проверка количества аргументов
        fprintf(stderr,"usage: talker hostname message\n");  // если не 3, выводим ошибку
        exit(1);                // и выходим с ошибкой
    }

    memset(&hints, 0, sizeof hints);     // обнуляем структуру hints
    hints.ai_family = AF_INET6;          // указываем использовать IPv6 (можно AF_INET для IPv4)
    hints.ai_socktype = SOCK_DGRAM;      // указываем тип сокета — датаграмма (UDP)

    rv = getaddrinfo(argv[1], SERVERPORT, &hints, &servinfo); // получаем адреса по имени хоста и порту
    if (rv != 0) {                     // если ошибка
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv)); // выводим сообщение об ошибке
        return 1;                      // выходим
    }

    // Проходим по всем найденным адресам и пытаемся создать сокет
    for(p = servinfo; p != NULL; p = p->ai_next) {
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol); // создаём сокет
        if (sockfd == -1) {          // если ошибка
            perror("talker: socket"); // выводим ошибку
            continue;                 // пробуем следующий адрес
        }
        break;                        // если сокет создан — выходим из цикла
    }

    if (p == NULL) {                  // если не удалось создать сокет ни с одним адресом
        fprintf(stderr, "talker: failed to create socket\n"); // выводим ошибку
        return 2;                     // выходим
    }

    // Отправляем сообщение на адрес p->ai_addr через сокет sockfd
    numbytes = sendto(
        sockfd,                       // сокет
        argv[2],                      // сообщение
        strlen(argv[2]),             // длина сообщения
        0,                            // флаги (обычно 0)
        p->ai_addr,                   // адрес получателя
        p->ai_addrlen                 // длина структуры адреса
    );
    if (numbytes == -1) {            // если ошибка при отправке
        perror("talker: sendto");    // выводим ошибку
        exit(1);                     // выходим
    }

    freeaddrinfo(servinfo);          // освобождаем список адресов

    printf("talker: sent %d bytes to %s\n", numbytes, argv[1]); // выводим, сколько байт отправили

    close(sockfd);                   // закрываем сокет

    return 0;                        // успешный выход
}
