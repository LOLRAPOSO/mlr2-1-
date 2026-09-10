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

    /* ---------- 1. Primeira linha: tempo total ---------- */
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

    /* ---------- 2. Linhas seguintes: tarefas ---------- */
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

    if (count == 0) {
        fprintf(stderr, "Erro: nenhuma tarefa encontrada no arquivo\n");
        fclose(f);
        return 1;
    }

    *n = count;
    fclose(f);
    return 0;
}

int main(int argc, char *argv[]) 
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