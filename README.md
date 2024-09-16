# SO---Codes

Projeto T1 - Sistema de Servidor e Clientes com Pipes e Threads
Este projeto implementa um sistema de servidor com pool de threads que recebe e processa requisições de clientes. As requisições podem ser de dois tipos:

Números: os clientes enviam números aleatórios.
Strings: os clientes enviam strings aleatórias.
O sistema é implementado utilizando pipes nomeados (FIFOs) para a comunicação entre os clientes e o servidor, e threads para processar as requisições simultaneamente. O servidor pode processar múltiplos clientes ao mesmo tempo.

Estrutura do Projeto
servidor.c: Implementa o servidor que recebe e processa as requisições de números e strings.
cliente.c: Simula um único cliente que envia ou números ou strings, de acordo com a escolha do usuário.
multi_cliente.c: Simula vários clientes simultâneos, alternando entre envio de números e strings.
cliente_ambos.c: Um cliente que envia ambos os tipos de requisições (números e strings) para o servidor em ciclos contínuos.

Como Rodar o Projeto
1. Compilar os Códigos
Use o GCC para compilar cada um dos códigos.

bash
Copiar código
# Compilar o servidor
gcc servidor.c -o servidor -lpthread

# Compilar o cliente simples
gcc cliente.c -o cliente -lpthread

# Compilar o cliente múltiplo
gcc multi_cliente.c -o multi_cliente -lpthread

# Compilar o cliente que envia ambos (número e string)
gcc cliente_ambos.c -o cliente_ambos

2. Executar o Servidor
O servidor deve ser iniciado primeiro para criar os pipes e estar pronto para receber as requisições dos clientes.

bash
Copiar código
./servidor
3. Executar os Clientes
Você pode executar qualquer um dos clientes, dependendo da simulação desejada.

Cliente Simples (cliente.c)
Este cliente envia números ou strings, conforme a escolha do usuário.

bash
Copiar código
./cliente
Cliente Múltiplo (multi_cliente.c)
Este cliente simula 10 clientes simultâneos, onde metade envia números e a outra metade envia strings.

bash
Copiar código
./multi_cliente
Cliente Ambos (cliente_ambos.c)
Este cliente envia tanto números quanto strings em cada ciclo de execução.

bash
Copiar código
./cliente_ambos
Funcionamento Detalhado
1. servidor.c
O servidor utiliza threads para processar requisições de clientes. Ele possui um pool de threads com algumas threads dedicadas ao processamento de números e outras ao processamento de strings.

Pipes Nomeados (FIFOs): Dois pipes são criados:
/tmp/fifo_numeros: Pipe para clientes que enviam números.
/tmp/fifo_strings: Pipe para clientes que enviam strings.
Fila de Tarefas: As requisições recebidas dos clientes são armazenadas em uma fila e processadas pelas threads do servidor.
2. cliente.c
O cliente permite que o usuário escolha entre enviar números ou strings. Após a escolha, o cliente entra em um loop onde envia requisições continuamente para o servidor.

3. multi_cliente.c
Simula 10 clientes simultâneos, utilizando threads para enviar números e strings ao servidor. As threads alternam entre enviar números ou strings.

4. cliente_ambos.c
Este cliente envia tanto números quanto strings para o servidor a cada ciclo. Ele abre os pipes correspondentes e envia as duas requisições.

Tecnologias Utilizadas
- Linguagem C
- Threads (biblioteca pthread)
- Pipes Nomeados (FIFOs) para comunicação entre processos
- Mutex e Condições para sincronização no servidor
- Observações
O servidor deve estar rodando antes de iniciar os clientes, pois ele é responsável por criar os pipes nomeados.
Para finalizar o servidor e os clientes, você pode usar Ctrl+C.
