#include "rogue.h"
#include <assert.h>
#include <stdio.h>

// Variaveis globais necessarias para linkagem
Entity*   player       = NULL;
Tile**    curr_room    = NULL;
WINDOW*   win          = NULL;
WINDOW*   win_minimapa = NULL;
WINDOW*   win_hud      = NULL;
MapCell** map          = NULL;
logQueue* logs         = NULL;
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

    // Teste 2: Contagem exata dos 15 templates de sala
    int total_templates = countTxtFiles("data/rooms/start") + countTxtFiles("data/rooms/boss") +
                          countTxtFiles("data/rooms/items") + countTxtFiles("data/rooms/normal");
    assert(total_templates == 15);

    // Teste 3: Conversao de caracteres E/A/B/I em entidades e piso '.'
    map   = createMap();
    map_x = 3;
    map_y = 3;
    assert(setRoom(3, 3, 1) == 0); // sala boss possui 'B'
    curr_room = map[3][3].tiles;

    bool found_boss = false;
    for (int i = 0; i < map[3][3].entity_count; ++i)
    {
        Entity* e = &map[3][3].entities[i];
        assert(e->active == true);
        assert(curr_room[e->pos.y][e->pos.x].ch == '.');
        assert(curr_room[e->pos.y][e->pos.x].walkable == true);
        if (e->type == ENTITY_BOSS)
        {
            found_boss = true;
            assert(e->hp == 50 && e->max_hp == 50 && e->attack == 20 && e->defense == 10);
        }
    }
    assert(found_boss == true);

    // Teste 4: Inicializacao do player e coleta de item (desaparece do mapa)
    Position start_pos = {5, 5};
    player             = createPlayer(start_pos);
    assert(player != NULL);
    assert(player->hp == 10 && player->attack == 4 && player->defense == 0 && player->active == true);

    assert(setRoom(2, 3, 4) == 0); // sala normal limpa para testar entidades controladas
    map_x     = 2;
    map_y     = 3;
    curr_room = map[3][2].tiles;

    freeRoomEntities(&map[3][2]);
    map[3][2].entities     = malloc(sizeof(Entity) * 3);
    map[3][2].entity_count = 3;
    assert(map[3][2].entities != NULL);

    initEntity(&map[3][2].entities[0], ENTITY_ITEM, 6, 5);
    map[3][2].entities[0].hp      = 0;
    map[3][2].entities[0].max_hp  = 0;
    map[3][2].entities[0].attack  = 0;
    map[3][2].entities[0].defense = 0;
    initEntity(&map[3][2].entities[1], ENTITY_ENEMY_MELEE, 7, 5);
    initEntity(&map[3][2].entities[2], ENTITY_ENEMY_RANGED, 5, 6);
    curr_room[5][6].ch       = '.';
    curr_room[5][6].walkable = true;
    curr_room[5][7].ch       = '.';
    curr_room[5][7].walkable = true;
    curr_room[6][5].ch       = '.';
    curr_room[6][5].walkable = true;

    // Move para o item em (6, 5): item desativa e player avanca
    movePlayer((Position){6, 5});
    assert(map[3][2].entities[0].active == false);
    assert(player->pos.x == 6 && player->pos.y == 5);

    // Teste 5: Combate corpo a corpo contra Goblin Melee (8/3/0) em (7, 5)
    // 1o golpe: dano 4, HP 8 -> 4, player nao avanca
    movePlayer((Position){7, 5});
    assert(map[3][2].entities[1].hp == 4);
    assert(map[3][2].entities[1].active == true);
    assert(player->pos.x == 6 && player->pos.y == 5);

    // 2o golpe: dano 4, HP 4 -> 0, inimigo desativado
    movePlayer((Position){7, 5});
    assert(map[3][2].entities[1].hp == 0);
    assert(map[3][2].entities[1].active == false);
    assert(player->pos.x == 6 && player->pos.y == 5);

    // 3o movimento: espaco livre, player avanca para (7, 5)
    movePlayer((Position){7, 5});
    assert(player->pos.x == 7 && player->pos.y == 5);

    // Teste 6: Combate contra Boss (50/20/10) garante dano minimo 1
    Entity boss;
    initEntity(&boss, ENTITY_BOSS, 10, 5);
    attackEntity(player, &boss);
    assert(boss.hp == 49 && boss.active == true);

    free(player);
    player = NULL;
    freeMap();
    assert(map == NULL);

    printf("Todos os testes do mapa procedural e entidades passaram com sucesso!\n");
    return 0;
}
