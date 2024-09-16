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

// Função unificada para enviar dados com base na escolha do usuário
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
    pthread_t thread;
    int params[2]; // params[0] = clientId, params[1] = dataType

    srand(time(NULL));

    // Cliente insere o seu próprio ID
    printf("Insira o identificador do cliente: ");
    scanf("%d", &params[0]);

    // Cliente escolhe o tipo de requisição
    printf("Escolha o tipo de solicitação:\n");
    printf("1 - Número\n");
    printf("2 - String\n");
    printf("Sua escolha: ");
    scanf("%d", &params[1]);

    if (params[1] != 1 && params[1] != 2) {
        printf("Escolha inválida! Saindo...\n");
        exit(EXIT_FAILURE);
    }

    // Cria uma thread para enviar dados de acordo com a escolha do cliente
    if (pthread_create(&thread, NULL, sendData, params) != 0) {
        perror("Erro ao criar a thread");
        exit(EXIT_FAILURE);
    }

    // Espera a thread terminar (nunca irá terminar neste caso)
    pthread_join(thread, NULL);

    return 0;
}
