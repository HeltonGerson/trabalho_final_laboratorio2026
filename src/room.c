#include "rogue.h"


/* Liberacao da room */
void freeRoom()
{
    for (int y = 0; y < ROOM_HEIGHT; ++y)
    {
        free(room[y]);
    }
    free(room);
}

/* Le uma room do data/rooms */
Tile** readRoom(int n)
{
    FILE* fp;
    // o dir é do makefile
    char* file_path = (char *)malloc(20*sizeof(char));
    sprintf(file_path, "data/rooms/%d.txt", n);
    fp = fopen(file_path, "r");
    
    Tile** room = malloc(sizeof(Tile*) * ROOM_HEIGHT);

    for (int y = 0; y < ROOM_HEIGHT; ++y)
    {
        room[y] = malloc(sizeof(Tile) * ROOM_WIDTH);
        for (int x = 0; x < ROOM_WIDTH; ++x)
        {
            room[y][x].walkable = false;
            room[y][x].ch = '\n';
            while (room[y][x].ch == '\n')
                room[y][x].ch = fgetc(fp);
            // o entorno
            if (x == 0 || x == ROOM_WIDTH - 1)
                room[y][x].ch = ACS_VLINE;
            if (y == 0 || y == ROOM_HEIGHT - 1)
                room[y][x].ch = ACS_HLINE;
            if (room[y][x].ch == '.')
                room[y][x].walkable = true;
        }
    }
    // Definindo os caracteres especiais do canto
    room[0][0].ch = ACS_ULCORNER;
    room[0][ROOM_WIDTH-1].ch = ACS_URCORNER;
    room[ROOM_HEIGHT-1][0].ch = ACS_LLCORNER;
    room[ROOM_HEIGHT-1][ROOM_WIDTH-1].ch = ACS_LRCORNER;
    for (int i = 0; i < 2; i++)
        room[(ROOM_HEIGHT)/2 - 1 + i][0].ch = ACS_LARROW;
    for (int i = 0; i < 2; i++)
        room[(ROOM_HEIGHT)/2 - 1 + i][ROOM_WIDTH-1].ch = ACS_RARROW;
    for (int i = 0; i < 4; i++)
        room[0][(ROOM_WIDTH)/2 - 2 + i].ch = ACS_UARROW;
    for (int i = 0; i < 4; i++)
        room[ROOM_HEIGHT-1][(ROOM_WIDTH)/2 -2 + i].ch = ACS_DARROW;
    return room;
}