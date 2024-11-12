#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <process.h>
#include <time.h>

HANDLE lock_X;  // Recurso X
HANDLE lock_Y;  // Recurso Y

double timestamps[5];      // Armazena timestamps das threads
const char* estados[5];    // Estado de cada thread
int lock_owners[2] = {-1, -1}; // Dono do lock (-1 significa livre)

// Função para obter o timestamp atual em milissegundos
double getCurrentTimestamp() {
    FILETIME ft;
    ULARGE_INTEGER li;
    GetSystemTimeAsFileTime(&ft);
    li.LowPart = ft.dwLowDateTime;
    li.HighPart = ft.dwHighDateTime;
    return (li.QuadPart / 10000.0); // Retorna timestamp em milissegundos
}

// Função para verificar deadlock e liberar o recurso.
int wound_wait(int t1, int t2, HANDLE* lock_primario, int lock_id) {
    ReleaseMutex(*lock_primario);
    lock_owners[lock_id] = -1;
    printf("[Deadlock] Thread %d liberando recursos para evitar deadlock com %d.\n", t1 + 1, t2 + 1);
    estados[t1] = "terminada";
    return 1;
}

// Função para verificar o estado do lock
const char* verificar_lock(HANDLE* lock, int lock_id) {
    if (WaitForSingleObject(*lock, 0) == WAIT_OBJECT_0) {
        ReleaseMutex(*lock);
        return "LIVRE";
    } else {
        int owner = lock_owners[lock_id];
        return owner == -1 ? "desconhecido" : "BLOQUEADO";
    }
}

// Função para liberar o recurso se estiver bloqueado
void liberar_recurso_se_bloqueado(HANDLE* lock, int lock_id, int thread_id) {
    if (WaitForSingleObject(*lock, 0) != WAIT_OBJECT_0) {
        ReleaseMutex(*lock);
        lock_owners[lock_id] = -1;
        printf("Thread %d liberou o recurso %d.\n", thread_id + 1, lock_id + 1);
    }
}

// Função principal de acesso ao recurso
void acessar_recurso(int thread_id, HANDLE* lock_primario, HANDLE* lock_secundario, int lock_id_primario, int lock_id_secundario) {
    estados[thread_id] = "executando";
    timestamps[thread_id] = getCurrentTimestamp();

    while (1) {
        printf("Verificando primeiro recurso para Thread-%d: %s\n", thread_id + 1, verificar_lock(lock_primario, lock_id_primario));

        if (WaitForSingleObject(*lock_primario, 0) == WAIT_OBJECT_0) {
            lock_owners[lock_id_primario] = thread_id;
            printf("Thread-%d obteve o primeiro recurso.\n", thread_id + 1);
            estados[thread_id] = "processando";

            while (1) {
                Sleep((500 + rand() % 1500));
                printf("Verificando segundo recurso para Thread-%d: %s\n", thread_id + 1, verificar_lock(lock_secundario, lock_id_secundario));

                if (WaitForSingleObject(*lock_secundario, 0) == WAIT_OBJECT_0) {
                    lock_owners[lock_id_secundario] = thread_id;
                    printf("Thread-%d obteve ambos os recursos. Processando...\n", thread_id + 1);
                    Sleep((1000 + rand() % 2000));
                    liberar_recurso_se_bloqueado(lock_secundario, lock_id_secundario, thread_id);
                    estados[thread_id] = "finalizada";
                    break;
                } else {
                    printf("Thread-%d aguardando o segundo recurso.\n", thread_id + 1);
                    estados[thread_id] = "aguardando";

                    for (int t = 0; t < 5; t++) {
                        if ((strcmp(estados[t], "processando") == 0 || strcmp(estados[t], "aguardando") == 0) && t != thread_id) {
                            if (wound_wait(thread_id, t, lock_primario, lock_id_primario)) {
                                return;
                            }
                        }
                    }
                    Sleep((500 + rand() % 1000));
                }
            }

            liberar_recurso_se_bloqueado(lock_primario, lock_id_primario, thread_id);
            break;
        } else {
            printf("Thread-%d aguardando o primeiro recurso.\n", thread_id + 1);
            Sleep(500 + rand() % 1000);
        }
    }
}

// Função de transação para cada thread
unsigned __stdcall transacao(void* arg) {
    int thread_id = *(int*)arg;
    printf("Thread-%d iniciou.\n", thread_id + 1);
    timestamps[thread_id] = getCurrentTimestamp();

    if (rand() % 2 == 0) {
        acessar_recurso(thread_id, &lock_X, &lock_Y, 0, 1);
    } else {
        printf("Thread-%d começou inversa\n", thread_id + 1);
        acessar_recurso(thread_id, &lock_Y, &lock_X, 1, 0);
    }

    printf("Thread-%d finalizou.\n", thread_id + 1);
    return 0;
}

int main() {
    srand((unsigned)time(0));
    HANDLE threads[5];
    int thread_ids[5];
    lock_X = CreateMutex(NULL, FALSE, NULL);
    lock_Y = CreateMutex(NULL, FALSE, NULL);

    for (int i = 0; i < 5; i++) {
        thread_ids[i] = i;
        threads[i] = (HANDLE)_beginthreadex(NULL, 0, transacao, &thread_ids[i], 0, NULL);
        Sleep((500 + rand() % 1000));
    }

    for (int i = 0; i < 5; i++) {
        WaitForSingleObject(threads[i], INFINITE);
        CloseHandle(threads[i]);
    }

    printf("Todas as threads finalizaram.\n");

    CloseHandle(lock_X);
    CloseHandle(lock_Y);

    return 0;
}
