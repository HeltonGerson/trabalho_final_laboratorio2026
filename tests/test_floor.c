#include "rogue.h"
#include <assert.h>
#include <stdio.h>

// Variaveis globais necessarias para linkagem
Entity*   player       = NULL;
Tile**    curr_room    = NULL;
WINDOW*   win          = NULL;
WINDOW*   win_minimapa = NULL;
MapCell** map          = NULL;
int       map_y        = 3;
int       map_x        = 3;

const int ROOM_WIDTH  = 32;
const int ROOM_HEIGHT = 10;
const int MAP_SIZE    = 7;
const int MAP_WIDTH   = 7;
const int MAP_HEIGHT  = 7;

int main(void)
{
    printf("Testando inicializacao do mapa 7x7 procedural e nevoa de guerra...\n");

    map = createMap();
    assert(map != NULL);

    int target_rooms = 10;
    setupFloor(map_x, map_y, target_rooms);

    // Centro (3, 3) deve ser sala inicial (-1), visitada e descoberta
    assert(map[3][3].id == -1);
    assert(map[3][3].tiles != NULL);
    assert(map[3][3].visited == true);
    assert(map[3][3].discovered == true);
    assert(map[3][3].tiles[0][0].ch == ACS_ULCORNER);
    assert(map[3][3].tiles[ROOM_HEIGHT - 1][ROOM_WIDTH - 1].ch == ACS_LRCORNER);

    int count_rooms = 0;
    for (int y = 0; y < MAP_SIZE; ++y)
    {
        for (int x = 0; x < MAP_SIZE; ++x)
        {
            if (map[y][x].id != 0)
            {
                count_rooms++;
                assert(map[y][x].id == -1 || (map[y][x].id >= 1 && map[y][x].id <= 20));
                assert(map[y][x].tiles != NULL);
            }
            else
            {
                assert(map[y][x].tiles == NULL);
                assert(map[y][x].visited == false);
            }
        }
    }

    // A contagem total de salas deve bater com a solicitada
    assert(count_rooms == target_rooms);

    // Vizinhos cardeais de (3, 3) que foram gerados devem estar descobertos e nao visitados
    int dy[4] = {-1, 1, 0, 0};
    int dx[4] = {0, 0, -1, 1};
    int discovered_neighbor_y = -1;
    int discovered_neighbor_x = -1;

    for (int i = 0; i < 4; ++i)
    {
        int ny = 3 + dy[i];
        int nx = 3 + dx[i];
        if (map[ny][nx].id != 0)
        {
            assert(map[ny][nx].discovered == true);
            assert(map[ny][nx].visited == false);
            if (discovered_neighbor_y == -1)
            {
                discovered_neighbor_y = ny;
                discovered_neighbor_x = nx;
            }
        }
    }

    // Deve existir pelo menos um vizinho conectado com mais de 1 sala gerada
    assert(discovered_neighbor_y != -1 && discovered_neighbor_x != -1);

    // Simula transicao para o vizinho descoberto
    map_y = discovered_neighbor_y;
    map_x = discovered_neighbor_x;
    updateFogOfWar();
    assert(map[map_y][map_x].visited == true);
    assert(map[map_y][map_x].discovered == true);

    // Liberacao de memoria
    freeMap();
    assert(map == NULL);

    printf("Todos os testes do mapa procedural passaram com sucesso!\n");
    return 0;
}
