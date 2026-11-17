#include "rogue.h"

// inicializa a pilha de coordenadas
coordStack* initializeCoordStack()
{
    coordStack* stack = malloc(sizeof(coordStack));
    stack->size       = 0;
    stack->top        = NULL;
    return stack;
}

// push na pilha de coordenadas
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

// pop na pilha de coordenadas
void popCoord(coordStack* stack)
{
    coordNode* aux = stack->top;
    stack->top     = stack->top->next;
    stack->size--;
    free(aux);
}

// erase na pilha de coordenadas
void eraseStack(coordStack* stack)
{
    while (stack->size != 0)
    {
        popCoord(stack);
    }
    free(stack);
}

// funcoes da Queue
coordQueue* initializeCoordQueue()
{
    coordQueue* q = malloc(sizeof(coordQueue));
    q->size       = 0;
    q->front      = NULL;
    q->back       = NULL;
    return q;
}

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

coordNode popCoordQueue(coordQueue* q)
{
    coordNode* aux = q->front;
    coordNode  r   = *(aux);
    q->front       = q->front->next;
    q->size--;
    free(aux);
    return r;
}

void eraseCoordQueue(coordQueue* q)
{
    while (q->size > 0)
    {
        popCoordQueue(q);
    }
    free(q);
}

logQueue* initializeLogQueue()
{
    logQueue* q = malloc(sizeof(logQueue));
    q->size  = 0;
    q->front = NULL;
    q->back  = NULL;
    return q;
}

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

char* popLogQueue(logQueue* q)
{
    logNode* aux = q->front;
    logNode  r   = *(aux);
    q->front       = q->front->next;
    q->size--;
    free(aux);
    return r.msg;
}

void eraseLogQueue(logQueue* q)
{
    while (q->size > 0)
    {
        popLogQueue(q);
    }
    free(q);
}
