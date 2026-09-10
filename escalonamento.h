#ifndef ESCALONAMENTO_H
#define ESCALONAMENTO_H

#define MAX_TASKS 32
#define MAX_NOME 32

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
}Task;

#endif