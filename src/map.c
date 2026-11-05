#include "rogue.h"

/* alocacao dinamica do mapa */
Tile** createMapTiles()
{
    Tile** tiles = malloc(MAP_HEIGHT * sizeof(Tile*));

    for (int y = 0; y < MAP_HEIGHT; ++y)
    {
        tiles[y] = malloc(MAP_WIDTH * sizeof(Tile));
        for (int x = 0; x < MAP_WIDTH; ++x)
        {
            tiles[y][x].ch = '#';
            tiles[y][x].walkable = false;
        }
    }

    return tiles;
}

Position setupMap()
{
    Position start_pos = {2,10};
    for (int y = 2; y < 6; ++y)
    {
        for (int x = 4; x < 15; ++x)
        {
            map[y][x].ch = '.';
            map[y][x].walkable = true;
        }
    }

    return start_pos;
}


/* Liberacao do mapa */
void freeMap()
{
    for (int y = 0; y < MAP_HEIGHT; ++y)
    {
        free(map[y]);
    }
    free(map);
}
