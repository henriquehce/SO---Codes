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

// Função para enviar dados com base no tipo de requisição (número ou string)
// Param: params[0] = clienteId, params[1] = tipo de requisição (1 = número, 2 = string)
void* enviarDados(void* arg) {
    int* parametros = (int*)arg;
    int clienteId = parametros[0];
    int tipoDado = parametros[1]; // 1 = número, 2 = string
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

            write(fd, buffer, strlen(buffer) + 1); // Envia o número com o identificador do cliente
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

            write(fd, buffer, strlen(buffer) + 1); // Envia a string com o identificador do cliente
            printf("Cliente %d enviou string: %s\n", clienteId, stringAleatoria);
        }

        close(fd);
        sleep(3); // Pausa de 3 segundos entre os envios
    }

    return NULL;
}

int main() {
    pthread_t threads[10]; // Simular 10 clientes
    int parametros[10][2]; // 10 clientes com [ID, Tipo de Requisição]

    srand(time(NULL)); // Inicializa a semente para geração de números aleatórios

    // Simulação de 10 clientes
    for (int i = 0; i < 10; i++) {
        parametros[i][0] = i + 1; // Define o ID do cliente (1 a 10)
        parametros[i][1] = (i % 2) + 1; // Alterna entre requisições de números (1) e strings (2)

        // Cria uma thread para cada cliente
        if (pthread_create(&threads[i], NULL, enviarDados, parametros[i]) != 0) {
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