#include "rogue.h"


/* Liberacao do mapa */
void freeRoom()
{
    for (int y = 0; y < MAP_HEIGHT; ++y)
    {
        free(room[y]);
    }
    free(room);
}

Tile** readRoom(int n)
{
    FILE* fp;
    // o dir é do makefile
    char* file_path = (char *)malloc(20*sizeof(char));
    sprintf(file_path, "data/rooms/%d.txt", n);
    fp = fopen(file_path, "r");
    Tile** room = malloc(sizeof(Tile*) * MAP_HEIGHT);

    for (int y = 0; y < MAP_HEIGHT; ++y)
    {
        room[y] = malloc(sizeof(Tile) * MAP_WIDTH);
        for (int x = 0; x < MAP_WIDTH; ++x)
        {

            room[y][x].ch = '\n';
            while (room[y][x].ch == '\n')
                room[y][x].ch = fgetc(fp);

            if (room[y][x].ch == '.')
                room[y][x].walkable = true;
            else
                room[y][x].walkable = false;
        }
    }
    return room;
}
