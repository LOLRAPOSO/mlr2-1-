


Readme · MD
# Escalonamento de Tarefas Críticas de Voo
 
Simulador que compara os algoritmos de escalonamento preemptivo **Rate-Monotonic (RATE)**
e **Earliest Deadline First (EDF)** em um cenário de tarefas periódicas com deadline menor
que o período (C ≤ D ≤ P).
 
## Arquivos
 
- **`escalonamento.c`** — contém toda a implementação:
  - `ler_entrada`: abre e valida o arquivo de entrada (tempo total + lista de tarefas),
    checando número de campos, valores numéricos/positivos e a restrição C ≤ D ≤ P.
  - `maior_prioridade`: compara duas tarefas prontas e decide qual tem prioridade
    maior, de acordo com o algoritmo escolhido (menor período para RATE, menor
    deadline absoluto para EDF), com desempate pela ordem de aparição no arquivo.
  - `escolher_tarefa`: percorre todas as tarefas prontas (`remaining > 0`) e retorna
    a de maior prioridade, ou `NULL` se nenhuma estiver pronta (CPU ociosa).
  - `fechar_bloco`: registra um bloco de execução (ou de ociosidade) no log da
    simulação, junto com sua duração e o motivo do encerramento.
  - `simular`: motor principal, executa a simulação instante a instante — processa
    chegadas de novas instâncias, detecta e trata perdas de deadline, escolhe a
    tarefa a rodar em cada unidade de tempo e monta o log de execução.
  - `gravar_saida`: grava o arquivo `.out` final, com as seções `EXECUTION BY`,
    `LOST DEADLINES`, `COMPLETE EXECUTION` e `KILLED`.
  - `main`: valida os argumentos de linha de comando e orquestra a chamada das
    funções acima.
- **`escalonamento.h`** — definições das estruturas usadas (`Task`, `BlocoExecucao`,
  `ResultadoSimulacao`) e protótipos de todas as funções do `escalonamento.c`.
- **`Makefile`** — compila o projeto e gera o executável `scheduler`.
## Como compilar
 
```bash
make
```
 
Gera o executável `scheduler` no diretório atual. Para limpar os arquivos gerados
(executável e `.out`):
 
```bash
make clean
```
 
## Como executar
 
```bash
./scheduler rate arquivo.txt
./scheduler edf arquivo.txt
```
 
O programa não imprime nada em stdout durante a execução normal. O resultado é
gravado em `rate_<login>.out` ou `edf_<login>.out` (onde `<login>` são as iniciais
do e-mail institucional, ex.: `mla@cesar.school` → `mla`).
 
### Formato do arquivo de entrada
 
```
[TEMPO TOTAL]
[NOME] [PERÍODO] [DEADLINE] [BURST]
...
```
 
Exemplo (`voo.txt`):
 
```
100
ATT 20 12 8
NAV 50 30 15
```
 
## Como foi testado
 
- **Exemplo do enunciado (`voo.txt`)**: a saída de `./scheduler rate voo.txt` foi
  comparada linha a linha com o exemplo `EXECUTION BY RATE` fornecido na
  especificação, incluindo os blocos de ociosidade (`idle`), e o resultado bateu
  exatamente.
- **EDF no mesmo cenário**: verificado manualmente que o NAV, que perde deadline no
  RATE, consegue completar todas as instâncias no EDF — evidência usada na análise
  comparativa do relatório.
- **Casos de erro**, cada um verificado quanto à mensagem em stderr, código de saída
  diferente de zero e ausência de arquivo `.out`:
  - número incorreto de argumentos;
  - primeiro argumento diferente de `rate`/`edf`;
  - arquivo de entrada inexistente;
  - linha malformada (campo faltando ou valor não numérico);
  - tarefa violando C ≤ D ≤ P.
## Sistema operacional utilizado
 
<!-- TODO: preencha com o SO real que você usou para desenvolver e testar -->
Ubuntu 24.04 (compilado com gcc, padrão C11).
 
