#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>

#define FIFO_NUMEROS "/tmp/fifo_numeros"
#define FIFO_STRINGS "/tmp/fifo_strings"

// Função para gerar uma string aleatória
void gerarStringAleatoria(char* str, int tamanho) {
    static char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    for (int i = 0; i < tamanho - 1; i++) {
        int chave = rand() % (int)(sizeof(charset) - 1);
        str[i] = charset[chave];
    }
    str[tamanho - 1] = '\0';
}

int main() {
    int clienteId;

    srand(time(NULL)); // Inicializa a semente para geração de números aleatórios

    // O usuário define o identificador do cliente
    printf("Insira o identificador do cliente: ");
    scanf("%d", &clienteId);

    while (1) {
        // Abre o pipe para enviar números
        int fdn = open(FIFO_NUMEROS, O_WRONLY);
        if (fdn == -1) {
            perror("Erro ao abrir o pipe para números");
            exit(EXIT_FAILURE);
        }

        // Gera um número aleatório
        int numero = rand() % 100;
        char buffer[256];
        snprintf(buffer, sizeof(buffer), "Cliente %d: %d", clienteId, numero);

        // Envia o número com o identificador do cliente
        write(fdn, buffer, strlen(buffer) + 1);
        close(fdn);

        // Abre o pipe para enviar strings
        int fd = open(FIFO_STRINGS, O_WRONLY);
        if (fd == -1) {
            perror("Erro ao abrir o pipe para strings");
            exit(EXIT_FAILURE);
        }

        // Gera uma string aleatória de 10 caracteres
        char stringAleatoria[256];
        gerarStringAleatoria(stringAleatoria, 10);

        // Envia a string com o identificador do cliente
        snprintf(buffer, sizeof(buffer), "Cliente %d: %s", clienteId, stringAleatoria);
        write(fd, buffer, strlen(buffer) + 1);
        close(fd);

        // Exibe no console o número e a string enviados
        printf("Cliente %d enviou número: %d e string: %s\n", clienteId, numero, stringAleatoria);

        sleep(1); // Pausa de 1 segundo entre os envios
    }

    return 0;
}