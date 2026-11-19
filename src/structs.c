#include "rogue.h"

/*
 * Aloca e inicializa uma pilha vazia de coordenadas (coordStack).
 * Chamada por: setupFloor (src/map.c).
 */
coordStack* initializeCoordStack()
{
    coordStack* stack = malloc(sizeof(coordStack));
    stack->size       = 0;
    stack->top        = NULL;
    return stack;
}

/*
 * Aloca um coordNode com as coordenadas (x, y) e o empilha no topo de stack.
 * Chamada por: setupFloor (src/map.c).
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
 * Remove e libera o nó do topo da pilha stack, decrementando stack->size.
 * Chamada por: setupFloor (src/map.c) e eraseStack (src/structs.c).
 */
void popCoord(coordStack* stack)
{
    coordNode* aux = stack->top;
    stack->top     = stack->top->next;
    stack->size--;
    free(aux);
}

/*
 * Desempilha todos os nós restantes com popCoord e libera a struct coordStack.
 * Chamada por: setupFloor (src/map.c).
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
 * Aloca e inicializa uma fila FIFO vazia de coordenadas (coordQueue).
 * Chamada por: chasePlayer (src/entity.c).
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
 * Aloca um coordNode com (x, y) e o insere no final (back) da fila q.
 * Chamada por: chasePlayer (src/entity.c).
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
 * Remove e libera o nó da frente (front) da fila q e retorna uma cópia do nó.
 * Chamada por: eraseCoordQueue (src/structs.c) e chasePlayer (src/entity.c).
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
 * Esvazia todos os nós da fila q via popCoordQueue e libera a struct coordQueue.
 * Chamada por: chasePlayer (src/entity.c).
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
 * Aloca e inicializa uma fila FIFO vazia para mensagens de log (logQueue).
 * Chamada por: main (src/main.c) e tests/test_floor.c.
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
 * Aloca um logNode apontando para msg e o enfileira no final da fila q.
 * Chamada por: fireRangedAttack, movePlayer, chasePlayer, actRangedEnemy e
 * actBoss (src/entity.c).
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
    new->msg = msg;
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
 * Remove e libera o nó da frente da fila q e retorna o ponteiro msg armazenado
 * (ou NULL se a fila estiver vazia).
 * Chamada por: eraseLogQueue (src/structs.c) e drawLogs (src/draw.c).
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
 * Esvazia todos os nós da fila q via popLogQueue e libera a struct logQueue.
 * Chamada por: cleanupRun (src/engine.c) e tests/test_floor.c.
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
