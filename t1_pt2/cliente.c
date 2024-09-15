#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include <time.h>

#define FIFO_NUMBERS "/tmp/fifo_numbers"
#define FIFO_STRINGS "/tmp/fifo_strings"

// Função para gerar uma string aleatória
void generateRandomString(char* str, int length) {
    static char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    for (int i = 0; i < length - 1; i++) {
        int key = rand() % (int)(sizeof(charset) - 1);
        str[i] = charset[key];
    }
    str[length - 1] = '\0';
}

// Função da thread que envia números
void* sendNumbers(void* arg) {
    int clientId = *((int*)arg);
    while (1) {
        int fd = open(FIFO_NUMBERS, O_WRONLY);
        if (fd == -1) {
            perror("Erro ao abrir o pipe para números");
            exit(EXIT_FAILURE);
        }

        int number = rand() % 100; // Gera número aleatório
        char buffer[256];
        snprintf(buffer, sizeof(buffer), "Cliente %d: %d", clientId, number);

        write(fd, buffer, strlen(buffer) + 1); // Envia o número com o identificador do cliente
        close(fd);

        printf("Cliente %d enviou número: %d\n", clientId, number);
        sleep(3); // Pausa de 3 segundos
    }
    return NULL;
}

// Função da thread que envia strings
void* sendStrings(void* arg) {
    int clientId = *((int*)arg);
    while (1) {
        int fd = open(FIFO_STRINGS, O_WRONLY);
        if (fd == -1) {
            perror("Erro ao abrir o pipe para strings");
            exit(EXIT_FAILURE);
        }

        char randomString[256];
        generateRandomString(randomString, 10); // Gera uma string aleatória de 10 caracteres

        char buffer[256];
        snprintf(buffer, sizeof(buffer), "Cliente %d: %s", clientId, randomString);

        write(fd, buffer, strlen(buffer) + 1); // Envia a string com o identificador do cliente
        close(fd);

        printf("Cliente %d enviou string: %s\n", clientId, randomString);
        sleep(3); // Pausa de 3 segundos
    }
    return NULL;
}

int main() {
    pthread_t threads[2];
    int clientId;

    srand(time(NULL));

    // O usuário define o identificador do cliente
    printf("Insira o identificador do cliente: ");
    scanf("%d", &clientId);

    // Cria uma thread para enviar números
    if (pthread_create(&threads[0], NULL, sendNumbers, &clientId) != 0) {
        perror("Erro ao criar thread de números");
        exit(EXIT_FAILURE);
    }

    // Cria uma thread para enviar strings
    if (pthread_create(&threads[1], NULL, sendStrings, &clientId) != 0) {
        perror("Erro ao criar thread de strings");
        exit(EXIT_FAILURE);
    }

    // Espera as threads terminarem (nunca irão terminar neste caso)
    for (int i = 0; i < 2; i++) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}
