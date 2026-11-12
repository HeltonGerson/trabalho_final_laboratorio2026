#include "rogue.h"
#include <curses.h>

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

// Conta quantos arquivos existem no diretório informado pra saber quantas salas tem
int countTxtFiles(const char* dir_path)
{
    DIR* dir = opendir(dir_path);
    if (!dir)
    {
        return -1;
    }

    struct dirent* entry;
    int            count = 0;

    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_name[0] == '.')
            continue;
        count++;
    }

    closedir(dir);
    return count;
}
// faz as verificações para ver se os parâmetros passados são válidos para uma sala
int setRoom(int x, int y, int room_type)
{
    if (x >= 0 && x < MAP_WIDTH && y >= 0 && y < MAP_HEIGHT && room_type >= 0 && room_type <= 20)
    {
        Tile** tiles = readRoom(room_type);
        if (!tiles)
            return 1;

        map[y][x].id    = (room_type == 0) ? -1 : room_type;
        map[y][x].tiles = tiles;
        return 0;
    }
    return 1;
}

/* Le uma room do data/rooms */
Tile** readRoom(int type)
{
    FILE* fp;
    char  room_type[8];
    int   room_n = 0;
    if (type == 0)
        strcpy(room_type, "start");
    else if (type == 1)
        strcpy(room_type, "boss");
    else if (type == 2 || type == 3)
        strcpy(room_type, "items");
    else if (type >= 4 && type <= 20)
        strcpy(room_type, "normal");
    else
        return NULL;

    char dir_path[50];
    snprintf(dir_path, sizeof(dir_path), "data/rooms/%s", room_type);
    int total_rooms = countTxtFiles(dir_path);

    if (total_rooms <= 0)
        return NULL;

    char file_path[128];
    room_n = rand() % total_rooms + 1;
    snprintf(file_path, sizeof(file_path), "%s/%d.txt", dir_path, room_n);
    fp = fopen(file_path, "r");
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
