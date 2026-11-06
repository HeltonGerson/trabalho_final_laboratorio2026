#include <assert.h>
#include <stdio.h>
#include "rogue.h"

// Variaveis globais necessarias para linkagem
Entity*   player = NULL;
Tile**    curr_room = NULL;
WINDOW*   win = NULL;
WINDOW*   win_minimapa = NULL;
MapCell** map = NULL;
int       map_y = 1;
int       map_x = 1;

const int ROOM_WIDTH  = 32;
const int ROOM_HEIGHT = 10;
const int MAP_SIZE    = 3;

int main(void)
{
    printf("Testando inicializacao do mapa 3x3 e nevoa de guerra...\n");

    map = createMap();
    assert(map != NULL);

    setupFloor();

    // Centro deve ser sala 1, visitada e descoberta
    assert(map[1][1].id == 1);
    assert(map[1][1].tiles != NULL);
    assert(map[1][1].visited == true);
    assert(map[1][1].discovered == true);

    // Cardeais devem ser sala 2, nao visitadas, mas descobertas
    assert(map[0][1].id == 2 && map[0][1].discovered == true && map[0][1].visited == false);
    assert(map[2][1].id == 2 && map[2][1].discovered == true && map[2][1].visited == false);
    assert(map[1][0].id == 2 && map[1][0].discovered == true && map[1][0].visited == false);
    assert(map[1][2].id == 2 && map[1][2].discovered == true && map[1][2].visited == false);

    // Cantos devem ser vazios e nao descobertos
    assert(map[0][0].id == 0 && map[0][0].discovered == false && map[0][0].tiles == NULL);
    assert(map[0][2].id == 0 && map[0][2].discovered == false && map[0][2].tiles == NULL);
    assert(map[2][0].id == 0 && map[2][0].discovered == false && map[2][0].tiles == NULL);
    assert(map[2][2].id == 0 && map[2][2].discovered == false && map[2][2].tiles == NULL);

    // Simula transicao para o Norte (map_y = 0, map_x = 1)
    map_y = 0;
    map_x = 1;
    updateFogOfWar();
    assert(map[0][1].visited == true);
    assert(map[0][1].discovered == true);

    // Liberacao de memoria
    freeMap();
    assert(map == NULL);

    printf("Todos os testes do mapa passaram com sucesso!\n");
    return 0;
}
