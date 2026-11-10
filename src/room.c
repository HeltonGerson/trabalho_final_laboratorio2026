#include "rogue.h"

void fillDoors(Tile** room)
{
    for (int i = 0; i < 2; i++)
        room[(ROOM_HEIGHT) / 2 - 1 + i][0].ch = ACS_LARROW;
    for (int i = 0; i < 2; i++)
        room[(ROOM_HEIGHT) / 2 - 1 + i][ROOM_WIDTH - 1].ch = ACS_RARROW;
    for (int i = 0; i < 4; i++)
        room[0][(ROOM_WIDTH) / 2 - 2 + i].ch = ACS_UARROW;
    for (int i = 0; i < 4; i++)
        room[ROOM_HEIGHT - 1][(ROOM_WIDTH) / 2 - 2 + i].ch = ACS_DARROW;
}

int setRoom(int x, int y, int choosed_room)
{
    if (x >= 0 && x < MAP_WIDTH && y >= 0 && y < MAP_HEIGHT &&
        choosed_room >= 1 && choosed_room <= 4)
    {
        Tile** tiles = readRoom(choosed_room);
        if (!tiles)
            return 1;

        map[y][x].id    = choosed_room;
        map[y][x].tiles = tiles;
        return 0;
    }
    return 1;
}

/* Liberacao dos tiles de uma room */
void freeRoomTiles(Tile** tiles)
{
    if (!tiles)
        return;
    for (int y = 0; y < ROOM_HEIGHT; ++y)
    {
        free(tiles[y]);
    }
    free(tiles);
}

/* Le uma room do data/rooms */
Tile** readRoom(int n)
{
    FILE* fp;
    // o dir é do makefile
    char* file_path = (char*)malloc(32 * sizeof(char));
    if (!file_path)
        return NULL;

    sprintf(file_path, "data/rooms/normal/%d.txt", n);
    fp = fopen(file_path, "r");
    free(file_path);

    if (!fp)
        return NULL;

    Tile** room = malloc(sizeof(Tile*) * ROOM_HEIGHT);

    for (int y = 0; y < ROOM_HEIGHT; ++y)
    {
        room[y] = malloc(sizeof(Tile) * ROOM_WIDTH);
        for (int x = 0; x < ROOM_WIDTH; ++x)
        {
            room[y][x].walkable = false;
            int input_ch        = '\n';
            while (input_ch == '\n')
                input_ch = fgetc(fp);

            room[y][x].ch = (chtype)input_ch;
            // o entorno
            if (x == 0 || x == ROOM_WIDTH - 1)
                room[y][x].ch = ACS_VLINE;
            if (y == 0 || y == ROOM_HEIGHT - 1)
                room[y][x].ch = ACS_HLINE;
            if (room[y][x].ch == '.')
                room[y][x].walkable = true;
        }
    }
    fclose(fp);

    // Definindo os caracteres especiais do canto
    room[0][0].ch                            = ACS_ULCORNER;
    room[0][ROOM_WIDTH - 1].ch               = ACS_URCORNER;
    room[ROOM_HEIGHT - 1][0].ch              = ACS_LLCORNER;
    room[ROOM_HEIGHT - 1][ROOM_WIDTH - 1].ch = ACS_LRCORNER;
    fillDoors(room);
    return room;
}

/* Verificar se as portas da room atual */
void roomDoors(void)
{
    fillDoors(curr_room);
    for (int y = 0; y < ROOM_HEIGHT; ++y)
    {
        if (curr_room[y][ROOM_WIDTH - 1].ch == ACS_RARROW)
        {
            if (map_x + 1 >= MAP_SIZE || map[map_y][map_x + 1].id == 0)
            {
                curr_room[y][ROOM_WIDTH - 1].ch = ACS_VLINE;
            }
        }
        if (curr_room[y][0].ch == ACS_LARROW)
        {
            if (map_x - 1 < 0 || map[map_y][map_x - 1].id == 0)
            {
                curr_room[y][0].ch = ACS_VLINE;
            }
        }
    }
    for (int x = 0; x < ROOM_WIDTH; ++x)
    {
        if (curr_room[ROOM_HEIGHT - 1][x].ch == ACS_DARROW)
        {
            if (map_y + 1 >= MAP_SIZE || map[map_y + 1][map_x].id == 0)
            {
                curr_room[ROOM_HEIGHT - 1][x].ch = ACS_HLINE;
            }
        }
        if (curr_room[0][x].ch == ACS_UARROW)
        {
            if (map_y - 1 < 0 || map[map_y - 1][map_x].id == 0)
            {
                curr_room[0][x].ch = ACS_HLINE;
            }
        }
    }
}
