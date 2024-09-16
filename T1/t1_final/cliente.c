#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
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

// Função unificada para enviar dados com base na escolha do usuário
// Param: params[0] = clientId, params[1] = tipo de dados (1 = número, 2 = string)
void* enviarDados(void* arg) {
    int* parametros = (int*)arg;
    int clienteId = parametros[0]; // ID do cliente fornecido pelo usuário
    int tipoDado = parametros[1]; // Tipo de dado: 1 para números, 2 para strings
    char buffer[256];

    while (1) {
        int fd;

        if (tipoDado == 1) {
            // Envia números
            fd = open(FIFO_NUMEROS, O_WRONLY);
            if (fd == -1) {
                perror("Erro ao abrir o pipe para números");
                exit(EXIT_FAILURE);
            }

            int numero = rand() % 100; // Gera número aleatório
            snprintf(buffer, sizeof(buffer), "Cliente %d: %d", clienteId, numero);

            // Envia o número com o identificador do cliente
            write(fd, buffer, strlen(buffer) + 1);
            printf("Cliente %d enviou número: %d\n", clienteId, numero);

        } else if (tipoDado == 2) {
            // Envia strings
            fd = open(FIFO_STRINGS, O_WRONLY);
            if (fd == -1) {
                perror("Erro ao abrir o pipe para strings");
                exit(EXIT_FAILURE);
            }

            char stringAleatoria[256];
            gerarStringAleatoria(stringAleatoria, 10); // Gera uma string aleatória de 10 caracteres
            snprintf(buffer, sizeof(buffer), "Cliente %d: %s", clienteId, stringAleatoria);

            // Envia a string com o identificador do cliente
            write(fd, buffer, strlen(buffer) + 1);
            printf("Cliente %d enviou string: %s\n", clienteId, stringAleatoria);
        }

        close(fd);
        sleep(3); // Pausa de 3 segundos entre os envios
    }

    return NULL;
}

int main() {
    pthread_t thread;
    int parametros[2]; // parametros[0] = clienteId, parametros[1] = tipo de dado (número ou string)

    srand(time(NULL)); // Inicializa a semente de números aleatórios

    // Cliente insere o seu próprio ID
    printf("Insira o identificador do cliente: ");
    scanf("%d", &parametros[0]);

    // Cliente escolhe o tipo de requisição
    printf("Escolha o tipo de solicitação:\n");
    printf("1 - Número\n");
    printf("2 - String\n");
    printf("Sua escolha: ");
    scanf("%d", &parametros[1]);

    // Valida a escolha do tipo de requisição
    if (parametros[1] != 1 && parametros[1] != 2) {
        printf("Escolha inválida! Saindo...\n");
        exit(EXIT_FAILURE);
    }

    // Cria uma thread para enviar dados de acordo com a escolha do cliente
    if (pthread_create(&thread, NULL, enviarDados, parametros) != 0) {
        perror("Erro ao criar a thread");
        exit(EXIT_FAILURE);
    }

    // Espera a thread terminar (nunca irá terminar neste caso)
    pthread_join(thread, NULL);

    return 0;
}
