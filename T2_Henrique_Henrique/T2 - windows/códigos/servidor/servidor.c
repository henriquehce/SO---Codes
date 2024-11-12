#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <windows.h>
#include <time.h>

#define THREAD_NUM_NUMBERS 2 // Quantidade de threads para processar números
#define THREAD_NUM_STRINGS 2 // Quantidade de threads para processar strings

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
CRITICAL_SECTION mutexFila;
HANDLE condFilaEvent;

// Nomes dos pipes (Named Pipes) usados para comunicação entre clientes e o servidor
char *fifo_numeros = "\\\\.\\pipe\\fifo_numeros";
char *fifo_strings = "\\\\.\\pipe\\fifo_strings";

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
    HANDLE hPipe;
    DWORD bytesWritten;
    char buffer[256];
    char fifoResposta[256];

    // Extraindo o nome do FIFO de resposta da solicitação
    sscanf(tarefa->dados, "Cliente %*d: %s", fifoResposta);

    if (tarefa->tipo == REQUISICAO_NUMERO) {
        int numero = rand() % 100;  // Gera número aleatório entre 0 e 99
        snprintf(buffer, sizeof(buffer), "%d", numero);
    } else if (tarefa->tipo == REQUISICAO_STRING) {
        char stringAleatoria[256];
        gerarStringAleatoria(stringAleatoria, 10); // Gera uma string aleatória de 10 caracteres
        snprintf(buffer, sizeof(buffer), "Resposta do servidor %s", stringAleatoria);
    }

    // Aqui você pode imprimir a resposta que será enviada para o cliente
    printf("Servidor gerou a resposta: %s\n", buffer);
    
    // Agora, vamos imprimir o caminho do FIFO de resposta, para garantir que é o FIFO correto
    printf("Enviando resposta para o FIFO: %s\n", fifoResposta);

    // Abrir o named pipe para o cliente

    hPipe = CreateFile(fifoResposta, GENERIC_READ | GENERIC_WRITE,
                       0,
                       NULL,
                       OPEN_EXISTING,
                       0,
                       NULL);
    if (hPipe == INVALID_HANDLE_VALUE) {
        DWORD dwError = GetLastError();
        printf("Erro ao abrir o pipe de resposta para números: %ld\n", dwError);
        exit(EXIT_FAILURE);
    }

    WriteFile(hPipe, buffer, strlen(buffer) + 1, &bytesWritten, NULL);
    printf("\nResposta enviada \n");
    CloseHandle(hPipe);
}

// Função que adiciona uma nova tarefa à fila de tarefas
void adicionarTarefa(Tarefa tarefa) {
    EnterCriticalSection(&mutexFila);
    filaTarefas[quantidadeTarefas] = tarefa;
    quantidadeTarefas++;
    LeaveCriticalSection(&mutexFila);
    SetEvent(condFilaEvent); // Sinaliza que uma nova tarefa foi adicionada
}
// Função que processa a fila de tarefas
DWORD WINAPI processarFilaTarefas(LPVOID args) {
    while (1) {
        Tarefa tarefa;

        WaitForSingleObject(condFilaEvent, INFINITE); // Espera até que haja tarefas na fila

        EnterCriticalSection(&mutexFila);
        if (quantidadeTarefas > 0) {
            tarefa = filaTarefas[0];
            for (int i = 0; i < quantidadeTarefas - 1; i++) {
                filaTarefas[i] = filaTarefas[i + 1];
            }
            quantidadeTarefas--;
        }
        LeaveCriticalSection(&mutexFila);

        executarTarefa(&tarefa);
    }
    return 0;
}

// Função que processa requisições de números ou strings, dependendo do tipo de thread (números ou strings)
DWORD WINAPI processarRequisicao(LPVOID arg) {
      // Extrair o tipo de requisição e o handle do pipe a partir do argumento
    int* args = (int*)arg;
    int tipoRequisicao = args[0];
    HANDLE hPipe = (HANDLE)args[1];
    free(arg);  // Libera a memória alocada para o argumento
    int a;
    char buffer[256];
    a++;
    DWORD bytesRead;

    while (1) {
        Tarefa tarefa;

        // Processar requisições dependendo do tipo (números ou strings)
        if (tipoRequisicao == REQUISICAO_NUMERO) {
            // Aguardar conexão com o cliente via pipe para números
            if (ConnectNamedPipe(hPipe, NULL)) {
                printf("Thread %d conectou \n", a);
                if (ReadFile(hPipe, buffer, sizeof(buffer), &bytesRead, NULL)) {
                    buffer[bytesRead] = '\0';  // Garantir que o buffer tenha a string terminada corretamente
                    tarefa.tipo = REQUISICAO_NUMERO;
                    strncpy(tarefa.dados, buffer, sizeof(tarefa.dados));
                    // Adicionar a tarefa à fila para processamento posterior
                    adicionarTarefa(tarefa);
                }
                // Fechar o pipe após o processamento
                CloseHandle(hPipe);
                hPipe = CreateNamedPipe(fifo_numeros, PIPE_ACCESS_DUPLEX,
                                PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
                                PIPE_UNLIMITED_INSTANCES, 512, 512, 0, NULL);
                if (hPipe == INVALID_HANDLE_VALUE) {
                printf("Erro ao criar o named pipe: %ld\n", GetLastError());
                }
            }
            
        } 
        else if (tipoRequisicao == REQUISICAO_STRING) {
           // Aguardar conexão com o cliente via pipe para strings
            if (ConnectNamedPipe(hPipe, NULL)) {
                printf(" Thread %d conectou \n", a);
                if (ReadFile(hPipe, buffer, sizeof(buffer), &bytesRead, NULL)) {
                    buffer[bytesRead] = '\0';  // Garantir que o buffer tenha a string terminada corretamente
                    tarefa.tipo = REQUISICAO_STRING;
                    strncpy(tarefa.dados, buffer, sizeof(tarefa.dados));
                    // Adicionar a tarefa à fila para processamento posterior
                    adicionarTarefa(tarefa);
                }
                // Fechar o pipe após o processamento
                CloseHandle(hPipe);
                hPipe = CreateNamedPipe(fifo_strings, PIPE_ACCESS_DUPLEX,
                                PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
                                PIPE_UNLIMITED_INSTANCES, 512, 512, 0, NULL);
                if (hPipe == INVALID_HANDLE_VALUE) {
                printf("Erro ao criar o named pipe: %ld\n", GetLastError());
                }
            }
        }
    }
    return 0;
}

int main() {
    srand(time(NULL));
    HANDLE threadsNumeros[THREAD_NUM_NUMBERS + THREAD_NUM_STRINGS];
    HANDLE threadProcessadorTarefas;

    InitializeCriticalSection(&mutexFila);
    condFilaEvent = CreateEvent(NULL, FALSE, FALSE, NULL); // Evento para fila de tarefas

    // Criar Named Pipes
    HANDLE hPipeNumeros = CreateNamedPipe(fifo_numeros, PIPE_ACCESS_DUPLEX,
                            PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
                            1,
                            512,
                            512,
                            0,
                            NULL);
    if (hPipeNumeros == INVALID_HANDLE_VALUE) {
        printf("Erro ao criar o named pipe para números: %d\n", GetLastError());
        return -1;
    }

    HANDLE hPipeStrings = CreateNamedPipe(fifo_strings, PIPE_ACCESS_DUPLEX,
                            PIPE_TYPE_MESSAGE | PIPE_READMODE_BYTE | PIPE_WAIT,
                            1,
                            512,
                            512,
                            0,
                            NULL);
    if (hPipeStrings == INVALID_HANDLE_VALUE) {
        printf("Erro ao criar o named pipe para strings: %d\n", GetLastError());
        return -1;
    }

    printf("Named Pipes criados com sucesso.\n");
    // Criar a thread que processa a fila de tarefas
     // Criar a thread para conexão de números


   for (int i = 0; i < THREAD_NUM_NUMBERS; i++) {
    int* argsNumero = malloc(2 * sizeof(int));
    argsNumero[0] = REQUISICAO_NUMERO;  // Tipo de requisição para números
    argsNumero[1] = (int)hPipeNumeros; // Passa o handle do pipe de números
    threadsNumeros[i] = CreateThread(NULL, 0, processarRequisicao, argsNumero, 0, NULL);
    }

    for (int i = 0; i < THREAD_NUM_STRINGS; i++) {
        int* argsString = malloc(2 * sizeof(int));
        argsString[0] = REQUISICAO_STRING;  // Tipo de requisição para strings
        argsString[1] = (int)hPipeStrings; // Passa o handle do pipe de strings
        threadsNumeros[THREAD_NUM_NUMBERS + i] = CreateThread(NULL, 0, processarRequisicao, argsString, 0, NULL);
    }
        // Criar a thread que processa a fila de tarefas
    threadProcessadorTarefas = CreateThread(NULL, 0, processarFilaTarefas, NULL, 0, NULL);

    // Esperar as threads terminarem (isso nunca acontece no nosso exemplo)
    WaitForMultipleObjects(THREAD_NUM_NUMBERS + THREAD_NUM_STRINGS, threadsNumeros, TRUE, INFINITE);

    // Fechar handles
    CloseHandle(hPipeNumeros);
    CloseHandle(hPipeStrings);

    DeleteCriticalSection(&mutexFila);
    return 0;
}
