# SO---Codes

Servidor e Cliente para Comunicação via Pipes:

Este projeto demonstra a implementação de um servidor e cliente(s) que se comunicam utilizando pipes nomeados (FIFOs). Existem duas versões deste sistema:

Servidor para um único cliente (servidor.c e cliente.c)
Servidor para múltiplos clientes (servidor2.c e cliente2.c)

Requisitos
Sistema operacional Linux (para o suporte a pipes nomeados).
GCC (GNU Compiler Collection) para compilar os arquivos em C.

Estrutura do Projeto
bash
Copiar código
.
├── cliente.c        # Cliente para a versão de um único cliente
├── cliente2.c       # Cliente para a versão de múltiplos clientes
├── servidor.c       # Servidor para a versão de um único cliente
├── servidor2.c      # Servidor para a versão de múltiplos clientes
└── README.md        # Instruções do projeto

1. Servidor para um Único Cliente
Arquivos:
servidor.c: Implementa o servidor que se comunica com apenas um cliente.
cliente.c: Implementa o cliente que envia requisições e recebe respostas do servidor.
Descrição:
Nesta versão, o servidor pode atender apenas um cliente por vez. O cliente pode solicitar números aleatórios ou strings aleatórias, que o servidor retorna. A comunicação é feita via pipes nomeados.

Como Compilar:
bash
Copiar código
gcc servidor.c -o servidor -lpthread
gcc cliente.c -o cliente
Como Executar:
Execute o servidor em um terminal:

bash
Copiar código
./servidor
Em outro terminal, execute o cliente:

bash
Copiar código
./cliente
Funcionamento:
O cliente enviará uma requisição para o servidor.
O servidor responde ao cliente com um número aleatório ou uma string aleatória.
O cliente exibe a resposta recebida.
O cliente pode optar por encerrar a conexão enviando um sinal de saída (0), o que encerra o servidor.


2. Servidor para Múltiplos Clientes
Arquivos:
servidor2.c: Implementa o servidor que pode atender múltiplos clientes simultaneamente.
cliente2.c: Implementa o cliente que pode se comunicar com o servidor em paralelo com outros clientes.
Descrição:
Nesta versão, o servidor pode lidar com múltiplos clientes simultâneos. O servidor utiliza threads para processar as requisições de diferentes clientes em paralelo, e os clientes são implementados como processos distintos.

Como Compilar:
bash
Copiar código
gcc servidor2.c -o servidor2 -lpthread
gcc cliente2.c -o cliente2
Como Executar:
Execute o servidor em um terminal:

bash
Copiar código
./servidor2
Em outro terminal, execute o cliente que simula múltiplos processos clientes:

bash
Copiar código
./cliente2
Funcionamento:
O cliente2 criará vários processos filhos, e cada processo fará múltiplas requisições ao servidor.
O servidor processará essas requisições simultaneamente, respondendo com números ou strings.
O cliente exibe as respostas recebidas e termina após concluir todas as requisições.

Observações:
FIFO Pipes: Este projeto usa pipes nomeados (FIFO) para comunicação entre processos. Os pipes são criados pelo servidor e usados pelos clientes para enviar e receber mensagens.

Paralelismo: A versão de múltiplos clientes demonstra paralelismo utilizando fork para criar processos clientes e pthread no servidor para lidar com múltiplas conexões simultaneamente.

Limpeza dos Pipes
Caso precise remover os pipes nomeados após executar os programas, use o comando:

bash
Copiar código
rm /tmp/num_pipe /tmp/string_pipe
Isso removerá os pipes criados durante a execução do servidor.

Licença
Este projeto é apenas para fins educacionais e não possui uma licença formal.

