#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

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
    printf("Cliente: Número recebido do servidor: %s\n", response);
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
    printf("Cliente: String recebida do servidor: %s\n", response);
}

// Função para enviar o sinal de encerramento para o servidor
void send_exit_signal() {
    int num_fd, str_fd;
    char exit_signal[] = "0";

    // Envia o sinal para a thread de números
    num_fd = open(NUM_PIPE, O_WRONLY);
    write(num_fd, exit_signal, strlen(exit_signal) + 1);
    close(num_fd);

    // Envia o sinal para a thread de strings
    str_fd = open(STRING_PIPE, O_WRONLY);
    write(str_fd, exit_signal, strlen(exit_signal) + 1);
    close(str_fd);

    printf("Cliente: Sinal de encerramento enviado para o servidor.\n");
}

int main() {
    int option;

    while (1) {
        printf("\nEscolha uma opção:\n");
        printf("1. Receber número aleatório\n");
        printf("2. Receber string\n");
        printf("0. Sair e fechar o servidor\n");
        scanf("%d", &option);

        if (option == 0) {
            send_exit_signal();  // Envia o sinal de saída e encerra
            break;
        } else if (option == 1) {
            request_number();    // Solicita um número ao servidor
        } else if (option == 2) {
            request_string();    // Solicita uma string ao servidor
        } else {
            printf("Opção inválida. Tente novamente.\n");
        }
    }

    return 0;
}
