#ifndef ESCALONAMENTO_H
#define ESCALONAMENTO_H

#define MAX_TASKS 32
#define MAX_NOME 32

typedef struct blocoexec{
    char nome[MAX_NOME];
    int duration;
    char motivo;
} BlocoExecucao;

typedef struct resultadosimulacao {
    BlocoExecucao blocos[64];
    int n_blocos;
} ResultadoSimulacao;

typedef struct task {
    char nome[MAX_NOME];
    int period;
    int deadline;
    int cpu;
    int posicao;
    int remaining;
    int arrival;
    int abs_deadline;
    int completed_count;
    int lost_count;
    int killed_count;
}Task;

#endif