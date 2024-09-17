#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <sys/resource.h> // Para getrusage()

#define FIFO_NUMEROS "/tmp/fifo_numeros"
#define FIFO_STRINGS "/tmp/fifo_strings"
#define REQUISICOES 10  // Número de requisições para calcular tempo médio
#define NUM_CLIENTES 1 // Defina o número de clientes

// Função para gerar uma string aleatória
void gerarStringAleatoria(char* str, int tamanho) {
    static char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    for (int i = 0; i < tamanho - 1; i++) {
        int chave = rand() % (int)(sizeof(charset) - 1);
        str[i] = charset[chave];
    }
    str[tamanho - 1] = '\0';
}

// Função para medir o uso de CPU
void usoCPU() {
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    printf("Uso de CPU: %ld.%06ld segundos\n", usage.ru_utime.tv_sec, usage.ru_utime.tv_usec);
}

// Função para enviar dados com base no tipo de requisição (número ou string)
// Param: params[0] = clienteId, params[1] = tipo de requisição (1 = número, 2 = string)
void* enviarDados(void* arg) {
    int* parametros = (int*)arg;
    int clienteId = parametros[0];
    int tipoDado = parametros[1]; // 1 = número, 2 = string
    char buffer[256];

    struct timespec start, end; // Para medir o tempo de resposta
    double tempoGasto, somaTempo = 0.0;
    int contadorRequisicoes = 0;

    while (contadorRequisicoes < REQUISICOES) { // Realiza um número fixo de requisições para medir o tempo médio
        int fd;

        // Inicia a medição do tempo
        clock_gettime(CLOCK_MONOTONIC, &start);

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

        // Finaliza a medição do tempo
        clock_gettime(CLOCK_MONOTONIC, &end);

        // Calcula o tempo gasto
        tempoGasto = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1E9;
        somaTempo += tempoGasto;

        printf("Cliente %d - Tempo de resposta: %f segundos\n", clienteId, tempoGasto);

        // Incrementa o contador de requisições
        contadorRequisicoes++;

        // Pausa de 3 segundos entre os envios
        sleep(3);
    }

    // Cálculo e exibição do tempo médio de resposta
    double tempoMedio = somaTempo / REQUISICOES;
    printf("Cliente %d - Tempo médio de resposta: %f segundos\n", clienteId, tempoMedio);

    // Exibe o uso de CPU após as requisições
    usoCPU();

    return NULL;
}

int main() {
    pthread_t threads[NUM_CLIENTES]; // Simular NUM_CLIENTES clientes
    int parametros[NUM_CLIENTES][2]; // NUM_CLIENTES clientes com [ID, Tipo de Requisição]

    srand(time(NULL)); // Inicializa a semente para geração de números aleatórios

    // Simulação de NUM_CLIENTES clientes
    for (int i = 0; i < NUM_CLIENTES; i++) {
        parametros[i][0] = i + 1; // Define o ID do cliente (1 a NUM_CLIENTES)
        parametros[i][1] = (i % 2) + 1; // Alterna entre requisições de números (1) e strings (2)

        // Cria uma thread para cada cliente
        if (pthread_create(&threads[i], NULL, enviarDados, parametros[i]) != 0) {
            perror("Erro ao criar a thread");
            exit(EXIT_FAILURE);
        }
    }

    // Espera as threads terminarem (nunca irão terminar neste caso)
    for (int i = 0; i < NUM_CLIENTES; i++) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}
