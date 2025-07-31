/*
** server.c -- a stream socket server demo
*/

#include <stdio.h>          // для printf(), perror() и др.
#include <stdlib.h>         // для exit()
#include <unistd.h>         // для close(), fork()
#include <errno.h>          // для errno
#include <string.h>         // для memset(), strerror()
#include <sys/types.h>      // базовые типы, напр. socklen_t
#include <sys/socket.h>     // для socket(), bind(), listen(), accept(), send()
#include <netinet/in.h>     // для struct sockaddr_in
#include <netdb.h>          // для getaddrinfo()
#include <arpa/inet.h>      // для inet_ntop()
#include <sys/wait.h>       // для waitpid()
#include <signal.h>         // для signal handling

#define PORT "3490"         // Port to listen

#define BACKLOG 10          // Размер очереди ожидания соединений (до 10 клиентов)


// Обработчик сигнала SIGCHLD для очистки зомби-процессов
void sigchld_handler(int s)
{
    (void)s; // quiet unused variable warning. I deliberately unuse this variable

    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;  // сохраняем значение errno, чтобы восстановить позже


    // Пока есть завершённые дочерние процессы — собираем их
    while(waitpid(-1, NULL, WNOHANG) > 0); //Это ожидание завершения дочернего процесса.
    
    //pid — кого ждать. -1 — любого дочернего.
    //status — куда записать код завершения.
    //0 — без особых опций.
    // Если не вызвать wait()/waitpid(), Дочерний процесс умирает, но его PID и код остаются в таблице процессов - это и есть зомби.
    errno = saved_errno;  // восстанавливаем errno
}


// Универсальный способ получить указатель на IP-адрес (IPv4 или IPv6)
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        // IPv4 — возвращаем указатель на поле sin_addr
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    // IPv6 — возвращаем указатель на поле sin6_addr
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}


int main(void)
{
    int sockfd, new_fd;                     // сокет слушателя и сокет для нового подключения
    struct addrinfo hints, *servinfo, *p;  // для getaddrinfo()
    struct sockaddr_storage their_addr;    // информация о подключившемся клиенте
    socklen_t sin_size;                    // размер структуры their_addr
    struct sigaction sa;                   // структура для настройки сигнала SIGCHLD
    int yes = 1;                           // для setsockopt
    char s[INET6_ADDRSTRLEN];              // строка для хранения IP-адреса клиента
    int rv;                                // код возврата

    memset(&hints, 0, sizeof hints);       // обнуляем hints перед использованием
    hints.ai_family = AF_INET;             // IPv4
    hints.ai_socktype = SOCK_STREAM;       // потоковый сокет (TCP)
    hints.ai_flags = AI_PASSIVE;           // использовать мой IP (0.0.0.0)

    // Получаем список структур, описывающих возможные адреса
    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // Перебираем все возможные адреса, пока не найдем подходящий
    for(p = servinfo; p != NULL; p = p->ai_next) {
        // Создаём сокет
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        // Настройка сокета на повторное использование адреса
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        // Пытаемся привязать сокет к адресу
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        break;  // успех — выходим из цикла
    }

    freeaddrinfo(servinfo);  // освобождаем память, выделенную getaddrinfo

    if (p == NULL)  {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    // Начинаем слушать порт
    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    // Устанавливаем обработчик сигнала SIGCHLD
    sa.sa_handler = sigchld_handler;  // функция-обработчик
    sigemptyset(&sa.sa_mask);         // не блокируем никакие сигналы
    sa.sa_flags = SA_RESTART;         // если прерван системный вызов — перезапустить его
    if (sigaction(SIGCHLD, &sa, NULL) == -1) //  Вызываем функцию, когда придет сигнал SIGCHLD 
    {
        perror("sigaction");
        exit(1);
    }

    printf("server: waiting for connections...\n");
    // Главный цикл: принимаем соединения
    while(1) {
        sin_size = sizeof their_addr;
        // Ждём подключения от клиента
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr,
            &sin_size);
        if (new_fd == -1) {
            perror("accept");
            continue;
        }
        char buff[50];
        int n = read(new_fd, buff, 49);
        buff[n-1] = '\0';
        int num = strcmp(buff,"quit");
        if (!strcmp(buff,"quit"))
        {
            break;
        }
        // Получаем строковое представление IP-адреса клиента
        inet_ntop(their_addr.ss_family,
            get_in_addr((struct sockaddr *)&their_addr),
            s, sizeof s);
        printf("server: got connection from %s\n", s);

        // Создаём дочерний процесс для обслуживания клиента.
        if (!fork()) //   Создает точную копию текущего процесса.
        {
            close(sockfd); // дочерний процесс не слушает новых клиентов
            // Отправляем сообщение клиенту
            if (send(new_fd, "Hello, world!", 13, 0) == -1)
                perror("send");
            close(new_fd);  // закрываем соединение
            exit(0);        // завершаем дочерний процесс
        }

        close(new_fd);  // родитель не будет использовать сокет клиента
    }

    return 0;
}
