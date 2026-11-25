#include "rogue.h"

/*
 * Cria uma pilha vazia de posições na memória (como uma pilha de pratos: o
 * último que entra é o primeiro que sai).
 * Onde é chamada: setupFloor (src/map.c).
 */
coordStack* initializeCoordStack()
{
    coordStack* stack = malloc(sizeof(coordStack));
    stack->size       = 0;
    stack->top        = NULL;
    return stack;
}

/*
 * Guarda uma posição (x, y) no topo da pilha.
 * Onde é chamada: setupFloor (src/map.c).
 */
void pushCoord(int x, int y, coordStack* stack)
{
    coordNode* node = malloc(sizeof(coordNode));
    if (node == NULL)
    {
        endwin();
        fprintf(stderr, "Erro ao alocar o nó da stack em structs.c");
        exit(1);
    }
    node->coord.x = x;
    node->coord.y = y;
    if (stack->size == 0)
    {
        node->next = NULL;
    }
    else
    {
        node->next = stack->top;
    }
    stack->top = node;
    stack->size++;
}

/*
 * Tira a posição que está no topo da pilha e libera o espaço dela na memória.
 * Onde é chamada: setupFloor (src/map.c) e eraseStack (src/structs.c).
 */
void popCoord(coordStack* stack)
{
    coordNode* aux = stack->top;
    stack->top     = stack->top->next;
    stack->size--;
    free(aux);
}

/*
 * Esvazia tudo o que sobrou na pilha e limpa a pilha inteira da memória.
 * Onde é chamada: setupFloor (src/map.c).
 */
void eraseStack(coordStack* stack)
{
    while (stack->size != 0)
    {
        popCoord(stack);
    }
    free(stack);
}

/*
 * Cria uma fila vazia de posições na memória (o primeiro que entra é o primeiro
 * que sai).
 * Onde é chamada: chasePlayer (src/entity.c).
 */
coordQueue* initializeCoordQueue()
{
    coordQueue* q = malloc(sizeof(coordQueue));
    q->size       = 0;
    q->front      = NULL;
    q->back       = NULL;
    return q;
}

/*
 * Coloca uma nova posição (x, y) no final da fila.
 * Onde é chamada: chasePlayer (src/entity.c).
 */
void pushCoordQueue(int x, int y, coordQueue* q)
{
    coordNode* new = malloc(sizeof(coordNode));
    if (new == NULL)
    {
        endwin();
        fprintf(stderr, "Erro ao alocar o nó da queue em structs.c");
        exit(1);
    }
    new->coord.x = x;
    new->coord.y = y;
    new->next    = NULL;

    if (q->size == 0)
    {
        q->front = new;
    }
    else
    {
        q->back->next = new;
    }
    q->back = new;
    q->size++;
}

/*
 * Tira o primeiro item da fila, limpa o espaço dele na memória e devolve uma
 * cópia com a posição guardada.
 * Onde é chamada: eraseCoordQueue (src/structs.c) e chasePlayer (src/entity.c).
 */
coordNode popCoordQueue(coordQueue* q)
{
    coordNode* aux = q->front;
    coordNode  r   = *(aux);
    q->front       = q->front->next;
    q->size--;
    free(aux);
    return r;
}

/*
 * Esvazia todos os itens da fila de posições e limpa a fila inteira da memória.
 * Onde é chamada: chasePlayer (src/entity.c).
 */
void eraseCoordQueue(coordQueue* q)
{
    while (q->size > 0)
    {
        popCoordQueue(q);
    }
    free(q);
}

/*
 * Cria uma fila vazia na memória para guardar as mensagens que vão aparecer
 * para o jogador.
 * Onde é chamada: main (src/main.c) e tests/test_floor.c.
 */
logQueue* initializeLogQueue()
{
    logQueue* q = malloc(sizeof(logQueue));
    if (!q)
        return NULL;

    q->size  = 0;
    q->front = NULL;
    q->back  = NULL;
    return q;
}

/*
 * Coloca uma nova mensagem no final da fila de mensagens.
 * Onde é chamada: main (src/main.c) e dealDamage, fireRangedAttack, movePlayer,
 * chasePlayer, actRangedEnemy e actBossI (src/entity.c).
 */
void pushLogQueue(char* msg, logQueue* q)
{
    if (!q)
        return;

    logNode* new = malloc(sizeof(logNode));
    if (new == NULL)
    {
        endwin();
        fprintf(stderr, "Erro ao alocar o nó da queue em structs.c");
        exit(1);
    }
    new->msg  = msg;
    new->next = NULL;

    if (q->size == 0)
    {
        q->front = new;
    }
    else
    {
        q->back->next = new;
    }
    q->back = new;
    q->size++;
}

/*
 * Tira a primeira mensagem da fila, limpa o espaço dela na memória e devolve o
 * texto guardado (ou nada se a fila estiver vazia).
 * Onde é chamada: eraseLogQueue (src/structs.c) e drawLogs (src/draw.c).
 */
char* popLogQueue(logQueue* q)
{
    if (!q || !q->front)
        return NULL;

    logNode* aux = q->front;
    char*    msg = aux->msg;
    q->front     = q->front->next;
    q->size--;
    if (q->size == 0)
        q->back = NULL;
    free(aux);
    return msg;
}

/*
 * Esvazia todas as mensagens da fila e limpa a fila inteira da memória.
 * Onde é chamada: cleanupRun (src/engine.c) e tests/test_floor.c.
 */
void eraseLogQueue(logQueue* q)
{
    if (!q)
        return;

    while (q->size > 0)
    {
        popLogQueue(q);
    }
    free(q);
}
