#include "rogue.h"
#include <curses.h>
#include <stdlib.h>

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
        node->below = NULL;
    }
    else
    {
        node->below = stack->top;
    }
    stack->top = node;
    stack->size++;
}

// pop na pilha de coordenadas
void popCoord(coordStack* stack)
{
    coordNode* aux = stack->top;
    stack->top     = stack->top->below;
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
