#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NAME_LEN 64

struct Node {
    char name[NAME_LEN];
    int age;
    struct Node *next;
};

void trim_newline(char *s) {
    size_t l = strlen(s);
    if (l && s[l-1] == '\n') s[l-1] = '\0';
}

int main(void) {
    int n;
    char buf[128];

    printf("Сколько записей? ");
    if (!fgets(buf, sizeof buf, stdin)) return 1;
    n = (int)strtol(buf, NULL, 10);
    if (n <= 0) {
        fprintf(stderr, "Неверное количество\n");
        return 1;
    }

    struct Node *head = NULL;
    struct Node *tail = NULL;

    for (int i = 0; i < n; i++) {
        struct Node *node = malloc(sizeof *node);
        if (!node) {
            perror("malloc");
            return 1;
        }
        node->next = NULL;

        // Имя
        printf("Имя #%d: ", i + 1);
        if (!fgets(node->name, sizeof node->name, stdin)) {
            free(node);
            break;
        }
        trim_newline(node->name);
        if (node->name[0] == '\0') strncpy(node->name, "<пусто>", sizeof node->name);

        // Возраст
        printf("Возраст #%d: ", i + 1);
        if (!fgets(buf, sizeof buf, stdin)) {
            free(node);
            break;
        }
        node->age = (int)strtol(buf, NULL, 10);

        // Вставка в конец списка
        if (!head) {
            head = tail = node;
        } else {
            tail->next = node;
            tail = node;
        }
    }

    // Печать списка
    printf("\nСписок:\n");
    struct Node *p = head;
    int idx = 1;
    while (p) {
        printf("%d. %s, %d лет\n", idx++, p->name, p->age);
        p = p->next;
    }

    // Очистка
    while (head) {
        struct Node *next = head->next;
        free(head);
        head = next;
    }

    return 0;
}
