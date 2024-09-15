#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>  // Inclui a biblioteca para mkfifo

#define NUM_THREADS 2
#define NUM_PIPE "/tmp/num_pipe"
#define STRING_PIPE "/tmp/string_pipe"

// Palavras em português que o servidor irá enviar para os clientes
const char* palavras[] = {"cachorro", "gato", "elefante", "passaro", "tigre"};
int palavras_size = sizeof(palavras) / sizeof(palavras[0]);

// Função que lida com pedidos de números aleatórios
void* handle_numbers(void* arg) {
    int num_fd;
    char buffer[10];
    char response[10];

    mkfifo(NUM_PIPE, 0666);  // Criar o pipe para números

    while (1) {
        num_fd = open(NUM_PIPE, O_RDONLY);
        read(num_fd, buffer, sizeof(buffer));

        // Verificar se a resposta é "0" (sair) ou "1" (solicitar número)
        if (strcmp(buffer, "0") == 0) {
            printf("Thread de Números: Finalizando servidor...\n");
            close(num_fd);
            exit(0);  // Termina o servidor
        } else if (strcmp(buffer, "1") == 0) {
            // Gera um número aleatório
            srand(time(NULL));
            int num_aleatorio = rand() % 100;  // Número aleatório entre 0 e 99
            sprintf(response, "%d", num_aleatorio);

            close(num_fd);

            // Envia o número aleatório de volta ao cliente
            num_fd = open(NUM_PIPE, O_WRONLY);
            write(num_fd, response, strlen(response) + 1);
            close(num_fd);

            // Imprimir no servidor o que foi enviado
            printf("Servidor (Números): Número enviado %d\n", num_aleatorio);
        } else {
            // Fechar o servidor se a resposta for inválida
            printf("Servidor: Resposta errada recebida! Encerrando o servidor...\n");
            close(num_fd);
            exit(1);  // Fecha o servidor devido a resposta errada
        }
    }
    return NULL;
}

// Função que lida com pedidos de palavras
void* handle_strings(void* arg) {
    int str_fd;
    char buffer[100];
    char response[100];

    mkfifo(STRING_PIPE, 0666);  // Criar o pipe para strings

    while (1) {
        str_fd = open(STRING_PIPE, O_RDONLY);
        read(str_fd, buffer, sizeof(buffer));

        // Verificar se a resposta é "0" (sair) ou "1" (solicitar string)
        if (strcmp(buffer, "0") == 0) {
            printf("Thread de Strings: Finalizando servidor...\n");
            close(str_fd);
            exit(0);  // Termina o servidor
        } else if (strcmp(buffer, "1") == 0) {
            // Envia uma palavra aleatória do vetor
            srand(time(NULL));
            int palavra_idx = rand() % palavras_size;
            const char* palavra = palavras[palavra_idx];
            strcpy(response, palavra);

            close(str_fd);

            str_fd = open(STRING_PIPE, O_WRONLY);
            write(str_fd, response, strlen(response) + 1);
            close(str_fd);

            // Imprimir no servidor o que foi enviado
            printf("Servidor (Strings): Palavra enviada %s\n", palavra);
        } else {
            // Fechar o servidor se a resposta for inválida
            printf("Servidor: Resposta errada recebida! Encerrando o servidor...\n");
            close(str_fd);
            exit(1);  // Fecha o servidor devido a resposta errada
        }
    }
    return NULL;
}

int main() {
    pthread_t threads[NUM_THREADS];

    // Criar thread para lidar com números
    pthread_create(&threads[0], NULL, handle_numbers, NULL);
    
    // Criar thread para lidar com strings
    pthread_create(&threads[1], NULL, handle_strings, NULL);

    // Esperar que as threads terminem (servidor sempre ativo)
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}
