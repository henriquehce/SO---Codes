#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#define NUM_PIPE "/tmp/num_pipe"
#define STRING_PIPE "/tmp/string_pipe"

// Função para enviar solicitação de número e receber resposta
void request_number() {
    int num_fd;
    char request[] = "1";
    char response[10];

    // Envia o pedido de número
    num_fd = open(NUM_PIPE, O_WRONLY);
    write(num_fd, request, strlen(request) + 1);
    close(num_fd);

    // Lê a resposta do servidor (número aleatório)
    num_fd = open(NUM_PIPE, O_RDONLY);
    read(num_fd, response, sizeof(response));
    close(num_fd);

    // Imprimir no cliente o que foi recebido
    printf("Cliente (PID %d): Número recebido do servidor: %s\n", getpid(), response);
}

// Função para enviar solicitação de string e receber resposta
void request_string() {
    int str_fd;
    char request[] = "1";
    char response[100];

    // Envia o pedido de string
    str_fd = open(STRING_PIPE, O_WRONLY);
    write(str_fd, request, strlen(request) + 1);
    close(str_fd);

    // Lê a resposta do servidor (palavra aleatória)
    str_fd = open(STRING_PIPE, O_RDONLY);
    read(str_fd, response, sizeof(response));
    close(str_fd);

    // Imprimir no cliente o que foi recebido
    printf("Cliente (PID %d): String recebida do servidor: %s\n", getpid(), response);
}

int main() {
    pid_t pid;
    int i;

    // Criar múltiplos processos clientes simultâneos
    for (i = 0; i < 3; i++) {
        pid = fork();
        if (pid == 0) {  // Processo filho
            for (int j = 0; j < 5; j++) {
                // Alternar entre solicitações de número e string
                if (j % 2 == 0) {
                    request_number();
                } else {
                    request_string();
                }
                sleep(1);  // Espera 1 segundo entre as requisições
            }
            exit(0);
        }
    }

    // Esperar que todos os processos filhos terminem
    for (i = 0; i < 3; i++) {
        wait(NULL);
    }

    return 0;
}
