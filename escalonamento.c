#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "escalonamento.h"

int ler_entrada(const char *path, Task tasks[], int *n, int *total_time) {
    FILE *f = fopen(path, "r");
    if (!f) {
        fprintf(stderr, "Erro: nao foi possivel abrir o arquivo '%s'\n", path);
        return 1;
    }

    char linha[256];

    if (fgets(linha, sizeof(linha), f) == NULL) {
        fprintf(stderr, "Erro: arquivo vazio ou sem tempo total\n");
        fclose(f);
        return 1;
    }

    char *endptr;
    long valor = strtol(linha, &endptr, 10);

    if (endptr == linha) {
        fprintf(stderr, "Erro: tempo total invalido (nao numerico)\n");
        fclose(f);
        return 1;
    }
    if (valor <= 0) {
        fprintf(stderr, "Erro: tempo total deve ser positivo\n");
        fclose(f);
        return 1;
    }
    *total_time = (int)valor;

    int count = 0;

    while (fgets(linha, sizeof(linha), f) != NULL) {
        if (strspn(linha, " \t\r\n") == strlen(linha)) continue;

        if (count >= MAX_TASKS) {
            fprintf(stderr, "Erro: numero de tarefas excede o limite (%d)\n", MAX_TASKS);
            fclose(f);
            return 1;
        }

        char nome[MAX_NOME];
        long period, deadline, cpu;

        int lidos = sscanf(linha, "%31s %ld %ld %ld", nome, &period, &deadline, &cpu);

        if (lidos != 4) {
            fprintf(stderr, "Erro: linha malformada (campo faltando): '%s'\n", linha);
            fclose(f);
            return 1;
        }

        if (period <= 0 || deadline <= 0 || cpu <= 0) {
            fprintf(stderr, "Erro: valores devem ser positivos (tarefa '%s')\n", nome);
            fclose(f);
            return 1;
        }

        if (cpu > deadline || deadline > period) {
            fprintf(stderr, "Erro: tarefa '%s' viola C <= D <= P\n", nome);
            fclose(f);
            return 1;
        }

        strncpy(tasks[count].nome, nome, MAX_NOME - 1);
        tasks[count].nome[MAX_NOME - 1] = '\0';
        tasks[count].period = (int)period;
        tasks[count].deadline = (int)deadline;
        tasks[count].cpu = (int)cpu;
        tasks[count].posicao = count;

        tasks[count].remaining = 0;
        tasks[count].arrival = 0;
        tasks[count].abs_deadline = 0;
        tasks[count].completed_count = 0;
        tasks[count].lost_count = 0;

        count++;
    }

    if (count == 0) {
        fprintf(stderr, "Erro: nenhuma tarefa encontrada no arquivo\n");
        fclose(f);
        return 1;
    }

    *n = count;
    fclose(f);
    return 0;
}

int maior_prioridade(Task *a, Task *b, int algoritmo) {
    if (algoritmo == 0) { 
        if (a->period != b->period)
            return a->period < b->period;
    } else { 
        if (a->abs_deadline != b->abs_deadline)
            return a->abs_deadline < b->abs_deadline;
    }
    return a->posicao < b->posicao;
}

void simular(Task tasks[], int n, int total_time, int algoritmo, ResultadoSimulacao *resultado) {
    Task *executando_atual = NULL;
    int inicio_bloco = 0;
    resultado->n_blocos = 0;

    for (int t = 0; t < total_time; t++) {

        for (int i = 0; i < n; i++) {
            if (t % tasks[i].period == 0) {
                tasks[i].remaining = tasks[i].cpu;
                tasks[i].arrival = t;
                tasks[i].abs_deadline = t + tasks[i].deadline;
            }
        }

        for (int i = 0; i < n; i++) {
            if (tasks[i].abs_deadline == t && tasks[i].remaining > 0) {
                tasks[i].lost_count++;

                if (executando_atual == &tasks[i]) {
                    fechar_bloco(resultado, executando_atual, inicio_bloco, t, 'L');
                    executando_atual = NULL;
                }

                tasks[i].remaining = 0;
            }
        }

        Task *escolhida = escolher_tarefa(tasks, n, algoritmo);

        if (escolhida != executando_atual) {
            if (executando_atual != NULL) {
                fechar_bloco(resultado, executando_atual, inicio_bloco, t, 'H');
            }
            inicio_bloco = t;
            executando_atual = escolhida;
        }

        if (escolhida != NULL) {
            escolhida->remaining--;
            if (escolhida->remaining == 0) {
                escolhida->completed_count++;
                fechar_bloco(resultado, escolhida, inicio_bloco, t + 1, 'F');
                executando_atual = NULL;
            }
        }
    }

    if (executando_atual != NULL) {
        fechar_bloco(resultado, executando_atual, inicio_bloco, total_time, 'K');
    }

    for (int i = 0; i < n; i++) {
        if (tasks[i].remaining > 0) {
            tasks[i].killed_count++;
        }
    }
}

void fechar_bloco(ResultadoSimulacao *resultado, Task *tarefa, int inicio, int fim, char motivo) {
    if (fim <= inicio) return; 

    BlocoExecucao *b = &resultado->blocos[resultado->n_blocos];

    if (tarefa == NULL) {
        strcpy(b->nome, "idle");
    } else {
        strncpy(b->nome, tarefa->nome, MAX_NOME - 1);
        b->nome[MAX_NOME - 1] = '\0';
    }

    b->duration = fim - inicio;
    b->motivo = motivo;
    resultado->n_blocos++;
}

int gravar_saida(const char *algoritmo_nome, Task tasks[], int n, ResultadoSimulacao *resultado) {
    char filename[64];
    snprintf(filename, sizeof(filename), "%s_%s.out", algoritmo_nome, LOGIN);

    FILE *f = fopen(filename, "w");
    if (!f) return 1;

    char algo_maiusculo[16];
    for (int i = 0; algoritmo_nome[i]; i++) {
        algo_maiusculo[i] = toupper((unsigned char)algoritmo_nome[i]);
    }
    algo_maiusculo[strlen(algoritmo_nome)] = '\0';

    fprintf(f, "EXECUTION BY %s\n", algo_maiusculo);
    for (int i = 0; i < resultado->n_blocos; i++) {
        BlocoExecucao *b = &resultado->blocos[i];
        if (strcmp(b->nome, "idle") == 0) {
            fprintf(f, "idle for %d units\n", b->duration);
        } else {
            fprintf(f, "[%s] for %d units - %c\n", b->nome, b->duration, b->motivo);
        }
    }

    fprintf(f, "\nLOST DEADLINES\n");
    for (int i = 0; i < n; i++) {
        fprintf(f, "[%s] %d\n", tasks[i].nome, tasks[i].lost_count);
    }

    fprintf(f, "\nCOMPLETE EXECUTION\n");
    for (int i = 0; i < n; i++) {
        fprintf(f, "[%s] %d\n", tasks[i].nome, tasks[i].completed_count);
    }

    fprintf(f, "\nKILLED\n");
    for (int i = 0; i < n; i++) {
        fprintf(f, "[%s] %d\n", tasks[i].nome, tasks[i].killed_count);
    }

    fclose(f);
    return 0;
}

int main(int argc, char *argv[]) /////////////////////////MAIN//////////////////////////////
{
        if (argc != 3) {
        fprintf(stderr, "Uso: %s <rate|edf> <arquivo>\n", argv[0]);
        return 1;
    }

    int algoritmo;
    if (strcmp(argv[1], "rate") == 0) algoritmo = 0;
    else if (strcmp(argv[1], "edf") == 0) algoritmo = 1;
    else {
        fprintf(stderr, "Erro: algoritmo deve ser 'rate' ou 'edf'\n");
        return 1;
    }

    Task tasks[MAX_TASKS];
    int n, total_time;

    if (ler_entrada(argv[2], tasks, &n, &total_time) != 0) {
        // ler_entrada já deve ter escrito a mensagem específica em stderr
        return 1;
    }

    ResultadoSimulacao resultado;
    simular(tasks, n, total_time, algoritmo, &resultado);

    if (gravar_saida(argv[1], tasks, n, &resultado) != 0) {
        fprintf(stderr, "Erro ao gravar arquivo de saída\n");
        return 1;
    }

    return 0;
}