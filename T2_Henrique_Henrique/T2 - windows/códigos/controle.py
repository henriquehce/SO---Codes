import threading
import time
import random
from datetime import datetime

#link do video:https://www.youtube.com/watch?v=d6Ml6XFzKFg&ab_channel=Henriquereis

# Criação dos locks (bloqueios) para os recursos X e Y
lock_X = threading.Lock()  # Recurso X
lock_Y = threading.Lock()  # Recurso Y

# Dicionários para armazenar timestamps e estados das threads
timestamps = {}
estado_thread = {}
lock_owners = {}  # Dicionário para rastrear os proprietários dos locks

# Função que verifica deadlock e libera o recurso.
def wound_wait(t1, t2, lock_primario):
    if t1 not in timestamps or t2 not in timestamps:
        print(f"[Erro] Timestamps ausentes para {t1} ou {t2}.")
        return False

    if timestamps[t1] >= timestamps[t2]:  #  Se a t1 for mais nova
        liberar_recurso_se_bloqueado(lock_primario, "X", t1)
        print(f"[Deadlock] Thread {t1} liberando recursos para evitar deadlock com {t2}.")
        estado_thread[t1]["estado"] = "terminada"
        return True
    return False

def verificar_lock(lock, nome_recurso):
    if lock.locked():
         # Retorna quem está segurando o lock
        owner = lock_owners.get(lock, "desconhecido")
        return f"O {nome_recurso} está BLOQUEADO por {owner}."
    else:
        return f"O {nome_recurso} está LIVRE."
       

def liberar_recurso_se_bloqueado(lock, nome_recurso, thread_name):
    if lock.locked():
        lock.release()
        print(f"{thread_name} liberou o {nome_recurso}.")
        lock_owners.pop(lock, None)  # Remove a thread que estava segurando o lock
    else:
        print(f"Erro: {thread_name} tentou liberar {nome_recurso}, mas ele já estava liberado.")

def acessar_recurso(thread_name, lock_primario, lock_secundario):
    global timestamps, estado_thread, lock_owners

    estado_thread[thread_name] = {"estado": "executando", "timestamp": datetime.now().timestamp()}

    while True:
        print(f"{verificar_lock(lock_primario, f'primeiro recurso para {thread_name}')}")

        if lock_primario.acquire(blocking=False):
            lock_owners[lock_primario] = thread_name  # Marca a thread como proprietária do lock
            print(f"{thread_name} obteve o primeiro recurso.")
            estado_thread[thread_name]["estado"] = "processando"

            while True:
                time.sleep(random.uniform(0.5, 2))
                print(f"{verificar_lock(lock_secundario, f'segundo recurso para {thread_name}')}")

                if lock_secundario.acquire(blocking=False):
                    lock_owners[lock_secundario] = thread_name  # Marca a thread como proprietária do lock
                    print(f"{thread_name} obteve ambos os recursos. Processando...")
                    time.sleep(random.uniform(1, 3))
                    liberar_recurso_se_bloqueado(lock_secundario, "segundo recurso", thread_name)
                    estado_thread[thread_name]["estado"] = "finalizada"
                    break

                else:
                    print(f"{thread_name} aguardando o segundo recurso.")
                    estado_thread[thread_name]["estado"] = "aguardando"

                    for t in estado_thread:
                        if estado_thread[t]["estado"] in ("processando", "aguardando") and t != thread_name:
                            if wound_wait(thread_name, t, lock_primario):
                                return

                    time.sleep(random.uniform(0.5, 1.5))

            liberar_recurso_se_bloqueado(lock_primario, "primeiro recurso", thread_name)
            break

        else:
            print(f"{thread_name} aguardando o primeiro recurso.")
            time.sleep(random.uniform(0.5, 1.5))

def transacao(thread_name):
    print(f"{thread_name} iniciou.")
    timestamps[thread_name] = datetime.now().timestamp()

    if random.choice([True, False]):
        acessar_recurso(thread_name, lock_X, lock_Y)
    else:
        print(f"{thread_name} começou inversa")
        acessar_recurso(thread_name, lock_Y, lock_X)

    print(f"{thread_name} finalizou.")

def main():
    threads = []

    for i in range(5):
        thread_name = f"Thread-{i+1}"
        t = threading.Thread(target=transacao, args=(thread_name,))
        threads.append(t)
        t.start()
        time.sleep(random.uniform(0.5, 1.5))

    for t in threads:
        t.join()

    print("Todas as threads finalizaram.")

if __name__ == "__main__":
    main()
