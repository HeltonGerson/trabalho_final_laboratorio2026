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
int       map_y           = 3;
int       map_x           = 3;
int       curr_floor      = 1;
int       number_of_rooms = 0;
bool      game_won        = false;

const int ROOM_WIDTH  = 31;
const int ROOM_HEIGHT = 11;
const int MAP_SIZE    = 7;
const int MAP_WIDTH   = 7;
const int MAP_HEIGHT  = 7;
const int LAST_FLOOR  = 2;

void layoutWindows(void) {}
void destroyWindows(void) {}

static void clearRoomInterior(Tile** tiles)
{
    for (int y = 1; y < ROOM_HEIGHT - 1; ++y)
    {
        for (int x = 1; x < ROOM_WIDTH - 1; ++x)
        {
            tiles[y][x].ch       = '.';
            tiles[y][x].walkable = true;
        }
    }
}

int main(void)
{
    printf("Testando inicializacao do mapa 7x7 procedural e nevoa de guerra...\n");

    logs = initializeLogQueue();
    assert(logs != NULL);

    map = createMap();
    assert(map != NULL);

    int target_rooms = 10;
    setupFloor(map_x, map_y, target_rooms);

    // Centro (3, 3) deve ser sala inicial (tipo 0), visitada e descoberta
    assert(map[3][3].id / ROOM_ID_BASE == 0 && map[3][3].id % ROOM_ID_BASE >= 1);
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
                int type   = map[y][x].id / ROOM_ID_BASE;
                int room_n = map[y][x].id % ROOM_ID_BASE;
                count_rooms++;
                assert(type >= 0 && type <= 3 && room_n >= 1);
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
    int dy[4]                 = {-1, 1, 0, 0};
    int dx[4]                 = {0, 0, -1, 1};
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
            assert(e->hp == 20 && e->max_hp == 20 && e->attack == 5 && e->ranged_attack == 3 &&
                   e->ranged_range == 7 && e->ranged_cooldown == 0 && e->defense == 1);
        }
    }
    assert(found_boss == true);

    // Teste 4: Inicializacao do player e coleta de item (desaparece do mapa)
    Position start_pos = {5, 5};
    player             = createPlayer(start_pos);
    assert(player != NULL);
    assert(player->hp == 10 && player->max_hp == 10 && player->attack == 4 &&
           player->ranged_attack == 2 && player->ranged_range == 10 &&
           player->ranged_cooldown == 0 && player->defense == 0 && player->active == true);

    assert(setRoom(2, 3, 4) == 0); // sala normal limpa para testar entidades controladas
    map_x     = 2;
    map_y     = 3;
    curr_room = map[3][2].tiles;
    clearRoomInterior(curr_room);

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
    assert(map[3][2].entities[2].hp == 8 && map[3][2].entities[2].max_hp == 8 &&
           map[3][2].entities[2].attack == 3 && map[3][2].entities[2].ranged_attack == 2 &&
           map[3][2].entities[2].ranged_range == 7 && map[3][2].entities[2].ranged_cooldown == 0 &&
           map[3][2].entities[2].defense == 0);

    // Move para o item em (6, 5): item desativa e player avanca
    movePlayer((Position){6, 5});
    assert(map[3][2].entities[0].active == false);
    assert(player->pos.x == 6 && player->pos.y == 5);

    // Teste 5: Combate corpo a corpo contra Goblin Melee (4/3/0) em (7, 5)
    // 1o golpe: dano 4, HP 4 -> 0, inimigo desativado e player nao avanca
    movePlayer((Position){7, 5});
    assert(map[3][2].entities[1].hp == 0);
    assert(map[3][2].entities[1].active == false);
    assert(player->pos.x == 6 && player->pos.y == 5);

    // 2o movimento: espaco livre, player avanca para (7, 5)
    movePlayer((Position){7, 5});
    assert(player->pos.x == 7 && player->pos.y == 5);

    // Combate em 2 golpes contra Goblin Ranged (8/3/0)
    attackEntity(player, &map[3][2].entities[2]);
    assert(map[3][2].entities[2].hp == 4);
    assert(map[3][2].entities[2].active == true);
    attackEntity(player, &map[3][2].entities[2]);
    assert(map[3][2].entities[2].hp == 0);
    assert(map[3][2].entities[2].active == false);

    // Teste 6: Dano minimo 1 quando defense >= attack (attackEntity e dealDamage)
    initEntity(player, ENTITY_PLAYER, 5, 5);
    Entity tank;
    initEntity(&tank, ENTITY_BOSS, 10, 5);
    assert(tank.hp == 20 && tank.max_hp == 20 && tank.attack == 5 && tank.ranged_attack == 3 &&
           tank.ranged_range == 7 && tank.ranged_cooldown == 0 && tank.defense == 1);
    tank.defense = 10;
    attackEntity(player, &tank);
    assert(tank.hp == 19 && tank.active == true);
    dealDamage(player->ranged_attack, &tank);
    assert(tank.hp == 18 && tank.active == true);

    // Teste 7: Arco do player (sem cooldown, consome turno, bloqueio por parede e alcance 10)
    clearRoomInterior(curr_room);
    freeRoomEntities(&map[3][2]);
    map[3][2].entities     = malloc(sizeof(Entity) * 1);
    map[3][2].entity_count = 1;
    assert(map[3][2].entities != NULL);

    initEntity(player, ENTITY_PLAYER, 5, 5);
    initEntity(&map[3][2].entities[0], ENTITY_ENEMY_MELEE, 13, 5);

    // 1o disparo cardinal dentro do alcance (dist = 8 <= 10): acerta e passa turno (inimigo aproxima)
    handleInput(KEY_RIGHT);
    assert(map[3][2].entities[0].hp == 2);
    assert(map[3][2].entities[0].pos.x == 12 && map[3][2].entities[0].pos.y == 5);
    assert(player->ranged_cooldown == 0);

    // 2o disparo consecutivo sem cooldown: mata o inimigo
    handleInput(KEY_RIGHT);
    assert(map[3][2].entities[0].hp == 0);
    assert(map[3][2].entities[0].active == false);

    // Parede '#' bloqueia disparo
    initEntity(&map[3][2].entities[0], ENTITY_ENEMY_MELEE, 10, 5);
    curr_room[5][7].ch       = '#';
    curr_room[5][7].walkable = false;
    assert(fireRangedAttack(player, 1, 0) == false);
    assert(map[3][2].entities[0].hp == 4);

    // Inimigo fora do alcance (dist = 11 > 10) nao e atingido
    curr_room[5][7].ch        = '.';
    curr_room[5][7].walkable  = true;
    map[3][2].entities[0].pos = (Position){16, 5};
    assert(fireRangedAttack(player, 1, 0) == false);
    assert(map[3][2].entities[0].hp == 4);

    // Teste 8: IA do Goblin Arqueiro (apenas linha/coluna cardeal) + cooldown + parede
    clearRoomInterior(curr_room);
    freeRoomEntities(&map[3][2]);
    map[3][2].entities     = malloc(sizeof(Entity) * 1);
    map[3][2].entity_count = 1;
    assert(map[3][2].entities != NULL);

    initEntity(player, ENTITY_PLAYER, 10, 6);
    initEntity(&map[3][2].entities[0], ENTITY_ENEMY_RANGED, 15, 6);

    // Linha reta limpa dentro do alcance (dist = 5 <= 7)
    passTime();
    assert(player->hp == 8);
    assert(map[3][2].entities[0].pos.x == 15 && map[3][2].entities[0].pos.y == 6);
    assert(map[3][2].entities[0].ranged_cooldown == 1);

    // Proximo turno: em cooldown, nao dispara e reseta cooldown para 0
    passTime();
    assert(player->hp == 8);
    assert(map[3][2].entities[0].pos.x == 15 && map[3][2].entities[0].pos.y == 6);
    assert(map[3][2].entities[0].ranged_cooldown == 0);

    // Diagonal limpa: Arqueiro NAO deve disparar na diagonal (apenas 4 cardeais)
    player->pos = (Position){12, 3};
    passTime();
    assert(player->hp == 8);
    assert(map[3][2].entities[0].ranged_cooldown == 0);

    // Parede '#' na mesma linha bloqueia tiro cardeal
    player->pos               = (Position){10, 6};
    curr_room[6][13].ch       = '#';
    curr_room[6][13].walkable = false;
    passTime();
    assert(player->hp == 8);
    assert(map[3][2].entities[0].ranged_cooldown == 0);

    // Teste 9: IA do Boss em 2 fases (Fase 1 melee/chase, Fase 2 ranged/evade)
    clearRoomInterior(curr_room);
    freeRoomEntities(&map[3][2]);
    map[3][2].entities     = malloc(sizeof(Entity) * 1);
    map[3][2].entity_count = 1;
    assert(map[3][2].entities != NULL);

    initEntity(player, ENTITY_PLAYER, 10, 5);
    initEntity(&map[3][2].entities[0], ENTITY_BOSS, 14, 5);

    // Fase 1 (hp = 20 > 10): nao atira a distancia, persegue o player
    passTime();
    assert(player->hp == 10);
    int dist_p1 = abs(map[3][2].entities[0].pos.x - player->pos.x) +
                  abs(map[3][2].entities[0].pos.y - player->pos.y);
    assert(dist_p1 < 4);

    // Fase 1 adjacente: ataque corpo a corpo (dano 5)
    map[3][2].entities[0].pos = (Position){11, 5};
    passTime();
    assert(player->hp == 5);

    // Fase 2 (hp = 10 <= 10): disparo a distancia (dano 3) e cooldown = 1
    player->hp                            = 10;
    map[3][2].entities[0].pos             = (Position){13, 5};
    map[3][2].entities[0].hp              = 10;
    map[3][2].entities[0].ranged_cooldown = 0;
    passTime();
    assert(player->hp == 7);
    assert(map[3][2].entities[0].ranged_cooldown == 1);

    // Proximo turno na Fase 2 (em cooldown): nao dispara, cooldown -> 0 e afasta-se (evadePlayer)
    int dx_before   = abs(map[3][2].entities[0].pos.x - player->pos.x);
    int dy_before   = abs(map[3][2].entities[0].pos.y - player->pos.y);
    int dist_before = (dx_before > dy_before) ? dx_before : dy_before;
    passTime();
    assert(player->hp == 7);
    assert(map[3][2].entities[0].ranged_cooldown == 0);
    int dx_after   = abs(map[3][2].entities[0].pos.x - player->pos.x);
    int dy_after   = abs(map[3][2].entities[0].pos.y - player->pos.y);
    int dist_after = (dx_after > dy_after) ? dx_after : dy_after;
    assert(dist_after > dist_before);

    // Teste 10: Game Over e trava de acoes apos morte do player
    clearRoomInterior(curr_room);
    freeRoomEntities(&map[3][2]);
    map[3][2].entities     = malloc(sizeof(Entity) * 2);
    map[3][2].entity_count = 2;
    assert(map[3][2].entities != NULL);

    initEntity(player, ENTITY_PLAYER, 10, 5);
    player->hp = 1;
    initEntity(&map[3][2].entities[0], ENTITY_ENEMY_MELEE, 11, 5);
    initEntity(&map[3][2].entities[1], ENTITY_ENEMY_MELEE, 15, 5);

    passTime();
    assert(player->hp == 0 && player->active == false);

    Position dead_player_pos = player->pos;
    Position enemy0_pos      = map[3][2].entities[0].pos;
    Position enemy1_pos      = map[3][2].entities[1].pos;

    handleInput('w');
    handleInput(KEY_RIGHT);
    movePlayer((Position){10, 4});
    assert(fireRangedAttack(player, 1, 0) == false);
    passTime();

    assert(player->pos.x == dead_player_pos.x && player->pos.y == dead_player_pos.y);
    assert(player->hp == 0 && player->active == false);
    assert(map[3][2].entities[0].pos.x == enemy0_pos.x &&
           map[3][2].entities[0].pos.y == enemy0_pos.y);
    assert(map[3][2].entities[1].pos.x == enemy1_pos.x &&
           map[3][2].entities[1].pos.y == enemy1_pos.y);

    // Teste 11: Prioridade de primeiro ataque do player ao entrar na linha de ataque
    clearRoomInterior(curr_room);
    freeRoomEntities(&map[3][2]);
    map[3][2].entities     = malloc(sizeof(Entity) * 2);
    map[3][2].entity_count = 2;
    assert(map[3][2].entities != NULL);

    // 11a: Melee na diagonal -> player move para adjacente cardeal e NAO toma dano nesse turno
    initEntity(player, ENTITY_PLAYER, 5, 5);
    initEntity(&map[3][2].entities[0], ENTITY_ENEMY_MELEE, 6, 6);
    initEntity(&map[3][2].entities[1], ENTITY_ENEMY_RANGED, 20, 9);
    movePlayer((Position){6, 5});
    assert(player->pos.x == 6 && player->pos.y == 5);
    assert(player->hp == 10); // prioridade do player: goblin nao golpeou ao entrar

    // Proximo turno: player ataca melee primeiro e mata o goblin sem tomar dano
    movePlayer((Position){6, 6});
    assert(map[3][2].entities[0].active == false);
    assert(player->hp == 10);

    // 11b: Arqueiro fora da linha -> player entra na linha exata e NAO leva tiro nesse turno
    initEntity(player, ENTITY_PLAYER, 15, 8);
    map[3][2].entities[1].ranged_cooldown = 0;
    movePlayer((Position){15, 9});
    assert(player->pos.x == 15 && player->pos.y == 9);
    assert(player->hp == 10); // prioridade do player: arqueiro so mira

    // Proximo turno: player atira primeiro no arqueiro
    handleInput(KEY_RIGHT);
    assert(map[3][2].entities[1].hp == 6);

    // Teste 12: Coleta de item com max_hp > 0 soma ao max_hp do jogador (sem sobrescrever)
    clearRoomInterior(curr_room);
    freeRoomEntities(&map[3][2]);
    map[3][2].entities     = malloc(sizeof(Entity) * 2);
    map[3][2].entity_count = 2;
    assert(map[3][2].entities != NULL);

    initEntity(player, ENTITY_PLAYER, 5, 5);
    player->hp = 6;
    initEntity(&map[3][2].entities[0], ENTITY_ITEM, 6, 5);
    map[3][2].entities[0].hp      = 0;
    map[3][2].entities[0].max_hp  = 5;
    map[3][2].entities[0].attack  = 0;
    map[3][2].entities[0].defense = 0;

    initEntity(&map[3][2].entities[1], ENTITY_ITEM, 7, 5);
    map[3][2].entities[1].hp      = 0;
    map[3][2].entities[1].max_hp  = 10;
    map[3][2].entities[1].attack  = 0;
    map[3][2].entities[1].defense = 0;

    movePlayer((Position){6, 5});
    assert(map[3][2].entities[0].active == false);
    assert(player->max_hp == 15 && player->hp == 15);

    movePlayer((Position){7, 5});
    assert(map[3][2].entities[1].active == false);
    assert(player->max_hp == 25 && player->hp == 25);

    free(player);
    player = NULL;
    eraseLogQueue(logs);
    logs = NULL;
    freeMap();
    assert(map == NULL);

    // Teste 13: Save/Load round-trip (persistencia completa de mapa, entidades e player)
    logs = initializeLogQueue();
    assert(logs != NULL);
    map   = createMap();
    map_x = 3;
    map_y = 3;
    setupFloor(map_x, map_y, 8);
    curr_room = map[map_y][map_x].tiles;
    player    = createPlayer((Position){8, 4});
    assert(player != NULL);
    player->hp              = 7;
    player->max_hp          = 15;
    player->attack          = 6;
    player->ranged_attack   = 4;
    player->ranged_range    = 9;
    player->ranged_cooldown = 1;
    player->defense         = 2;

    int expected_ids[7][7];
    int expected_counts[7][7];
    bool expected_visited[7][7];
    for (int y = 0; y < MAP_HEIGHT; ++y)
    {
        for (int x = 0; x < MAP_WIDTH; ++x)
        {
            expected_ids[y][x]     = map[y][x].id;
            expected_counts[y][x]  = map[y][x].entity_count;
            expected_visited[y][x] = map[y][x].visited;
        }
    }

    int target_ey = -1, target_ex = -1;
    for (int y = 0; y < MAP_HEIGHT && target_ey == -1; ++y)
    {
        for (int x = 0; x < MAP_WIDTH && target_ey == -1; ++x)
        {
            if (map[y][x].id != 0 && map[y][x].entity_count > 0)
            {
                target_ey = y;
                target_ex = x;
            }
        }
    }
    assert(target_ey != -1 && target_ex != -1);
    map[target_ey][target_ex].entities[0].hp              = 2;
    map[target_ey][target_ex].entities[0].ranged_cooldown = 1;
    Entity expected_entity                                = map[target_ey][target_ex].entities[0];

    save(player);
    assert(saveExists() == true);

    cleanupRun();
    assert(map == NULL && player == NULL);

    logs = initializeLogQueue();
    map  = createMap();
    assert(continueGame() == true);
    assert(player != NULL);
    assert(map_x == 3 && map_y == 3);
    assert(player->hp == 7 && player->max_hp == 15);
    assert(player->attack == 6 && player->defense == 2);
    assert(player->ranged_attack == 4 && player->ranged_range == 9 && player->ranged_cooldown == 1);
    assert(player->pos.x == 8 && player->pos.y == 4);

    for (int y = 0; y < MAP_HEIGHT; ++y)
    {
        for (int x = 0; x < MAP_WIDTH; ++x)
        {
            assert(map[y][x].id == expected_ids[y][x]);
            assert(map[y][x].entity_count == expected_counts[y][x]);
            assert(map[y][x].visited == expected_visited[y][x]);
        }
    }

    Entity* loaded_entity = &map[target_ey][target_ex].entities[0];
    assert(loaded_entity->active == expected_entity.active);
    assert(loaded_entity->ch == expected_entity.ch);
    assert(loaded_entity->type == expected_entity.type);
    assert(loaded_entity->hp == expected_entity.hp);
    assert(loaded_entity->max_hp == expected_entity.max_hp);
    assert(loaded_entity->attack == expected_entity.attack);
    assert(loaded_entity->ranged_attack == expected_entity.ranged_attack);
    assert(loaded_entity->ranged_range == expected_entity.ranged_range);
    assert(loaded_entity->ranged_cooldown == expected_entity.ranged_cooldown);
    assert(loaded_entity->defense == expected_entity.defense);
    assert(loaded_entity->pos.x == expected_entity.pos.x);
    assert(loaded_entity->pos.y == expected_entity.pos.y);

    cleanupRun();
    remove(SAVE_PATH);
    assert(map == NULL && player == NULL);

    // Teste 14: Morte do Boss no ultimo andar gera escada e bloqueia vitoria com inimigos vivos
    logs       = initializeLogQueue();
    map        = createMap();
    curr_floor = LAST_FLOOR;
    game_won   = false;
    map_x      = 3;
    map_y      = 3;
    assert(setRoom(map_x, map_y, 0) == 0);
    curr_room = map[map_y][map_x].tiles;
    clearRoomInterior(curr_room);
    freeRoomEntities(&map[map_y][map_x]);
    map[map_y][map_x].entities     = malloc(sizeof(Entity) * 2);
    map[map_y][map_x].entity_count = 2;
    assert(map[map_y][map_x].entities != NULL);

    player = createPlayer((Position){10, 5});
    initEntity(&map[map_y][map_x].entities[0], ENTITY_BOSS, 11, 5);
    initEntity(&map[map_y][map_x].entities[1], ENTITY_ENEMY_MELEE, 20, 5);

    dealDamage(100, &map[map_y][map_x].entities[0]);
    assert(map[map_y][map_x].entities[0].active == false);
    assert(game_won == false);
    Entity* spawned_door = getEntityAt(11, 5);
    assert(spawned_door != NULL && spawned_door->type == ENTITY_NEXT_FLOOR_DOOR);

    movePlayer((Position){11, 5});
    assert(game_won == false);
    assert(logs->size > 0);

    cleanupRun();

    // Teste 15: Trancamento de portas por inimigos vivos e desbloqueio ao zerar
    acs_map[(unsigned char)'+'] = '>';
    acs_map[(unsigned char)','] = '<';
    acs_map[(unsigned char)'-'] = '^';
    acs_map[(unsigned char)'.'] = 'v';
    acs_map[(unsigned char)'x'] = '|';
    acs_map[(unsigned char)'q'] = '-';
    logs  = initializeLogQueue();
    map   = createMap();
    map_x = 3;
    map_y = 3;
    assert(setRoom(3, 3, 0) == 0);
    assert(setRoom(4, 3, 4) == 0);

    freeRoomEntities(&map[3][3]);
    map[3][3].entities     = malloc(sizeof(Entity) * 2);
    map[3][3].entity_count = 2;
    assert(map[3][3].entities != NULL);
    initEntity(&map[3][3].entities[0], ENTITY_ITEM, 10, 5);
    initEntity(&map[3][3].entities[1], ENTITY_CONSUMABLE, 11, 5);

    freeRoomEntities(&map[3][4]);
    map[3][4].entities     = malloc(sizeof(Entity) * 2);
    map[3][4].entity_count = 2;
    assert(map[3][4].entities != NULL);
    initEntity(&map[3][4].entities[0], ENTITY_ENEMY_MELEE, 20, 5);
    map[3][4].entities[0].healingItem = 0;
    initEntity(&map[3][4].entities[1], ENTITY_ENEMY_RANGED, 22, 5);
    map[3][4].entities[1].healingItem = 5;

    curr_room = map[3][3].tiles;
    clearRoomInterior(curr_room);
    clearRoomInterior(map[3][4].tiles);
    player = createPlayer((Position){ROOM_WIDTH - 2, 5});
    roomDoors();
    assert(roomHasAliveEnemies() == false);
    assert(curr_room[5][ROOM_WIDTH - 1].ch == ACS_RARROW);

    // Entra na sala vizinha (4, 3) com 2 inimigos vivos: porta esquerda tranca com '#'
    movePlayer((Position){ROOM_WIDTH - 1, 5});
    assert(map_x == 4 && map_y == 3);
    assert(roomHasAliveEnemies() == true);
    assert(curr_room[5][0].ch == '#');
    assert(curr_room[5][ROOM_WIDTH - 1].ch == ACS_VLINE);

    // Tentar sair pela porta trancada nao muda de sala
    movePlayer((Position){0, 5});
    assert(map_x == 4 && map_y == 3);

    // Mata 1o inimigo: porta continua trancada
    dealDamage(100, &map[3][4].entities[0]);
    assert(roomHasAliveEnemies() == true);
    assert(curr_room[5][0].ch == '#');

    // Mata 2o inimigo (mesmo dropando consumivel): porta destranca para ACS_LARROW
    dealDamage(100, &map[3][4].entities[1]);
    assert(roomHasAliveEnemies() == false);
    assert(curr_room[5][0].ch == ACS_LARROW);

    // Agora o player consegue sair de volta para (3, 3)
    movePlayer((Position){0, 5});
    assert(map_x == 3 && map_y == 3);

    cleanupRun();

    printf("Todos os 15 testes do mapa procedural, entidades, combate, IA, save/load, vitoria e tranca de portas passaram com sucesso!\n");
    return 0;
}
