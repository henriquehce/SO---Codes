#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <windows.h>
#include <time.h>

#define FIFO_NUMEROS "\\\\.\\pipe\\fifo_numeros"
#define FIFO_STRINGS "\\\\.\\pipe\\fifo_strings"
#define FIFO_RESPOSTA_BASE "\\\\.\\pipe\\fifo_resposta_cliente_"

#define BUFFER_SIZE 256

DWORD WINAPI enviarDados(LPVOID arg) {
    
    char buffer[BUFFER_SIZE];
    DWORD dwRead, dwWritten;

    int* parametros = (int*)arg;
    int clienteId = parametros[0]; 
    int tipoDado = parametros[1]; 
    char fifoResposta[256];

    // Criação do FIFO de resposta para este cliente
    snprintf(fifoResposta, sizeof(fifoResposta), "%s%d", FIFO_RESPOSTA_BASE, clienteId);
    printf("criado nome  %s", fifoResposta);

    // Esperar por um cliente para conectar (isso simula a criação do pipe para esse cliente)
    
    while (1) {
        HANDLE fd;
        if (tipoDado == 1) {
            fd = CreateFile(FIFO_NUMEROS, GENERIC_READ | GENERIC_WRITE,
                       0,
                       NULL,
                       OPEN_EXISTING,
                       0,
                       NULL);
            if (fd == INVALID_HANDLE_VALUE) {
                DWORD dwError = GetLastError();
                printf("Erro ao abrir o pipe para numeros: %ld\n", dwError);
                exit(EXIT_FAILURE);
            }

            snprintf(buffer, sizeof(buffer), "Cliente %d: %s", clienteId, fifoResposta);
            WriteFile(fd, buffer, strlen(buffer) + 1, NULL, NULL);
            printf("Cliente %d enviou uma solicitacao de numero\n", clienteId);

        }else if (tipoDado == 2) {
            fd = CreateFile(FIFO_STRINGS, GENERIC_READ | GENERIC_WRITE,
                       0,
                       NULL,
                       OPEN_EXISTING,
                       0,
                       NULL);
            if (fd == INVALID_HANDLE_VALUE) {
                DWORD dwError = GetLastError();
                printf("Erro ao abrir o pipe para strings: %ld\n", dwError);
                exit(EXIT_FAILURE);
            }

            snprintf(buffer, sizeof(buffer), "Cliente %d: %s", clienteId, fifoResposta);
            WriteFile(fd, buffer, strlen(buffer) + 1, NULL, NULL);
            printf("Cliente %d enviou uma solicitacao de string\n", clienteId);

        }

        CloseHandle(fd);
        Sleep(5000);  // Pausa para o próximo envio
    }

    return 0;
}

DWORD WINAPI lerDados(LPVOID arg) {
    int* parametros = (int*)arg;
    int clienteId = parametros[0]; 
    int tipoDado = parametros[1]; 
    DWORD bytesLidos;
    char buffer[256];
    char fifoResposta[256];

    // Criação do FIFO de resposta exclusivo para este cliente
   


    while (1) {
        snprintf(fifoResposta, sizeof(fifoResposta), "%s%d", FIFO_RESPOSTA_BASE, clienteId);

        HANDLE hPipeResposta = CreateNamedPipe(fifoResposta, PIPE_ACCESS_DUPLEX, PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
                        PIPE_UNLIMITED_INSTANCES, 512, 512, 0, NULL);
        if (hPipeResposta == INVALID_HANDLE_VALUE) {
        printf("Erro ao criar o named pipe para strings: %d\n", GetLastError());
        return -1;
        }
       
        if (!ConnectNamedPipe(hPipeResposta, NULL))
        {
        printf("Falha em conectar ao cliente. Codigo do erro: %d\n", GetLastError());
        CloseHandle(hPipeResposta);
        return 1;
        }

        // Lê a resposta do servidor
        if (ReadFile(hPipeResposta, buffer, sizeof(buffer), &bytesLidos, NULL)) {
            buffer[bytesLidos] = '\0';  // Garantir que a string seja bem terminada
            printf("Cliente %d recebeu resposta: %s\n", clienteId, buffer);
        }

        CloseHandle(hPipeResposta);

        // Pausa antes de tentar ler novamente
        Sleep(3000); // Ajuste o tempo conforme necessário
    }

    return 0;
}

int main() {
    int parametros[2]; 
    HANDLE thread1, thread2;

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

    thread1 = CreateThread(NULL, 0, enviarDados, (LPVOID)parametrosThread1, 0, NULL);
    if (thread1 == NULL) {
        perror("Erro ao criar a thread para enviar dados");
        exit(EXIT_FAILURE);
    }


    int* parametrosThread2 = malloc(sizeof(int) * 2);
    parametrosThread2[0] = parametros[0];
    parametrosThread2[1] = parametros[1];

    thread2 = CreateThread(NULL, 0, lerDados, (LPVOID)parametrosThread2, 0, NULL);
    if (thread2 == NULL) {
        perror("Erro ao criar a thread para ler dados");
        exit(EXIT_FAILURE);
    }

    // Aguardar as threads terminarem (não há realmente uma condição de saída)
    WaitForSingleObject(thread1, INFINITE);
    WaitForSingleObject(thread2, INFINITE);

    // Limpeza
    CloseHandle(thread1);
    CloseHandle(thread2);

    return 0;
}
