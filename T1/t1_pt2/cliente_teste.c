#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
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


int main() {
    int clientId;

    srand(time(NULL));

    // O usuário define o identificador do cliente
    printf("Insira o identificador do cliente: ");
    scanf("%d", &clientId);

        while (1) {
        int fdn = open(FIFO_NUMBERS, O_WRONLY);
        if (fdn == -1) {
            perror("Erro ao abrir o pipe para números");
            exit(EXIT_FAILURE);
        }

        int number = rand() % 100; // Gera número aleatório
        char buffer[256];
        snprintf(buffer, sizeof(buffer), "Cliente %d: %d", clientId, number);

        write(fdn, buffer, strlen(buffer) + 1); // Envia o número com o identificador do cliente
        close(fdn);    

        int fd = open(FIFO_STRINGS, O_WRONLY);
        if (fd == -1) {
            perror("Erro ao abrir o pipe para strings");
            exit(EXIT_FAILURE);
        }

        char randomString[256];
        generateRandomString(randomString, 10); // Gera uma string aleatória de 10 caracteres

        snprintf(buffer, sizeof(buffer), "Cliente %d: %s", clientId, randomString);

        write(fd, buffer, strlen(buffer) + 1); // Envia a string com o identificador do cliente
        close(fd);

        printf("Cliente %d enviou número: %d e string: %s\n", clientId, number, randomString);
        sleep(1); // Pausa de 3 segundos
    }

    return 0;
}