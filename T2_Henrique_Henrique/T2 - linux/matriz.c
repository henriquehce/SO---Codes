#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define ROWS 16384    // Número de linhas da matriz (16k)
#define COLS 16384    // Número de colunas da matriz (16k)
#define ITERATIONS 1000000  // Número de iterações para acessar a matriz

int main() {
    // Alocando uma matriz grande dinamicamente
    int **matrix = malloc(ROWS * sizeof(int *));
    if (matrix == NULL) {
        fprintf(stderr, "Falha ao alocar memória para linhas\n");
        return 1;
    }
    for (size_t i = 0; i < ROWS; i++) {
        matrix[i] = malloc(COLS * sizeof(int));
        if (matrix[i] == NULL) {
            fprintf(stderr, "Falha ao alocar memória para colunas\n");
            return 1;
        }
    }

    // Inicializando a matriz para garantir que a memória está alocada
    for (size_t i = 0; i < ROWS; i++) {
        for (size_t j = 0; j < COLS; j++) {
            matrix[i][j] = 0;
        }
    }

    srand(time(NULL));  // Inicializando o gerador de números aleatórios

    // Acessando a matriz aleatoriamente para provocar page faults
    for (size_t i = 0; i < ITERATIONS; i++) {
        size_t row = rand() % ROWS;  // Linha aleatória
        size_t col = rand() % COLS;  // Coluna aleatória
        matrix[row][col] = rand();   // Escreve um valor aleatório
    }

    printf("Execução completa. Verifique o consumo de memória.\n");

    // Mantendo o programa em execução para monitoramento
    while (1) {
        // Loop infinito para manter o programa ativo
    }

    // Liberando a memória alocada (não será alcançado com o loop infinito, mas é boa prática incluir)
    for (size_t i = 0; i < ROWS; i++) {
        free(matrix[i]);
    }
    free(matrix);

    return 0;
}
