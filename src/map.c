#include "rogue.h"
#include <curses.h>

/*
 * Aloca dinamicamente a matriz MAP_SIZE x MAP_SIZE de MapCell, inicializando cada
 * célula com zeros via calloc.
 * Chamada por: main (src/main.c) e tests/test_floor.c.
 */
MapCell** createMap(void)
{
    MapCell** m = malloc(sizeof(MapCell*) * MAP_HEIGHT);
    for (int i = 0; i < MAP_HEIGHT; ++i)
    {
        m[i] = calloc(MAP_WIDTH, sizeof(MapCell));

        return m;
    }

}
    /*
     * Gera proceduralmente o andar via busca em profundidade (DFS) usando coordStack:
     * cria a sala inicial em (start_pos_x, start_pos_y), expande até number_of_rooms
     * garantindo ao menos uma sala de itens e uma de boss, e atualiza a névoa de guerra.
     * Chamada por: main (src/main.c) e tests/test_floor.c.
     */
    void setupFloor(int start_pos_x, int start_pos_y, int number_of_rooms)
    {
        coordStack* st = initializeCoordStack();
        if (setRoom(start_pos_x, start_pos_y, 0))
        {
            endwin();
            fprintf(stderr, "Erro ao preencher a sala inicial");
            exit(1);
        }
        pushCoord(start_pos_x, start_pos_y, st);
        number_of_rooms--;

        int  dx[4]  = {-1, 0, 1, 0};
        int  dy[4]  = {0, 1, 0, -1};
        bool i_flag = 0;
        bool b_flag = 0;

        while (number_of_rooms > 0 && st->top != NULL)
        {
            int free_dirs[4];
            int count_dirs = 0;

            for (int i = 0; i < 4; i++)
            {
                int nx = st->top->coord.x + dx[i];
                int ny = st->top->coord.y + dy[i];

                if (nx >= 0 && nx < MAP_WIDTH && ny >= 0 && ny < MAP_HEIGHT && map[ny][nx].id == 0)
                {
                    free_dirs[count_dirs] = i;
                    count_dirs++;
                }
            }

            if (count_dirs == 0)
            {
                popCoord(st);
                continue;
            }
            int choose_dir = free_dirs[rand() % count_dirs];
            int nx         = st->top->coord.x + dx[choose_dir];
            int ny         = st->top->coord.y + dy[choose_dir];
            int room_type  = rand() % 20 + 1;
            if (room_type == 1 && b_flag == 1)
                room_type = 4;

            if (number_of_rooms == 2 && !i_flag)
                room_type = 2;
            else if (number_of_rooms == 1 && !b_flag)
                room_type = 1;

            if (room_type == 2 || room_type == 3)
                i_flag = 1;
            else if (room_type == 1)
                b_flag = 1;

            if (setRoom(nx, ny, room_type))
            {
                endwin();
                fprintf(stderr, "Erro ao preencher uma sala");
                exit(1);
            }
            pushCoord(nx, ny, st);
            number_of_rooms--;
        }
        updateFogOfWar();
        eraseStack(st);
    }

    /*
     * Marca a sala atual (map_x, map_y) como visitada e descoberta e revela as salas
     * adjacentes cardeais válidas como descobertas para o minimapa.
     * Chamada por: setupFloor (src/map.c), movePlayer (src/entity.c) e tests/test_floor.c.
     */
    void updateFogOfWar(void)
    {
        if (!map || map[map_y][map_x].id == 0)
            return;

        map[map_y][map_x].visited = true;

        for (int y = 0; y < MAP_HEIGHT; ++y)
        {
            for (int x = 0; x < MAP_WIDTH; ++x)
            {
                if (map[y][x].id == 0 || !map[y][x].visited)
                    continue;

                map[y][x].discovered = true;

                // Adjacentes conectadas a qualquer sala visitada reveladas como ?
                if (y > 0 && map[y - 1][x].id != 0)
                    map[y - 1][x].discovered = true;
                if (y + 1 < MAP_HEIGHT && map[y + 1][x].id != 0)
                    map[y + 1][x].discovered = true;
                if (x > 0 && map[y][x - 1].id != 0)
                    map[y][x - 1].discovered = true;
                if (x + 1 < MAP_WIDTH && map[y][x + 1].id != 0)
                    map[y][x + 1].discovered = true;
            }
        }
    }

    /*
     * Percorre toda a matriz map, libera os tiles e entidades de cada MapCell, libera
     * as linhas e o ponteiro principal e redefine map para NULL.
     * Chamada por: cleanupRun (src/engine.c) e tests/test_floor.c.
     */
    void freeMap(void)
    {
        if (!map)
            return;

        for (int y = 0; y < MAP_SIZE; ++y)
        {
            for (int x = 0; x < MAP_SIZE; ++x)
            {
                if (map[y][x].tiles)
                {
                    freeRoomTiles(map[y][x].tiles);
                    map[y][x].tiles = NULL;
                }
                freeRoomEntities(&map[y][x]);
            }
            free(map[y]);
        }
        free(map);
        map = NULL;
    }
