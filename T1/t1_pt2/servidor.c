#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#define THREAD_NUM_NUMBERS 2 // Quantidade de threads para números
#define THREAD_NUM_STRINGS 2 // Quantidade de threads para strings

typedef enum {NUMBER_REQUEST, STRING_REQUEST} TaskType;

typedef struct Task {
    TaskType type;
    char data[256];
} Task;

Task taskQueue[256];
int taskCount = 0;

pthread_mutex_t mutexQueue;
pthread_cond_t condQueue;

char *fifo_numbers = "/tmp/fifo_numbers";
char *fifo_strings = "/tmp/fifo_strings";

void executeTask(Task* task) {
    if (task->type == NUMBER_REQUEST) {
        printf("Recebida solicitação de número: %s\n", task->data);
    } else if (task->type == STRING_REQUEST) {
        printf("Recebida solicitação de string: %s\n", task->data);
    }
}

void submitTask(Task task) {
    pthread_mutex_lock(&mutexQueue);
    taskQueue[taskCount] = task;
    taskCount++;
    pthread_mutex_unlock(&mutexQueue);
    pthread_cond_signal(&condQueue);
}

void* workerThread(void* arg) {
    int isNumberWorker = *((int*)arg);
    free(arg);

    char buffer[256];
    int fd;
    ssize_t bytesRead;

    while (1) {
        Task task;

        if (isNumberWorker) {
            fd = open(fifo_numbers, O_RDONLY);
            if (fd == -1) {
                perror("Falha ao abrir fifo_numbers");
                continue;
            }
            bytesRead = read(fd, buffer, sizeof(buffer));
            close(fd);

            if (bytesRead > 0) {
                buffer[bytesRead] = '\0';
                task.type = NUMBER_REQUEST;
                strncpy(task.data, buffer, sizeof(task.data));
                submitTask(task);
            }
        } else {
            fd = open(fifo_strings, O_RDONLY);
            if (fd == -1) {
                perror("Falha ao abrir fifo_strings");
                continue;
            }
            bytesRead = read(fd, buffer, sizeof(buffer));
            close(fd);

            if (bytesRead > 0) {
                buffer[bytesRead] = '\0';
                task.type = STRING_REQUEST;
                strncpy(task.data, buffer, sizeof(task.data));
                submitTask(task);
            }
        }
    }
}

void* processTaskQueue(void* args) {
    while (1) {
        Task task;

        pthread_mutex_lock(&mutexQueue);
        while (taskCount == 0) {
            pthread_cond_wait(&condQueue, &mutexQueue);
        }

        task = taskQueue[0];
        for (int i = 0; i < taskCount - 1; i++) {
            taskQueue[i] = taskQueue[i + 1];
        }
        taskCount--;
        pthread_mutex_unlock(&mutexQueue);

        executeTask(&task);
    }
}

int main() {
    pthread_t workers[THREAD_NUM_NUMBERS + THREAD_NUM_STRINGS];
    pthread_t taskProcessor;

    // Inicializa mutex e condição
    pthread_mutex_init(&mutexQueue, NULL);
    pthread_cond_init(&condQueue, NULL);

    // Cria os pipes nomeados (FIFOs)
    mkfifo(fifo_numbers, 0666);
    mkfifo(fifo_strings, 0666);

    // Cria as threads workers para números e strings
    for (int i = 0; i < THREAD_NUM_NUMBERS; i++) {
        int *isNumberWorker = malloc(sizeof(int));
        *isNumberWorker = 1;
        pthread_create(&workers[i], NULL, workerThread, isNumberWorker);
    }

    for (int i = 0; i < THREAD_NUM_STRINGS; i++) {
        int *isNumberWorker = malloc(sizeof(int));
        *isNumberWorker = 0;
        pthread_create(&workers[THREAD_NUM_NUMBERS + i], NULL, workerThread, isNumberWorker);
    }

    // Cria a thread que processa a fila de tarefas
    pthread_create(&taskProcessor, NULL, processTaskQueue, NULL);

    // Aguarda as threads terminarem
    for (int i = 0; i < THREAD_NUM_NUMBERS + THREAD_NUM_STRINGS; i++) {
        pthread_join(workers[i], NULL);
    }
    pthread_join(taskProcessor, NULL);

    // Destroi mutex e condição
    pthread_mutex_destroy(&mutexQueue);
    pthread_cond_destroy(&condQueue);

    // Remove os pipes
    unlink(fifo_numbers);
    unlink(fifo_strings);

    return 0;
}
