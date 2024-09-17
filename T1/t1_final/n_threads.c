#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#define THREAD_NUM_NUMBERS 5 // Quantidade de threads para processar números
#define THREAD_NUM_STRINGS 5 // Quantidade de threads para processar strings

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

// Contador de threads ativas
int threadsAtivas = 0;
pthread_mutex_t mutexThreadsAtivas; // Mutex para proteger o contador de threads

// Mutex e variável de condição para sincronização de acesso à fila de tarefas
pthread_mutex_t mutexFila;
pthread_cond_t condFila;

// Nomes dos pipes (FIFO) usados para comunicação entre clientes e o servidor
char *fifo_numeros = "/tmp/fifo_numeros";
char *fifo_strings = "/tmp/fifo_strings";

// Função que executa a tarefa recebida (processa números ou strings)
// Param: tarefa - um ponteiro para a estrutura Tarefa
void executarTarefa(Tarefa* tarefa) {
    if (tarefa->tipo == REQUISICAO_NUMERO) {
        // Exibe o número processado
        printf("Recebida solicitação de número: %s\n", tarefa->dados);
    } else if (tarefa->tipo == REQUISICAO_STRING) {
        // Exibe a string processada
        printf("Recebida solicitação de string: %s\n", tarefa->dados);
    }
}

// Função que adiciona uma nova tarefa à fila de tarefas
// Protegida por mutex para garantir a consistência da fila
void adicionarTarefa(Tarefa tarefa) {
    pthread_mutex_lock(&mutexFila); // Trava o mutex antes de acessar a fila
    filaTarefas[quantidadeTarefas] = tarefa; // Adiciona a nova tarefa no final da fila
    quantidadeTarefas++; // Incrementa o contador de tarefas
    pthread_mutex_unlock(&mutexFila); // Libera o mutex
    pthread_cond_signal(&condFila); // Sinaliza que há uma nova tarefa disponível
}

// Função que processa requisições de números ou strings, dependendo do tipo de thread (números ou strings)
// Param: arg - um ponteiro que indica o tipo de requisição (número = 1, string = 0)
void* processarRequisicao(void* arg) {
    int tipoRequisicaoNumero = *((int*)arg); // Define se a thread processa números ou strings
    free(arg); // Libera a memória alocada para o argumento

    char buffer[256];
    int fd;
    ssize_t bytesLidos;

    while (1) { // Loop infinito para processar requisições continuamente
        Tarefa tarefa; // Estrutura que irá armazenar a requisição

        // Incrementa o contador de threads ativas
        pthread_mutex_lock(&mutexThreadsAtivas);
        threadsAtivas++;
        pthread_mutex_unlock(&mutexThreadsAtivas);

        if (tipoRequisicaoNumero) {
            // Abre o pipe nomeado para números e lê a requisição
            fd = open(fifo_numeros, O_RDONLY);
            if (fd == -1) {
                perror("Falha ao abrir fifo_numeros");
                continue;
            }
            bytesLidos = read(fd, buffer, sizeof(buffer)); // Lê os dados do pipe
            close(fd);

            if (bytesLidos > 0) { // Verifica se houve leitura de dados
                buffer[bytesLidos] = '\0'; // Adiciona o caractere nulo no final da string
                tarefa.tipo = REQUISICAO_NUMERO; // Define o tipo de requisição como número
                strncpy(tarefa.dados, buffer, sizeof(tarefa.dados)); // Copia os dados lidos para a estrutura Tarefa
                adicionarTarefa(tarefa); // Adiciona a tarefa à fila
            }
        } else {
            // Abre o pipe nomeado para strings e lê a requisição
            fd = open(fifo_strings, O_RDONLY);
            if (fd == -1) {
                perror("Falha ao abrir fifo_strings");
                continue;
            }
            bytesLidos = read(fd, buffer, sizeof(buffer)); // Lê os dados do pipe
            close(fd);

            if (bytesLidos > 0) { // Verifica se houve leitura de dados
                buffer[bytesLidos] = '\0'; // Adiciona o caractere nulo no final da string
                tarefa.tipo = REQUISICAO_STRING; // Define o tipo de requisição como string
                strncpy(tarefa.dados, buffer, sizeof(tarefa.dados)); // Copia os dados lidos para a estrutura Tarefa
                adicionarTarefa(tarefa); // Adiciona a tarefa à fila
            }
        }

        // Decrementa o contador de threads ativas após processamento
        pthread_mutex_lock(&mutexThreadsAtivas);
        threadsAtivas--;
        pthread_mutex_unlock(&mutexThreadsAtivas);
    }
}

// Função que processa a fila de tarefas
// Retira as tarefas da fila em ordem e executa uma a uma
void* processarFilaTarefas(void* args) {
    while (1) { // Loop infinito para processar tarefas da fila
        Tarefa tarefa;

        pthread_mutex_lock(&mutexFila); // Trava o mutex para acessar a fila
        while (quantidadeTarefas == 0) { // Espera até que haja uma tarefa na fila
            pthread_cond_wait(&condFila, &mutexFila); // Aguarda sinalização de nova tarefa
        }

        // Retira a primeira tarefa da fila
        tarefa = filaTarefas[0];
        for (int i = 0; i < quantidadeTarefas - 1; i++) {
            filaTarefas[i] = filaTarefas[i + 1]; // Move as tarefas restantes para o início da fila
        }
        quantidadeTarefas--; // Decrementa o contador de tarefas
        pthread_mutex_unlock(&mutexFila); // Libera o mutex

        executarTarefa(&tarefa); // Executa a tarefa retirada da fila
    }
}

// Função para monitorar e exibir o número de threads ativas a cada 5 segundos
void* monitorarThreadsAtivas(void* args) {
    while (1) {
        pthread_mutex_lock(&mutexThreadsAtivas);
        printf("Threads ativas: %d\n", threadsAtivas);
        pthread_mutex_unlock(&mutexThreadsAtivas);
        sleep(5); // Exibe a cada 5 segundos
    }
}

int main() {
    pthread_t threadsNumeros[THREAD_NUM_NUMBERS + THREAD_NUM_STRINGS]; // Threads para processar números e strings
    pthread_t threadProcessadorTarefas; // Thread que processa a fila de tarefas
    pthread_t threadMonitoramento; // Thread que monitora o número de threads ativas

    // Inicializa mutex e condição de fila
    pthread_mutex_init(&mutexFila, NULL);
    pthread_cond_init(&condFila, NULL);
    
    // Inicializa mutex para threads ativas
    pthread_mutex_init(&mutexThreadsAtivas, NULL);

    // Cria os pipes nomeados (FIFOs) para números e strings
    mkfifo(fifo_numeros, 0666);
    mkfifo(fifo_strings, 0666);

    // Cria as threads que processam números
    for (int i = 0; i < THREAD_NUM_NUMBERS; i++) {
        int *tipoRequisicaoNumero = malloc(sizeof(int));
        *tipoRequisicaoNumero = 1; // Define como thread de números
        pthread_create(&threadsNumeros[i], NULL, processarRequisicao, tipoRequisicaoNumero);
    }

    // Cria as threads que processam strings
    for (int i = 0; i < THREAD_NUM_STRINGS; i++) {
        int *tipoRequisicaoNumero = malloc(sizeof(int));
        *tipoRequisicaoNumero = 0; // Define como thread de strings
        pthread_create(&threadsNumeros[THREAD_NUM_NUMBERS + i], NULL, processarRequisicao, tipoRequisicaoNumero);
    }

    // Cria a thread que processa a fila de tarefas
    pthread_create(&threadProcessadorTarefas, NULL, processarFilaTarefas, NULL);

    // Cria a thread que monitora o número de threads ativas
    pthread_create(&threadMonitoramento, NULL, monitorarThreadsAtivas, NULL);

    // Aguarda a execução das threads
    for (int i = 0; i < THREAD_NUM_NUMBERS + THREAD_NUM_STRINGS; i++) {
        pthread_join(threadsNumeros[i], NULL);
    }
    pthread_join(threadProcessadorTarefas, NULL);
    pthread_join(threadMonitoramento, NULL);

    // Destroi mutex e condição
    pthread_mutex_destroy(&mutexFila);
    pthread_cond_destroy(&condFila);

    pthread_mutex_destroy(&mutexThreadsAtivas);

    // Remove os pipes criados
    unlink(fifo_numeros);
    unlink(fifo_strings);

    return 0;
}
