#include "rogue.h"
#include <stdlib.h>


coordStack* initializeCoordStack(){
    coordStack *stack = malloc(sizeof(coordStack));
    stack->size = 0;
    stack->top = NULL;
    return stack;
}

void pushCoord(int x, int y, coordStack *stack){
    coordNode *node = malloc(sizeof(coordNode));
    if(node == NULL){
        printw("Erro ao alocar o nó da stack em structs.c");
        exit(1);
    }
    node->coord.x = x;
    node->coord.y = y;
    if(stack->size == 0){
        node->below = NULL;
    }else{
        node->below = stack->top;
    }
    stack->top = node;
    stack->size++;
}

void popCoord(coordStack *stack){
    coordNode *aux = stack->top;
    stack->top = stack->top->below;
    stack->size--;
    free(aux);
}

void eraseStack(coordStack *stack){
    while (stack->size != 0) {
        popCoord(stack);
    }
    free(stack);
}