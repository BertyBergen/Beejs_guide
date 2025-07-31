/*
** listener.c -- a datagram sockets "server" demo
*/

#include <stdio.h>              // Стандартная библиотека ввода/вывода
#include <stdlib.h>             // Стандартная библиотека (exit, malloc, free и т.д.)
#include <unistd.h>             // Для close(), read(), write() и других POSIX функций
#include <errno.h>              // Для обработки ошибок (переменная errno)
#include <string.h>             // Для строковых функций, например memset()
#include <sys/types.h>          // Для определения типов данных, используемых в сокетах
#include <sys/socket.h>         // Основной заголовок для работы с сокетами
#include <netinet/in.h>         // Для структуры sockaddr_in (IPv4) и sockaddr_in6 (IPv6)
#include <arpa/inet.h>          // Для функций преобразования IP-адресов (inet_ntop и др.)
#include <netdb.h>              // Для getaddrinfo и связанных структур

#define MYPORT "4950"           // Порт, на котором будет слушать сервер

#define MAXBUFLEN 100           // Максимальная длина буфера для приёма данных

// Функция для получения указателя на IP-адрес из структуры sockaddr
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        // Если IPv4, вернуть указатель на поле sin_addr
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    // Иначе IPv6 — вернуть указатель на поле sin6_addr
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(void)
{
    int sockfd;                             // Дескриптор сокета
    struct addrinfo hints, *servinfo, *p;   // Подсказки и результаты для getaddrinfo
    int rv;                                 // Переменная для хранения кода возврата
    int numbytes;                           // Кол-во принятых байт
    struct sockaddr_storage their_addr;     // Хранит адрес клиента (любой версии IP)
    char buf[MAXBUFLEN];                    // Буфер для приёма данных
    socklen_t addr_len;                     // Длина адреса клиента
    char s[INET6_ADDRSTRLEN];               // Буфер для строки IP-адреса

    memset(&hints, 0, sizeof hints);        // Обнуляем структуру hints
    hints.ai_family = AF_INET6;             // AF_INET6 — использовать IPv6 (можно AF_INET для IPv4)
    hints.ai_socktype = SOCK_DGRAM;         // Указываем тип сокета — UDP
    hints.ai_flags = AI_PASSIVE;            // Указываем, что будем использовать любой локальный IP

    // Получаем информацию для создания и привязки сокета
    if ((rv = getaddrinfo(NULL, MYPORT, &hints, &servinfo)) != 0) {
        // Если ошибка — печатаем и выходим
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // Проходим по всем возможным адресам, пока не удастся создать и привязать сокет
    for(p = servinfo; p != NULL; p = p->ai_next) {
        // Создаём сокет
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("listener: socket"); // Сообщаем об ошибке
            continue;                   // Пробуем следующий адрес
        }

        // Пробуем привязать сокет к адресу
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);             // Закрываем сокет при неудаче
            perror("listener: bind");
            continue;                  // Пробуем следующий адрес
        }

        break; // Успешно — выходим из цикла
    }

    if (p == NULL) {
        // Если не удалось привязаться ни к одному адресу
        fprintf(stderr, "listener: failed to bind socket\n");
        return 2;
    }

    freeaddrinfo(servinfo); // Освобождаем память, выделенную getaddrinfo

    printf("listener: waiting to recvfrom...\n"); // Готов к приёму данных

    addr_len = sizeof their_addr; // Устанавливаем размер адреса клиента
    // Получаем данные от клиента (recvfrom блокируется до получения данных)
    if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0,
        (struct sockaddr *)&their_addr, &addr_len)) == -1) {
        perror("recvfrom"); // Если ошибка — сообщаем
        exit(1);            // Завершаем программу
    }

    // Выводим IP-адрес клиента, преобразованный в строку
    printf("listener: got packet from %s\n",
        inet_ntop(their_addr.ss_family,
            get_in_addr((struct sockaddr *)&their_addr),
            s, sizeof s));
    
    // Выводим размер пакета
    printf("listener: packet is %d bytes long\n", numbytes);
    buf[numbytes] = '\0';  // Добавляем завершающий ноль, чтобы строка завершалась
    // Выводим содержимое пакета
    printf("listener: packet contains \"%s\"\n", buf);

    close(sockfd); // Закрываем сокет

    return 0; // Успешное завершение
}
