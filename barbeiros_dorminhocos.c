#include <stdio.h>       // Biblioteca padrão de entrada e saída
#include <stdlib.h>      // Biblioteca padrão para funções utilitárias, como malloc e rand
#include <pthread.h>     // Biblioteca para manipulação de threads
#include <semaphore.h>   // Biblioteca para manipulação de semáforos
#include <unistd.h>      // Biblioteca para manipulação de chamadas ao sistema, como sleep
#include <time.h>        // Biblioteca para manipulação de tempo, usada para inicializar a semente do rand

#define NUM_BARBEIROS 3  // Definindo o número de barbeiros
#define NUM_CADEIRAS 5   // Definindo o número de cadeiras de espera

sem_t clientes;          // Semáforo que conta o número de clientes esperando
sem_t barbeiros;         // Semáforo que conta o número de barbeiros disponíveis
sem_t mutex;             // Mutex para garantir exclusão mútua ao acessar cadeiras de espera

int cadeiras_livres = NUM_CADEIRAS;  // Variável que mantém o controle do número de cadeiras de espera disponíveis

void *cliente(void *arg);  // Declaração da função do cliente
void *barbeiro(void *arg); // Declaração da função do barbeiro

void *cliente(void *arg) {
    int id = *(int *)arg;    // Identifica o cliente com base no argumento passado

    sleep(rand() % 5 + 1);   // Simula o tempo aleatório até o cliente chegar ao salão

    sem_wait(&mutex);        // Aguarda acesso exclusivo às cadeiras de espera
    if (cadeiras_livres > 0) {  // Verifica se há cadeiras livres
        cadeiras_livres--;       // O cliente ocupa uma cadeira, decrementando o número de cadeiras livres
        printf("Cliente %d sentou na cadeira de espera. Cadeiras livres: %d\n", id, cadeiras_livres);
        sem_post(&clientes);     // Informa que um cliente está esperando
        sem_post(&mutex);        // Libera o acesso às cadeiras de espera

        sem_wait(&barbeiros);    // Aguarda até que um barbeiro esteja disponível
        printf("Cliente %d está sendo atendido.\n", id);

        // Simula o tempo do corte de cabelo
        sleep(rand() % 5 + 1);

        printf("Cliente %d terminou o corte e saiu.\n", id);
        sem_post(&barbeiros);    // Barbeiro disponível novamente

        sem_wait(&mutex);        // Aguarda acesso exclusivo para liberar a cadeira
        cadeiras_livres++;       // Libera a cadeira ocupada pelo cliente
        sem_post(&mutex);        // Libera o acesso às cadeiras de espera
    } else {  // Se não houver cadeiras livres
        printf("Cliente %d foi embora sem ser atendido. Cadeiras cheias.\n", id);
        sem_post(&mutex);  // Libera o acesso às cadeiras de espera
    }

    pthread_exit(NULL);  // Termina a thread do cliente
}

void *barbeiro(void *arg) {
    int id = *(int *)arg;  // Identifica o barbeiro com base no argumento passado

    while (1) {  // Loop infinito para o barbeiro atender continuamente
        sem_wait(&clientes);  // Aguarda até que haja um cliente
        sem_wait(&mutex);     // Aguarda acesso exclusivo às cadeiras de espera

        printf("Barbeiro %d está atendendo um cliente.\n", id);

        sem_post(&mutex);      // Libera o acesso às cadeiras de espera
        sem_post(&barbeiros);  // Informa que o barbeiro está ocupado atendendo

        // Simula o tempo do corte de cabelo
        sleep(rand() % 5 + 1);

        printf("Barbeiro %d terminou o atendimento e está pronto para o próximo cliente.\n", id);
    }

    pthread_exit(NULL);  // Termina a thread do barbeiro (embora nunca seja alcançado por causa do loop infinito)
}

int main() {
    srand(time(NULL));  // Inicializa a semente do gerador de números aleatórios

    pthread_t th_barbeiros[NUM_BARBEIROS];  // Array de threads para os barbeiros
    pthread_t th_clientes[10];  // Array de threads para os clientes (número ajustável conforme necessário)

    // Inicializa os semáforos
    sem_init(&clientes, 0, 0);              // Inicializa o semáforo de clientes com 0 (nenhum cliente no início)
    sem_init(&barbeiros, 0, NUM_BARBEIROS); // Inicializa o semáforo de barbeiros com o número de barbeiros disponíveis
    sem_init(&mutex, 0, 1);                 // Inicializa o mutex para controle de acesso às cadeiras

    int ids[10];  // Array de IDs para os clientes e barbeiros
    for (int i = 0; i < NUM_BARBEIROS; i++) {  // Criação das threads dos barbeiros
        ids[i] = i + 1;  // Define o ID do barbeiro
        pthread_create(&th_barbeiros[i], NULL, barbeiro, &ids[i]);  // Cria a thread do barbeiro
    }

    for (int i = 0; i < 10; i++) {  // Criação das threads dos clientes
        ids[i] = i + 1;  // Define o ID do cliente
        pthread_create(&th_clientes[i], NULL, cliente, &ids[i]);  // Cria a thread do cliente
        sleep(rand() % 3);  // Simula um intervalo aleatório entre a chegada dos clientes
    }

    for (int i = 0; i < 10; i++) {  // Aguarda a conclusão de todas as threads dos clientes
        pthread_join(th_clientes[i], NULL);
    }

    for (int i = 0; i < NUM_BARBEIROS; i++) {
        pthread_cancel(th_barbeiros[i]);  // Encerra as threads dos barbeiros (simplificação para terminar o programa)
    }

    // Destrói os semáforos ao final do programa
    sem_destroy(&clientes);
    sem_destroy(&barbeiros);
    sem_destroy(&mutex);

    return 0;  // Retorna 0 para indicar execução bem-sucedida
}
