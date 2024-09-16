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

// Função para enviar dados com base no tipo de requisição
void* sendData(void* arg) {
    int* params = (int*)arg;
    int clientId = params[0];
    int dataType = params[1]; // 1 = número, 2 = string
    char buffer[256];

    while (1) {
        int fd;

        if (dataType == 1) {
            // Envia números
            fd = open(FIFO_NUMBERS, O_WRONLY);
            if (fd == -1) {
                perror("Erro ao abrir o pipe para números");
                exit(EXIT_FAILURE);
            }

            int number = rand() % 100; // Gera número aleatório
            snprintf(buffer, sizeof(buffer), "Cliente %d: %d", clientId, number);

            write(fd, buffer, strlen(buffer) + 1); // Envia o número com o identificador do cliente
            printf("Cliente %d enviou número: %d\n", clientId, number);

        } else if (dataType == 2) {
            // Envia strings
            fd = open(FIFO_STRINGS, O_WRONLY);
            if (fd == -1) {
                perror("Erro ao abrir o pipe para strings");
                exit(EXIT_FAILURE);
            }

            char randomString[256];
            generateRandomString(randomString, 10); // Gera uma string aleatória de 10 caracteres
            snprintf(buffer, sizeof(buffer), "Cliente %d: %s", clientId, randomString);

            write(fd, buffer, strlen(buffer) + 1); // Envia a string com o identificador do cliente
            printf("Cliente %d enviou string: %s\n", clientId, randomString);
        }

        close(fd);
        sleep(3); // Pausa de 3 segundos entre os envios
    }

    return NULL;
}

int main() {
    pthread_t threads[10]; // Simular 10 clientes
    int params[10][2]; // 10 clientes com [ID, Tipo de Requisição]

    srand(time(NULL));

    // Simulação de 10 clientes
    for (int i = 0; i < 10; i++) {
        params[i][0] = i + 1; // Cliente ID
        params[i][1] = (i % 2) + 1; // Tipo de Requisição: 1 (número), 2 (string)

        if (pthread_create(&threads[i], NULL, sendData, params[i]) != 0) {
            perror("Erro ao criar a thread");
            exit(EXIT_FAILURE);
        }
    }

    // Espera as threads terminarem (nunca irão terminar neste caso)
    for (int i = 0; i < 10; i++) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}
