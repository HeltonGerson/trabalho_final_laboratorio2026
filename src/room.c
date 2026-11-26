#include "rogue.h"

/*
 * Coloca os símbolos de porta (setinhas para esquerda, direita, cima e baixo)
 * no meio das quatro paredes da sala.
 * Onde é chamada: readRoom e roomDoors (src/room.c).
 */
void fillDoors(Tile** room)
{
    for (int i = 0; i < 3; i++)
        room[(ROOM_HEIGHT) / 2 - 1 + i][0].ch = ACS_LARROW;
    for (int i = 0; i < 3; i++)
        room[(ROOM_HEIGHT) / 2 - 1 + i][ROOM_WIDTH - 1].ch = ACS_RARROW;
    for (int i = 1; i < 4; i++)
        room[0][(ROOM_WIDTH) / 2 - 2 + i].ch = ACS_UARROW;
    for (int i = 1; i < 4; i++)
        room[ROOM_HEIGHT - 1][(ROOM_WIDTH) / 2 - 2 + i].ch = ACS_DARROW;
}

/*
 * Abre uma pasta e conta quantos arquivos de modelo de sala existem lá dentro
 * (ignorando arquivos ocultos que começam com ponto), ou avisa com -1 se não
 * conseguir abrir a pasta.
 * Onde é chamada: setRoom (src/room.c) e tests/test_floor.c.
 */
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
/*
 * Confere se a posição no mapa e o tipo de sala são válidos, sorteia um dos
 * modelos daquela pasta, monta o código da sala, carrega o desenho dela e
 * coloca os inimigos e itens dentro.
 * Onde é chamada: setupFloor (src/map.c) e tests/test_floor.c.
 */
int setRoom(int x, int y, int room_type)
{
    if (x >= 0 && x < MAP_WIDTH && y >= 0 && y < MAP_HEIGHT && room_type >= 0 && room_type <= 20)
    {
        int         type     = 0;
        const char* room_dir = NULL;

        if (room_type == 0)
        {
            type     = 0;
            room_dir = "start";
        }
        else if (room_type == 1)
        {
            type     = 1;
            room_dir = "boss";
        }
        else if (room_type == 2 || room_type == 3)
        {
            type     = 2;
            room_dir = "items";
        }
        else
        {
            type     = 3;
            room_dir = "normal";
        }

        char dir_path[50];
        snprintf(dir_path, sizeof(dir_path), "data/rooms/%s", room_dir);
        int total_rooms = countTxtFiles(dir_path);

        if (total_rooms <= 0)
            return 1;

        int room_n  = rand() % total_rooms + 1;
        int room_id = type * ROOM_ID_BASE + room_n;

        Tile** tiles = readRoom(room_id);
        if (!tiles)
            return 1;

        map[y][x].id    = room_id;
        map[y][x].tiles = tiles;
        if (spawnRoomEntities(&map[y][x]) != 0)
        {
            freeRoomTiles(tiles);
            map[y][x].id    = 0;
            map[y][x].tiles = NULL;
            return 1;
        }
        return 0;
    }
    return 1;
}

/*
 * Descobre qual arquivo de texto abrir pelo código da sala, lê o desenho da
 * sala do arquivo, marca onde dá para andar (.), coloca as paredes e os cantos
 * da borda e adiciona as portas.
 * Onde é chamada: setRoom (src/room.c) e continueGame (src/engine.c).
 */
Tile** readRoom(int room_id)
{
    if (room_id <= 0)
        return NULL;

    int type   = room_id / ROOM_ID_BASE;
    int room_n = room_id % ROOM_ID_BASE;

    if (room_n <= 0)
        return NULL;

    const char* room_type = NULL;
    if (type == 0)
        room_type = "start";
    else if (type == 1)
        room_type = "boss";
    else if (type == 2)
        room_type = "items";
    else if (type == 3)
        room_type = "normal";
    else
        return NULL;

    char file_path[128];
    snprintf(file_path, sizeof(file_path), "data/rooms/%s/%d.txt", room_type, room_n);

    FILE* fp = fopen(file_path, "r");
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
            // Desenha as paredes ao redor da sala
            if (x == 0 || x == ROOM_WIDTH - 1)
                room[y][x].ch = ACS_VLINE;
            if (y == 0 || y == ROOM_HEIGHT - 1)
                room[y][x].ch = ACS_HLINE;
            if (room[y][x].ch == '.')
                room[y][x].walkable = true;
        }
    }
    fclose(fp);

    // Coloca os desenhos certos nos quatro cantos da sala
    room[0][0].ch                            = ACS_ULCORNER;
    room[0][ROOM_WIDTH - 1].ch               = ACS_URCORNER;
    room[ROOM_HEIGHT - 1][0].ch              = ACS_LLCORNER;
    room[ROOM_HEIGHT - 1][ROOM_WIDTH - 1].ch = ACS_LRCORNER;
    fillDoors(room);
    return room;
}

/*
 * Limpa da memória todos os quadradinhos desenhados de uma sala.
 * Onde é chamada: setRoom (src/room.c) e freeMap (src/map.c).
 */
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

/*
 * Atualiza as portas da sala atual: fecha com parede normal onde não existe
 * sala vizinha e tranca com '#' as portas que levam a outras salas enquanto
 * ainda houver inimigos vivos ali dentro.
 * Onde é chamada: main (src/main.c), newFloor (src/map.c), movePlayer e
 * dealDamage (src/entity.c), e tests/test_floor.c.
 */
void roomDoors(void)
{
    if (!curr_room)
        return;

    bool locked = roomHasAliveEnemies();
    fillDoors(curr_room);
    for (int y = 0; y < ROOM_HEIGHT; ++y)
    {
        if (curr_room[y][ROOM_WIDTH - 1].ch == ACS_RARROW)
        {
            if (map_x + 1 >= MAP_SIZE || map[map_y][map_x + 1].id == 0)
            {
                curr_room[y][ROOM_WIDTH - 1].ch = ACS_VLINE;
            }
            else if (locked)
            {
                curr_room[y][ROOM_WIDTH - 1].ch = '#';
            }
        }
        if (curr_room[y][0].ch == ACS_LARROW)
        {
            if (map_x - 1 < 0 || map[map_y][map_x - 1].id == 0)
            {
                curr_room[y][0].ch = ACS_VLINE;
            }
            else if (locked)
            {
                curr_room[y][0].ch = '#';
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
            else if (locked)
            {
                curr_room[ROOM_HEIGHT - 1][x].ch = '#';
            }
        }
        if (curr_room[0][x].ch == ACS_UARROW)
        {
            if (map_y - 1 < 0 || map[map_y - 1][map_x].id == 0)
            {
                curr_room[0][x].ch = ACS_HLINE;
            }
            else if (locked)
            {
                curr_room[0][x].ch = '#';
            }
        }
    }
}
