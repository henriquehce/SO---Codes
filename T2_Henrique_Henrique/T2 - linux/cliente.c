#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include <time.h>

#define FIFO_NUMEROS "/tmp/fifo_numeros"
#define FIFO_STRINGS "/tmp/fifo_strings"
#define FIFO_RESPOSTA_BASE "/tmp/fifo_resposta_cliente_"

void* enviarDados(void* arg) {
    int* parametros = (int*)arg;
    int clienteId = parametros[0]; 
    int tipoDado = parametros[1]; 
    char buffer[256];
    char fifoResposta[256];

    // Criação do FIFO de resposta para este cliente
    snprintf(fifoResposta, sizeof(fifoResposta), "%s%d", FIFO_RESPOSTA_BASE, clienteId);
    mkfifo(fifoResposta, 0666);

    while (1) {
        int fd;
        if (tipoDado == 1) {
            fd = open(FIFO_NUMEROS, O_WRONLY);
            if (fd == -1) {
                perror("Erro ao abrir o pipe para números");
                exit(EXIT_FAILURE);
            }

            snprintf(buffer, sizeof(buffer), "Cliente %d: %s", clienteId, fifoResposta);
            write(fd, buffer, strlen(buffer) + 1);
            printf("Cliente %d enviou uma solicitação de número\n", clienteId);
        } else if (tipoDado == 2) {
            fd = open(FIFO_STRINGS, O_WRONLY);
            if (fd == -1) {
                perror("Erro ao abrir o pipe para strings");
                exit(EXIT_FAILURE);
            }

            snprintf(buffer, sizeof(buffer), "Cliente %d: %s", clienteId, fifoResposta);
            write(fd, buffer, strlen(buffer) + 1);
            printf("Cliente %d enviou uma solicitação de string\n", clienteId);
        }

        close(fd);
        sleep(3); 
    }

    return NULL;
}

void* lerDados(void* arg) {
    int* parametros = (int*)arg;
    int clienteId = parametros[0]; 
    int tipoDado = parametros[1]; 
    ssize_t bytesLidos;
    char buffer[256];
    char fifoResposta[256];

    // Criação do FIFO de resposta exclusivo para este cliente
    snprintf(fifoResposta, sizeof(fifoResposta), "%s%d", FIFO_RESPOSTA_BASE, clienteId);
    
    while (1) {
        // Abre o FIFO de resposta exclusivo para este cliente
        int fd = open(fifoResposta, O_RDONLY);
        if (fd == -1) {
            perror("Falha ao abrir FIFO de resposta do cliente");
            continue;
        }

        // Lê a resposta do servidor
        bytesLidos = read(fd, buffer, sizeof(buffer));
        close(fd);

        if (bytesLidos == -1) {
            perror("Erro ao ler do FIFO");
            continue;
        }

        if (bytesLidos > 0) {
            buffer[bytesLidos] = '\0'; // Garantir que a string seja bem terminada
            printf("Cliente %d recebeu resposta: %s\n", clienteId, buffer);
        }

        // Pausa antes de tentar ler novamente
        sleep(3); // Ajuste o tempo conforme necessário
    }

    return NULL;
}

int main() {
    pthread_t thread, thread2;
    int parametros[2]; 

    srand(time(NULL));

    printf("Insira o identificador do cliente: ");
    scanf("%d", &parametros[0]);

    printf("Escolha o tipo de solicitação:\n");
    printf("1 - Número\n");
    printf("2 - String\n");
    printf("Sua escolha: ");
    scanf("%d", &parametros[1]);

    if (parametros[1] != 1 && parametros[1] != 2) {
        printf("Escolha inválida! Saindo...\n");
        exit(EXIT_FAILURE);
    }

    int* parametrosThread1 = malloc(sizeof(int) * 2);
    parametrosThread1[0] = parametros[0];
    parametrosThread1[1] = parametros[1];

    if (pthread_create(&thread, NULL, enviarDados, (void*)parametrosThread1) != 0) {
        perror("Erro ao criar a thread");
        exit(EXIT_FAILURE);
    }

    int* parametrosThread2 = malloc(sizeof(int) * 2);
    parametrosThread2[0] = parametros[0];
    parametrosThread2[1] = parametros[1];

    if (pthread_create(&thread2, NULL, lerDados, (void*)parametrosThread2) != 0) {
        perror("Erro ao criar a thread");
        exit(EXIT_FAILURE);
    }

    pthread_join(thread, NULL);
    pthread_join(thread2, NULL);

    return 0;
}