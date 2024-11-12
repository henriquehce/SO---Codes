#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>

#define THREAD_NUM_NUMBERS 4 // Quantidade de threads para processar números
#define THREAD_NUM_STRINGS 4 // Quantidade de threads para processar strings

// Enum para identificar o tipo de requisição (números ou strings)
typedef enum {REQUISICAO_NUMERO, REQUISICAO_STRING} TipoRequisicao;

// Estrutura que representa uma tarefa, com seu tipo (número ou string) e os dados a serem processados
typedef struct Tarefa {
    TipoRequisicao tipo;
    char dados[256]; // Buffer que armazena os dados (número ou string) da requisição
} Tarefa;

// Fila de tarefas e contador de quantas tarefas estão na fila
Tarefa filaTarefas[256];
int quantidadeTarefas = 0;

// Mutex e variável de condição para sincronização de acesso à fila de tarefas
pthread_mutex_t mutexFila;
pthread_cond_t condFila;

// Nomes dos pipes (FIFO) usados para comunicação entre clientes e o servidor
char *fifo_numeros = "/tmp/fifo_numeros";
char *fifo_strings = "/tmp/fifo_strings";

// Função para gerar uma string aleatória
void gerarStringAleatoria(char* str, int tamanho) {
    static char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    for (int i = 0; i < tamanho - 1; i++) {
        int chave = rand() % (int)(sizeof(charset) - 1);
        str[i] = charset[chave];
    }
    str[tamanho - 1] = '\0';
}

// Função que executa a tarefa recebida (processa números ou strings)
// Param: tarefa - um ponteiro para a estrutura Tarefa
void executarTarefa(Tarefa* tarefa) {
    int fd;
    char buffer[256];
    char fifoResposta[256];

    // Extraindo o nome do FIFO de resposta da solicitação
    sscanf(tarefa->dados, "Cliente %*d: %s", fifoResposta);
    printf(fifoResposta);

    if (tarefa->tipo == REQUISICAO_NUMERO) {
        int numero = rand() % 100;  // Gera número aleatório entre 0 e 99
        snprintf(buffer, sizeof(buffer), "Resposta do servidor: %d", numero);
    } else if (tarefa->tipo == REQUISICAO_STRING) {
        char stringAleatoria[256];
        gerarStringAleatoria(stringAleatoria, 10); // Gera uma string aleatória de 10 caracteres
        snprintf(buffer, sizeof(buffer), "Resposta do servidor %s", stringAleatoria);
    }

     // Aqui você pode imprimir a resposta que será enviada para o cliente
    printf("Servidor gerou a resposta: %s\n", buffer);
    
    // Agora, vamos imprimir o caminho do FIFO de resposta, para garantir que é o FIFO correto
    printf("Enviando resposta para o FIFO: %s\n", fifoResposta);

    fd = open(fifoResposta, O_WRONLY);
    if (fd == -1) {
        perror("Falha ao abrir fifo de resposta do cliente");
        exit(EXIT_FAILURE);
    }

    write(fd, buffer, strlen(buffer) + 1);
    close(fd);
}


// Função que adiciona uma nova tarefa à fila de tarefas
void adicionarTarefa(Tarefa tarefa) {
    pthread_mutex_lock(&mutexFila);
    filaTarefas[quantidadeTarefas] = tarefa;
    quantidadeTarefas++;
    pthread_mutex_unlock(&mutexFila);
    pthread_cond_signal(&condFila);
}

// Função que processa requisições de números ou strings, dependendo do tipo de thread (números ou strings)
void* processarRequisicao(void* arg) {
    int tipoRequisicaoNumero = ((int)arg);
    free(arg);

    char buffer[256];
    int fd;
    ssize_t bytesLidos;

    while (1) {
        Tarefa tarefa;

        if (tipoRequisicaoNumero) {
            fd = open(fifo_numeros, O_RDONLY);
            if (fd == -1) {
                perror("Falha ao abrir fifo_numeros");
                continue;
            }
            bytesLidos = read(fd, buffer, sizeof(buffer));
            close(fd);

            if (bytesLidos > 0) {
                buffer[bytesLidos] = '\0';
                tarefa.tipo = REQUISICAO_NUMERO;
                strncpy(tarefa.dados, buffer, sizeof(tarefa.dados));
                adicionarTarefa(tarefa);
            }
        } else {
            fd = open(fifo_strings, O_RDONLY);
            if (fd == -1) {
                perror("Falha ao abrir fifo_strings");
                continue;
            }
            bytesLidos = read(fd, buffer, sizeof(buffer));
            close(fd);

            if (bytesLidos > 0) {
                buffer[bytesLidos] = '\0';
                tarefa.tipo = REQUISICAO_STRING;
                strncpy(tarefa.dados, buffer, sizeof(tarefa.dados));
                adicionarTarefa(tarefa);
            }
        }
    }
}

// Função que processa a fila de tarefas
void* processarFilaTarefas(void* args) {
    while (1) {
        Tarefa tarefa;

        pthread_mutex_lock(&mutexFila);
        while (quantidadeTarefas == 0) {
            pthread_cond_wait(&condFila, &mutexFila);
        }

        tarefa = filaTarefas[0];
        for (int i = 0; i < quantidadeTarefas - 1; i++) {
            filaTarefas[i] = filaTarefas[i + 1];
        }
        quantidadeTarefas--;
        pthread_mutex_unlock(&mutexFila);

        executarTarefa(&tarefa);
    }
}

int main() {
    pthread_t threadsNumeros[THREAD_NUM_NUMBERS + THREAD_NUM_STRINGS];
    pthread_t threadProcessadorTarefas;

    pthread_mutex_init(&mutexFila, NULL);
    pthread_cond_init(&condFila, NULL);

    mkfifo(fifo_numeros, 0666);
    mkfifo(fifo_strings, 0666);

    for (int i = 0; i < THREAD_NUM_NUMBERS; i++) {
        int *tipoRequisicaoNumero = malloc(sizeof(int));
        *tipoRequisicaoNumero = 1;
        pthread_create(&threadsNumeros[i], NULL, processarRequisicao, tipoRequisicaoNumero);
    }

    for (int i = 0; i < THREAD_NUM_STRINGS; i++) {
        int *tipoRequisicaoNumero = malloc(sizeof(int));
        *tipoRequisicaoNumero = 0;
        pthread_create(&threadsNumeros[THREAD_NUM_NUMBERS + i], NULL, processarRequisicao, tipoRequisicaoNumero);
    }

    pthread_create(&threadProcessadorTarefas, NULL, processarFilaTarefas, NULL);

    for (int i = 0; i < THREAD_NUM_NUMBERS + THREAD_NUM_STRINGS; i++) {
        pthread_join(threadsNumeros[i], NULL);
    }
    pthread_join(threadProcessadorTarefas, NULL);

    pthread_mutex_destroy(&mutexFila);
    pthread_cond_destroy(&condFila);

    unlink(fifo_numeros);
    unlink(fifo_strings);

    return 0;
}