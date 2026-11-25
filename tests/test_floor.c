#include "rogue.h"
#include <assert.h>
#include <stdio.h>

// Informações globais que os testes precisam para conversar com o resto do jogo
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

/*
 * Limpa o interior da sala deixando só chão livre para facilitar os testes.
 * Onde é chamada: main (tests/test_floor.c).
 */
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

    // O centro (3, 3) precisa ser a sala inicial (tipo 0), já visitada e visível no mapa
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

    // A quantidade total de salas criadas precisa bater com a pedida
    assert(count_rooms == target_rooms);

    // As salas vizinhas ligadas a (3, 3) precisam aparecer como descobertas, mas ainda não visitadas
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

    // Precisa existir pelo menos uma sala vizinha ligada quando o andar tem mais de 1 sala
    assert(discovered_neighbor_y != -1 && discovered_neighbor_x != -1);

    // Simula o jogador entrando na sala vizinha já descoberta
    map_y = discovered_neighbor_y;
    map_x = discovered_neighbor_x;
    updateFogOfWar();
    assert(map[map_y][map_x].visited == true);
    assert(map[map_y][map_x].discovered == true);

    // Limpa o mapa da memória
    freeMap();
    assert(map == NULL);

    // Teste 2: Confere se existem exatamente os 15 modelos de sala nas pastas
    int total_templates = countTxtFiles("data/rooms/start") + countTxtFiles("data/rooms/boss") +
                          countTxtFiles("data/rooms/items") + countTxtFiles("data/rooms/normal");
    assert(total_templates == 15);

    // Teste 3: Confere se as letras E/A/B/I viram personagens e itens e deixam o chão '.' livre
    map   = createMap();
    map_x = 3;
    map_y = 3;
    assert(setRoom(3, 3, 1) == 0); // Sala do chefão sempre tem 'B'
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
            assert(e->hp == 35 && e->max_hp == 35 && e->attack == 7 && e->ranged_attack == 5 &&
                   e->ranged_range == 8 && e->ranged_cooldown == 0 && e->defense == 1);
        }
    }
    assert(found_boss == true);

    // Teste 4: Criação do jogador e coleta de item (o item some do mapa quando é pego)
    Position start_pos = {5, 5};
    player             = createPlayer(start_pos);
    assert(player != NULL);
    assert(player->hp == 20 && player->max_hp == 20 && player->attack == 4 &&
           player->ranged_attack == 2 && player->ranged_range == 8 &&
           player->ranged_cooldown == 0 && player->defense == 0 && player->active == true);

    assert(setRoom(2, 3, 4) == 0); // Usa uma sala comum limpa para controlar os testes
    map_x     = 2;
    map_y     = 3;
    curr_room = map[3][2].tiles;
    clearRoomInterior(curr_room);

    freeRoomEntities(&map[3][2]);
    map[3][2].entities     = malloc(sizeof(Entity) * 3);
    map[3][2].entity_count = 3;
    assert(map[3][2].entities != NULL);

    initEntity(&map[3][2].entities[0], ENTITY_ITEM, 6, 5);
    map[3][2].entities[0].hp            = 0;
    map[3][2].entities[0].max_hp        = 0;
    map[3][2].entities[0].attack        = 0;
    map[3][2].entities[0].ranged_attack = 0;
    map[3][2].entities[0].defense       = 0;
    initEntity(&map[3][2].entities[1], ENTITY_ENEMY_MELEE, 7, 5);
    initEntity(&map[3][2].entities[2], ENTITY_ENEMY_RANGED, 5, 6);
    assert(map[3][2].entities[2].hp == 6 && map[3][2].entities[2].max_hp == 6 &&
           map[3][2].entities[2].attack == 3 && map[3][2].entities[2].ranged_attack == 2 &&
           map[3][2].entities[2].ranged_range == 5 && map[3][2].entities[2].ranged_cooldown == 0 &&
           map[3][2].entities[2].defense == 0);

    // Anda até o item em (6, 5): o item é recolhido e o jogador avança para lá
    movePlayer((Position){6, 5});
    assert(map[3][2].entities[0].active == false);
    assert(player->pos.x == 6 && player->pos.y == 5);

    // Teste 5: Luta de perto contra o Goblin (6 de vida, 4 de ataque, 0 de defesa) em (7, 5)
    // 2 golpes com attack 4: 6 -> 2 -> 0
    movePlayer((Position){7, 5});
    assert(map[3][2].entities[1].hp == 2);
    assert(map[3][2].entities[1].active == true);
    movePlayer((Position){7, 5});
    assert(map[3][2].entities[1].hp == 0);
    assert(map[3][2].entities[1].active == false);
    assert(player->pos.x == 6 && player->pos.y == 5);

    // 3º passo: o espaço agora está livre, então o jogador entra em (7, 5)
    movePlayer((Position){7, 5});
    assert(player->pos.x == 7 && player->pos.y == 5);

    // Luta em 2 golpes contra o Goblin Arqueiro (6 de vida, 3 de ataque, 0 de defesa)
    attackEntity(player, &map[3][2].entities[2]);
    assert(map[3][2].entities[2].hp == 2);
    assert(map[3][2].entities[2].active == true);
    attackEntity(player, &map[3][2].entities[2]);
    assert(map[3][2].entities[2].hp == 0);
    assert(map[3][2].entities[2].active == false);

    // Teste 6: Garante que todo golpe tira pelo menos 1 de vida mesmo quando a defesa é maior que o ataque
    initEntity(player, ENTITY_PLAYER, 5, 5);
    Entity tank;
    initEntity(&tank, ENTITY_BOSS, 10, 5);
    assert(tank.hp == 35 && tank.max_hp == 35 && tank.attack == 7 && tank.ranged_attack == 5 &&
           tank.ranged_range == 8 && tank.ranged_cooldown == 0 && tank.defense == 1);
    tank.defense = 10;
    attackEntity(player, &tank);
    assert(tank.hp == 34 && tank.active == true);
    dealDamage(player->ranged_attack, &tank);
    assert(tank.hp == 33 && tank.active == true);

    // Teste 7: Arco do jogador (atira sem tempo de espera, passa o turno, para em paredes e tem alcance 8)
    clearRoomInterior(curr_room);
    freeRoomEntities(&map[3][2]);
    map[3][2].entities     = malloc(sizeof(Entity) * 1);
    map[3][2].entity_count = 1;
    assert(map[3][2].entities != NULL);

    initEntity(player, ENTITY_PLAYER, 5, 5);
    initEntity(&map[3][2].entities[0], ENTITY_ENEMY_MELEE, 13, 5);
    map[3][2].entities[0].hp = 4;

    // 1º tiro em linha reta dentro do alcance (distância 8 <= 8): acerta e passa o turno (o inimigo se aproxima)
    handleInput(KEY_RIGHT);
    assert(map[3][2].entities[0].hp == 2);
    assert(map[3][2].entities[0].pos.x == 12 && map[3][2].entities[0].pos.y == 5);
    assert(player->ranged_cooldown == 0);

    // 2º tiro seguido sem precisar esperar: derrota o inimigo
    handleInput(KEY_RIGHT);
    assert(map[3][2].entities[0].hp == 0);
    assert(map[3][2].entities[0].active == false);

    // Parede '#' no meio do caminho bloqueia a flecha
    initEntity(&map[3][2].entities[0], ENTITY_ENEMY_MELEE, 10, 5);
    curr_room[5][7].ch       = '#';
    curr_room[5][7].walkable = false;
    assert(fireRangedAttack(player, 1, 0) == false);
    assert(map[3][2].entities[0].hp == 6);

    // Inimigo longe demais (distância 9 > 8) não é atingido pela flecha
    curr_room[5][7].ch        = '.';
    curr_room[5][7].walkable  = true;
    map[3][2].entities[0].pos = (Position){14, 5};
    assert(fireRangedAttack(player, 1, 0) == false);
    assert(map[3][2].entities[0].hp == 6);

    // Flecha atravessa consumível ('C') e porta de saída ('/') sem destruí-los e acerta inimigo atrás
    freeRoomEntities(&map[3][2]);
    map[3][2].entities     = malloc(sizeof(Entity) * 3);
    map[3][2].entity_count = 3;
    assert(map[3][2].entities != NULL);
    initEntity(&map[3][2].entities[0], ENTITY_CONSUMABLE, 7, 5);
    map[3][2].entities[0].hp = 4;
    initEntity(&map[3][2].entities[1], ENTITY_NEXT_FLOOR_DOOR, 8, 5);
    map[3][2].entities[1].ch = '/';
    initEntity(&map[3][2].entities[2], ENTITY_ENEMY_MELEE, 10, 5);
    assert(fireRangedAttack(player, 1, 0) == true);
    assert(map[3][2].entities[0].active == true && map[3][2].entities[0].hp == 4);
    assert(map[3][2].entities[1].active == true);
    assert(map[3][2].entities[2].hp == 6 - player->ranged_attack);

    // Teste 8: Comportamento do Arqueiro (enxerga em 360°, mira por 2 turnos, descansa 3 turnos parado, reposiciona via BFS 8 direções quando sem visão)
    clearRoomInterior(curr_room);
    freeRoomEntities(&map[3][2]);
    map[3][2].entities     = malloc(sizeof(Entity) * 1);
    map[3][2].entity_count = 1;
    assert(map[3][2].entities != NULL);

    initEntity(player, ENTITY_PLAYER, 11, 3);
    player->hp = 10;
    initEntity(&map[3][2].entities[0], ENTITY_ENEMY_RANGED, 15, 6);

    // Turno 1 em ângulo inclinado dentro do alcance (distância 5 <= 5): começa a mirar (1º turno de mira), sem causar dano
    passTime();
    assert(player->hp == 10);
    assert(map[3][2].entities[0].aim_turns == 1);
    assert(map[3][2].entities[0].ranged_cooldown == 0);

    // Jogador anda sem sair da visão -> Turno 2: continua mirando (2º turno de mira), ainda sem dano
    player->pos = (Position){12, 3};
    passTime();
    assert(player->hp == 10);
    assert(map[3][2].entities[0].aim_turns == 2);
    assert(map[3][2].entities[0].ranged_cooldown == 0);

    // Turno 3: atira na posição atual do jogador, tira 2 de vida, zera a mira e entra em descanso de 3 turnos
    passTime();
    assert(player->hp == 8);
    assert(map[3][2].entities[0].pos.x == 15 && map[3][2].entities[0].pos.y == 6);
    assert(map[3][2].entities[0].aim_turns == 0);
    assert(map[3][2].entities[0].ranged_cooldown == 3);

    // Nos próximos 3 turnos descansando (mesmo com parede na visão): fica parado, não mira nem atira
    curr_room[5][14].ch       = '#';
    curr_room[5][14].walkable = false;
    for (int cd = 2; cd >= 0; --cd)
    {
        passTime();
        assert(player->hp == 8);
        assert(map[3][2].entities[0].pos.x == 15 && map[3][2].entities[0].pos.y == 6);
        assert(map[3][2].entities[0].aim_turns == 0);
        assert(map[3][2].entities[0].ranged_cooldown == cd);
    }

    // Remove a parede e confirma que volta a mirar depois que o descanso zera
    curr_room[5][14].ch       = '.';
    curr_room[5][14].walkable = true;
    passTime();
    assert(player->hp == 8);
    assert(map[3][2].entities[0].aim_turns == 1);

    // Parede '#' na linha de visão cancela a mira e faz o arqueiro andar 1 passo via BFS para casa com visão 360° mais distante
    curr_room[5][14].ch       = '#';
    curr_room[5][14].walkable = false;
    passTime();
    assert(player->hp == 8);
    assert(map[3][2].entities[0].aim_turns == 0);
    assert(map[3][2].entities[0].pos.x == 16 && map[3][2].entities[0].pos.y == 5);

    // Sem parede, mas fora do alcance (distância > 5), zera mira e anda 1 passo para entrar no alcance
    curr_room[5][14].ch       = '.';
    curr_room[5][14].walkable = true;
    player->pos               = (Position){7, 6};
    map[3][2].entities[0].pos = (Position){15, 6};
    passTime();
    assert(player->hp == 8);
    assert(map[3][2].entities[0].aim_turns == 0);
    assert(map[3][2].entities[0].pos.x == 14);

    // Totalmente cercado por paredes: fica parado com segurança
    map[3][2].entities[0].pos = (Position){15, 6};
    for (int dy = -1; dy <= 1; ++dy)
        for (int dx = -1; dx <= 1; ++dx)
            if (dx != 0 || dy != 0)
            {
                curr_room[6 + dy][15 + dx].ch       = '#';
                curr_room[6 + dy][15 + dx].walkable = false;
            }
    passTime();
    assert(map[3][2].entities[0].pos.x == 15 && map[3][2].entities[0].pos.y == 6);
    assert(map[3][2].entities[0].aim_turns == 0);

    // Teste 9: Comportamento do Chefão (disparo alinhado em 8 direções, descanso e ataque de perto)
    clearRoomInterior(curr_room);
    freeRoomEntities(&map[3][2]);
    map[3][2].entities     = malloc(sizeof(Entity) * 1);
    map[3][2].entity_count = 1;
    assert(map[3][2].entities != NULL);

    initEntity(player, ENTITY_PLAYER, 10, 5);
    initEntity(&map[3][2].entities[0], ENTITY_BOSS, 14, 5);

    // Alinhado com descanso 0: atira de longe (tira 5 de vida) e ativa descanso 2
    passTime();
    assert(player->hp == 15);
    assert(map[3][2].entities[0].ranged_cooldown == 2);

    // Em descanso: aproxima do jogador sem atirar
    passTime();
    assert(player->hp == 15);
    assert(map[3][2].entities[0].ranged_cooldown == 1);
    assert(abs(map[3][2].entities[0].pos.x - player->pos.x) < 4);

    // Colado no jogador: dá golpe de perto (tira 7 de vida)
    map[3][2].entities[0].pos = (Position){11, 5};
    passTime();
    assert(player->hp == 8);

    // Teste 10: Fim de jogo e bloqueio de qualquer ação depois que o jogador morre
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

    // Teste 11: Chance do jogador bater primeiro ao entrar no alcance de um inimigo
    clearRoomInterior(curr_room);
    freeRoomEntities(&map[3][2]);
    map[3][2].entities     = malloc(sizeof(Entity) * 2);
    map[3][2].entity_count = 2;
    assert(map[3][2].entities != NULL);

    // 11a: Goblin estava longe -> jogador entra na diagonal e NÃO toma dano nesse turno
    initEntity(player, ENTITY_PLAYER, 4, 5);
    player->hp = 10;
    initEntity(&map[3][2].entities[0], ENTITY_ENEMY_MELEE, 6, 6);
    map[3][2].entities[0].hp = 4;
    initEntity(&map[3][2].entities[1], ENTITY_ENEMY_RANGED, 20, 9);
    movePlayer((Position){5, 5});
    assert(player->pos.x == 5 && player->pos.y == 5);
    assert(player->hp == 10); // O jogador tem prioridade: o goblin não bateu quando ele acabou de se aproximar

    // No turno seguinte: jogador bate primeiro e derrota o goblin sem levar dano
    movePlayer((Position){6, 6});
    assert(map[3][2].entities[0].active == false);
    assert(player->hp == 10);

    // 11b: Arqueiro estava sem visão -> jogador entra na frente dele, e o arqueiro só começa a mirar
    initEntity(player, ENTITY_PLAYER, 12, 9);
    player->hp                            = 10;
    map[3][2].entities[1].pos             = (Position){18, 9};
    map[3][2].entities[1].ranged_cooldown = 0;
    map[3][2].entities[1].aim_turns       = 0;
    movePlayer((Position){13, 9});
    assert(player->pos.x == 13 && player->pos.y == 9);
    assert(player->hp == 10);
    assert(map[3][2].entities[1].aim_turns == 1);

    // No turno seguinte: jogador atira primeiro no arqueiro; o arqueiro só avança a mira sem atirar
    handleInput(KEY_RIGHT);
    assert(map[3][2].entities[1].hp == 4);
    assert(player->hp == 10);
    assert(map[3][2].entities[1].aim_turns == 2);

    // Teste 12: Pegar item de vida máxima soma ao max_hp curando apenas o bônus, e item de arco soma ranged_attack
    clearRoomInterior(curr_room);
    freeRoomEntities(&map[3][2]);
    map[3][2].entities     = malloc(sizeof(Entity) * 2);
    map[3][2].entity_count = 2;
    assert(map[3][2].entities != NULL);

    initEntity(player, ENTITY_PLAYER, 5, 5);
    player->hp = 6;
    initEntity(&map[3][2].entities[0], ENTITY_ITEM, 6, 5);
    map[3][2].entities[0].hp            = 0;
    map[3][2].entities[0].max_hp        = 5;
    map[3][2].entities[0].attack        = 0;
    map[3][2].entities[0].ranged_attack = 0;
    map[3][2].entities[0].defense       = 0;

    initEntity(&map[3][2].entities[1], ENTITY_ITEM, 7, 5);
    map[3][2].entities[1].hp            = 0;
    map[3][2].entities[1].max_hp        = 10;
    map[3][2].entities[1].attack        = 0;
    map[3][2].entities[1].ranged_attack = 2;
    map[3][2].entities[1].defense       = 0;

    movePlayer((Position){6, 5});
    assert(map[3][2].entities[0].active == false);
    assert(player->max_hp == 25 && player->hp == 11);

    movePlayer((Position){7, 5});
    assert(map[3][2].entities[1].active == false);
    assert(player->max_hp == 35 && player->hp == 21);
    assert(player->ranged_attack == 4);

    // Escalonamento de inimigos no andar 2 (+35% HP/ATK e +1 DEF em melee/boss)
    curr_floor = 2;
    Entity f2_melee, f2_ranged, f2_boss;
    initEntity(&f2_melee, ENTITY_ENEMY_MELEE, 1, 1);
    initEntity(&f2_ranged, ENTITY_ENEMY_RANGED, 2, 1);
    initEntity(&f2_boss, ENTITY_BOSS, 3, 1);
    assert(f2_melee.max_hp == 8 && f2_melee.attack == 5 && f2_melee.defense == 1);
    assert(f2_ranged.max_hp == 8 && f2_ranged.ranged_attack == 3 && f2_ranged.defense == 0);
    assert(f2_boss.max_hp == 47 && f2_boss.attack == 9 && f2_boss.ranged_attack == 7 &&
           f2_boss.defense == 2);
    curr_floor = 1;

    free(player);
    player = NULL;
    eraseLogQueue(logs);
    logs = NULL;
    freeMap();
    assert(map == NULL);

    // Teste 13: Salvar e carregar o jogo mantém tudo igual (mapa, inimigos, itens e jogador)
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

    // Teste 14: Derrotar o Chefão no último andar faz aparecer a saída e bloqueia a vitória se ainda houver inimigos vivos
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

    // Teste 15: Portas trancam quando há inimigos vivos na sala e destrancam quando todos são derrotados
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

    // Entra na sala vizinha (4, 3) com 2 inimigos vivos: a porta da esquerda tranca com '#'
    movePlayer((Position){ROOM_WIDTH - 1, 5});
    assert(map_x == 4 && map_y == 3);
    assert(roomHasAliveEnemies() == true);
    assert(curr_room[5][0].ch == '#');
    assert(curr_room[5][ROOM_WIDTH - 1].ch == ACS_VLINE);

    // Tentar sair pela porta trancada não deixa mudar de sala
    movePlayer((Position){0, 5});
    assert(map_x == 4 && map_y == 3);

    // Derrota o 1º inimigo: a porta continua trancada
    dealDamage(100, &map[3][4].entities[0]);
    assert(roomHasAliveEnemies() == true);
    assert(curr_room[5][0].ch == '#');

    // Derrota o 2º inimigo (mesmo ele deixando um item de cura): a porta destranca e volta a ser seta
    dealDamage(100, &map[3][4].entities[1]);
    assert(roomHasAliveEnemies() == false);
    assert(curr_room[5][0].ch == ACS_LARROW);

    // Agora o jogador consegue voltar para a sala (3, 3)
    movePlayer((Position){0, 5});
    assert(map_x == 3 && map_y == 3);

    cleanupRun();

    // Teste 16: Movimento e ataque na diagonal do Goblin de perto e do Chefão (o Arqueiro continua parado)
    logs  = initializeLogQueue();
    map   = createMap();
    map_x = 3;
    map_y = 3;
    assert(setRoom(3, 3, 0) == 0);
    curr_room = map[3][3].tiles;
    clearRoomInterior(curr_room);
    freeRoomEntities(&map[3][3]);
    map[3][3].entities     = malloc(sizeof(Entity) * 3);
    map[3][3].entity_count = 3;
    assert(map[3][3].entities != NULL);

    player = createPlayer((Position){5, 5});
    assert(player != NULL);
    player->hp = 20;

    // 16a: Goblin de perto anda na diagonal ((8, 8) -> (7, 7)) e o Arqueiro (10, 8) entra no alcance em (9, 7)
    initEntity(&map[3][3].entities[0], ENTITY_ENEMY_MELEE, 8, 8);
    initEntity(&map[3][3].entities[1], ENTITY_ENEMY_RANGED, 10, 8);
    initEntity(&map[3][3].entities[2], ENTITY_BOSS, 20, 8);
    map[3][3].entities[2].active = false;

    int hp_before = player->hp;
    passTime();
    assert(map[3][3].entities[0].pos.x == 7 && map[3][3].entities[0].pos.y == 7);
    assert(map[3][3].entities[1].pos.x == 9 && map[3][3].entities[1].pos.y == 7);
    assert(map[3][3].entities[1].aim_turns == 0);
    assert(player->hp == hp_before);

    // 16b: Jogador tem a vez primeiro na diagonal contra o Goblin, e o Goblin ataca na diagonal no turno seguinte
    player->pos                 = (Position){4, 5};
    map[3][3].entities[0].pos   = (Position){6, 6};
    hp_before                   = player->hp;
    movePlayer((Position){5, 5});
    assert(player->hp == hp_before);
    passTime();
    assert(player->hp == hp_before - map[3][3].entities[0].attack);

    // 16c: Chefão fora da linha de tiro persegue na diagonal, ataca na diagonal e respeita a vez do jogador
    map[3][3].entities[0].active = false;
    map[3][3].entities[1].active = false;
    map[3][3].entities[2].active = true;
    player->pos                  = (Position){5, 5};
    map[3][3].entities[2].pos    = (Position){8, 7};
    curr_room[7][7].walkable     = false;
    hp_before                    = player->hp;
    passTime();
    assert(map[3][3].entities[2].pos.x == 7 && map[3][3].entities[2].pos.y == 6);
    assert(player->hp == hp_before);
    curr_room[7][7].walkable = true;

    player->pos               = (Position){4, 5};
    map[3][3].entities[2].pos = (Position){6, 6};
    hp_before                 = player->hp;
    movePlayer((Position){5, 5});
    assert(player->hp == hp_before);
    passTime();
    assert(player->hp == hp_before - map[3][3].entities[2].attack);

    cleanupRun();

    printf("Todos os 16 testes do mapa procedural, entidades, combate, IA, save/load, vitoria e tranca de portas passaram com sucesso!\n");
    return 0;
}
