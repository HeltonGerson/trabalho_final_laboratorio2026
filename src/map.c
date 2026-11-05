#include "rogue.h"
int** createMap()
{
    int** m = malloc(sizeof(int*)*MAP_SIZE);
    for (int i = 0; i < MAP_SIZE; ++i)
    {
        m[i] = calloc(MAP_SIZE, sizeof(int));
    }
    return m;
}

void fillMap()
{
    for (int i = 0; i < MAP_SIZE; i++)
    {
        for (int j = 0; j < MAP_SIZE; j++)
        {
            map[i][j] = i;
            printw("%d", map[i][j]);
        }
        printw("\n");
    }
}

void freeMap()
{
    for (int y = 0; y < MAP_SIZE; ++y)
    {
        free(map[y]);
    }
    free(map);
}
