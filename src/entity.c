#include "rogue.h"
#include <curses.h>
#include <stdio.h>
#include <stdlib.h>

static bool     player_moved_turn = false;
static Position prev_player_pos   = {0, 0};

/*
 * Verifica se as posições a e b estão adjacentes nas 4 direções cardeais
 * (distância de Manhattan igual a 1).
 * Chamada por: chasePlayer e actBoss (src/entity.c).
 */
static bool isCardinalAdjacent(Position a, Position b)
{
    return abs(a.x - b.x) + abs(a.y - b.y) == 1;
}

/*
 * Converte um caractere do template de sala ('E', 'A', 'B', 'I') para o
 * respectivo EntityType em out_type; retorna false se não for entidade.
 * Chamada por: spawnRoomEntities (src/entity.c).
 */
static bool charToEntityType(chtype ch, EntityType* out_type)
{
    switch (ch)
    {
    case 'E':
        *out_type = ENTITY_ENEMY_MELEE;
        return true;
    case 'A':
        *out_type = ENTITY_ENEMY_RANGED;
        return true;
    case 'B':
        *out_type = ENTITY_BOSS;
        return true;
    case 'I':
        *out_type = ENTITY_ITEM;
        return true;
    default:
        return false;
    }
}

/*
 * Inicializa a struct Entity e com posição (x, y), tipo, estado ativo e atributos
 * base (HP, ataque corpo a corpo/distância, alcance e defesa); itens chamam chooseItem.
 * Chamada por: spawnRoomEntities e createPlayer (src/entity.c), e tests/test_floor.c.
 */
void initEntity(Entity* e, EntityType type, int x, int y)
{
    if (!e)
        return;

    e->pos.x           = x;
    e->pos.y           = y;
    e->type            = type;
    e->ranged_attack   = 0;
    e->ranged_range    = 0;
    e->ranged_cooldown = 0;
    e->active          = true;
    e->healingItem     = 0;
    int healing        = rand() % 10 + 1;

    switch (type)
    {
    case ENTITY_PLAYER:
        e->ch            = '@';
        e->max_hp        = 99999;
        e->hp            = e->max_hp;
        e->attack        = 99999;
        e->ranged_attack = 2;
        e->ranged_range  = 10;
        e->defense       = 0;
        break;
    case ENTITY_ENEMY_MELEE:
        e->ch      = 'E';
        e->max_hp  = 4;
        e->hp      = e->max_hp;
        e->attack  = 3;
        e->defense = 0;
        if (healing == 1)
            e->healingItem = 5;
        else if (healing == 2)
            e->healingItem = 10;
        break;
    case ENTITY_ENEMY_RANGED:
        e->ch            = 'A';
        e->max_hp        = 8;
        e->hp            = e->max_hp;
        e->attack        = 3;
        e->ranged_attack = 2;
        e->ranged_range  = 7;
        e->defense       = 0;
        if (healing == 1)
            e->healingItem = 5;
        else if (healing == 2)
            e->healingItem = 10;
        break;
    case ENTITY_BOSS:
        e->ch            = 'B';
        e->max_hp        = 20;
        e->hp            = e->max_hp;
        e->attack        = 5;
        e->ranged_attack = 3;
        e->ranged_range  = 7;
        e->defense       = 1;
        break;
    case ENTITY_ITEM:
        chooseItem(e, 'i');
        break;
    case ENTITY_CONSUMABLE:
        chooseItem(e, 'c');
        break;
    case ENTITY_NEXT_FLOOR_DOOR:
        break;
    }
}

/*
 * Limpa os caracteres de entidades do template já carregado em posMap->tiles,
 * mantendo apenas '.' e '#' e tornando as demais células internas caminháveis.
 * Chamada por: continueGame (src/engine.c).
 */
void removeEntityChars(MapCell* posMap)
{
    if (!posMap || !posMap->tiles)
        return;

    for (int i = 1; i < ROOM_HEIGHT - 1; i++)
    {
        for (int j = 1; j < ROOM_WIDTH - 1; j++)
        {
            if (posMap->tiles[i][j].ch == '.' || posMap->tiles[i][j].ch == '#')
                continue;
            posMap->tiles[i][j].ch       = '.';
            posMap->tiles[i][j].walkable = true;
        }
    }
}

/*
 * Sorteia um entre 8 tipos de item e configura os bônus de atributo (hp, max_hp,
 * attack ou defense) da entidade ENTITY_ITEM.
 * Chamada por: initEntity (src/entity.c).
 */
void chooseItem(Entity* e, char type)
{
    e->hp      = 0;
    e->max_hp  = 0;
    e->attack  = 0;
    e->defense = 0;
    if (type == 'i')
    {
        int item = rand() % 5 + 1;
        e->ch    = 'I';
        switch (item)
        {       // mudar depois, talvez um txt
        case 1: // hp
            e->max_hp = 10;
            break;
        case 2:
            e->max_hp = 20;
            break;
        case 3: // ataque
            e->attack = 5;
            break;
        case 4:
            e->attack = 10;
            break;
        case 5: // defesa
            e->defense = 5;
            break;
        case 6:
            e->defense = 10;
            break;
        }
    }
    else if (type == 'c')
    {
        e->ch = 'C';
    }
}

/*
 * Varre o interior de cell->tiles contando marcadores de entidade, aloca o vetor
 * cell->entities, inicializa cada entidade via initEntity e troca o tile para '.'.
 * Chamada por: setRoom (src/room.c).
 */
int spawnRoomEntities(MapCell* cell)
{
    if (!cell || !cell->tiles)
        return 1;

    freeRoomEntities(cell);

    EntityType type;
    int        count = 0;
    for (int y = 1; y < ROOM_HEIGHT - 1; ++y)
    {
        for (int x = 1; x < ROOM_WIDTH - 1; ++x)
        {
            if (charToEntityType(cell->tiles[y][x].ch, &type))
                count++;
        }
    }

    if (count == 0)
        return 0;

    cell->entities = malloc(sizeof(Entity) * count);
    if (!cell->entities)
        return 1;

    int idx = 0;
    for (int y = 1; y < ROOM_HEIGHT - 1; ++y)
    {
        for (int x = 1; x < ROOM_WIDTH - 1; ++x)
        {
            if (charToEntityType(cell->tiles[y][x].ch, &type))
            {
                initEntity(&cell->entities[idx], type, x, y);
                cell->tiles[y][x].ch       = '.';
                cell->tiles[y][x].walkable = true;
                idx++;
            }
        }
    }
    cell->entity_count = count;
    return 0;
}

/*
 * Libera o vetor dinâmico entities de uma MapCell e zera entity_count.
 * Chamada por: spawnRoomEntities (src/entity.c), freeMap (src/map.c) e tests/test_floor.c.
 */
void freeRoomEntities(MapCell* cell)
{
    if (!cell)
        return;

    free(cell->entities);
    cell->entities     = NULL;
    cell->entity_count = 0;
}

/*
 * Procura e retorna o ponteiro para a entidade ativa na coordenada (x, y) da sala
 * atual, ou NULL caso a posição esteja livre.
 * Chamada por: canHitLineOfSight, fireRangedAttack, movePlayer, chasePlayer e
 * evadePlayer (src/entity.c).
 */
Entity* getEntityAt(int x, int y)
{
    if (!map || map_x < 0 || map_x >= MAP_WIDTH || map_y < 0 || map_y >= MAP_HEIGHT)
        return NULL;

    MapCell* cell = &map[map_y][map_x];
    for (int i = 0; i < cell->entity_count; ++i)
    {
        Entity* e = &cell->entities[i];
        if (e->active && e->pos.x == x && e->pos.y == y)
            return e;
    }
    return NULL;
}

/*
 * Calcula o dano efetivo (raw_attack - target->defense, mínimo 1), exibe o flash
 * visual com drawHitFlash, reduz o HP do alvo e o desativa se chegar a 0.
 * Chamada por: attackEntity e fireRangedAttack (src/entity.c), e tests/test_floor.c.
 */
void dealDamage(int raw_attack, Entity* target)
{
    if (!target || !target->active)
        return;

    int damage = raw_attack - target->defense;
    if (damage < 1)
        damage = 1;

    drawHitFlash(target->pos);

    target->hp -= damage;
    if (target->hp <= 0)
    {
        int      heal_amount = target->healingItem;
        Position drop_pos    = target->pos;

        target->hp     = 0;
        target->active = false;

        if (target->type == ENTITY_BOSS && curr_floor < LAST_FLOOR)
        {
            Position boss_pos = target->pos;

            target->hp     = 0;
            target->active = false;

            MapCell* cell     = &map[map_y][map_x];
            Entity*  new_list = realloc(cell->entities, sizeof(Entity) * (cell->entity_count + 1));
            if (new_list == NULL)
            {
                fprintf(stderr, "Error when allocating the next floor door entity\n");
                return;
            }

            cell->entities = new_list;

            Entity door = {0};
            door.active = true;
            door.ch     = '/';
            door.pos    = boss_pos;
            door.type   = ENTITY_NEXT_FLOOR_DOOR;

            cell->entities[cell->entity_count] = door;
            cell->entity_count++;
            cell->tiles[boss_pos.y][boss_pos.x].walkable = true;
        }
        if (heal_amount > 0)
        {
            MapCell* cell     = &map[map_y][map_x];
            Entity*  new_list = realloc(cell->entities, sizeof(Entity) * (cell->entity_count + 1));
            if (new_list != NULL)
            {
                cell->entities = new_list;
                initEntity(&cell->entities[cell->entity_count], ENTITY_CONSUMABLE, drop_pos.x,
                           drop_pos.y);
                cell->entities[cell->entity_count].hp = heal_amount;
                cell->entity_count++;

                cell->tiles[drop_pos.y][drop_pos.x].walkable = true;
                pushLogQueue("The enemy has spawned a consumable", logs);
            }
        }
    }
}
/*
 * Executa um ataque corpo a corpo de attacker em target aplicando attacker->attack
 * por meio de dealDamage.
 * Chamada por: movePlayer, chasePlayer e actBoss (src/entity.c), e tests/test_floor.c./
 */
void attackEntity(Entity* attacker, Entity* target)
{
    if (!attacker || !attacker->active)
        return;

    dealDamage(attacker->attack, target);
}

/*
 * Verifica se target está alinhado a origin (cardeal, ou também diagonal se
 * allow_diag for true), dentro de range e sem obstáculos no caminho; grava a
 * direção unitária em out_dx/out_dy quando fornecidos.
 * Chamada por: actRangedEnemy e actBoss (src/entity.c).
 */
bool canHitLineOfSight(Position origin, Position target, int range, bool allow_diag, int* out_dx,
                       int* out_dy)
{
    if (!curr_room || range <= 0)
        return false;
    if (target.x < 0 || target.x >= ROOM_WIDTH || target.y < 0 || target.y >= ROOM_HEIGHT)
        return false;

    int diff_x = target.x - origin.x;
    int diff_y = target.y - origin.y;
    int abs_x  = abs(diff_x);
    int abs_y  = abs(diff_y);

    if (!allow_diag && diff_x != 0 && diff_y != 0)
        return false;
    if (allow_diag && diff_x != 0 && diff_y != 0 && abs_x != abs_y)
        return false;

    int dist = (abs_x > abs_y) ? abs_x : abs_y;
    if (dist < 1 || dist > range)
        return false;

    int step_x = (diff_x > 0) - (diff_x < 0);
    int step_y = (diff_y > 0) - (diff_y < 0);

    for (int step = 1; step < dist; ++step)
    {
        int nx = origin.x + step_x * step;
        int ny = origin.y + step_y * step;
        if (nx < 0 || nx >= ROOM_WIDTH || ny < 0 || ny >= ROOM_HEIGHT)
            return false;
        if (!curr_room[ny][nx].walkable)
            return false;
        Entity* blocker = getEntityAt(nx, ny);
        if (blocker != NULL && blocker->type != ENTITY_ITEM)
            return false;
    }

    if (out_dx)
        *out_dx = step_x;
    if (out_dy)
        *out_dy = step_y;
    return true;
}

/*
 * Dispara um projétil em linha reta na direção (dx, dy) até attacker->ranged_range,
 * parando em paredes ou bloqueadores; anima com drawProjectilePath, aplica dano
 * via dealDamage se acertar e registra o resultado em logs.
 * Chamada por: handleInput, actRangedEnemy e actBoss (src/entity.c), e tests/test_floor.c.
 */
bool fireRangedAttack(Entity* attacker, int dx, int dy)
{
    // shortcut: hitscan raycast; swap this body to spawn projectile entities later
    if (!attacker || !attacker->active || attacker->ranged_range <= 0 || !curr_room)
        return false;
    if (dx == 0 && dy == 0)
        return false;
    if (attacker == player && (!player || !player->active || player->hp <= 0))
        return false;

    bool     is_player = (attacker->type == ENTITY_PLAYER);
    Position last_pos  = attacker->pos;
    bool     hit_wall  = false;

    for (int step = 1; step <= attacker->ranged_range; ++step)
    {
        int nx = attacker->pos.x + dx * step;
        int ny = attacker->pos.y + dy * step;

        if (nx < 0 || nx >= ROOM_WIDTH || ny < 0 || ny >= ROOM_HEIGHT ||
            !curr_room[ny][nx].walkable)
        {
            hit_wall = true;
            break;
        }

        last_pos.x = nx;
        last_pos.y = ny;

        if (is_player)
        {
            Entity* target = getEntityAt(nx, ny);
            if (!target || target->type == ENTITY_ITEM)
                continue;

            drawProjectilePath(attacker->pos, last_pos, dx, dy, true, false);
            dealDamage(attacker->ranged_attack, target);
            if (target->hp == 0)
                pushLogQueue("Arrow killed the enemy!", logs);
            else
                pushLogQueue("Arrow hit the enemy!", logs);
            return true;
        }
        else
        {
            if (player && player->active && player->pos.x == nx && player->pos.y == ny)
            {
                drawProjectilePath(attacker->pos, last_pos, dx, dy, false, false);
                dealDamage(attacker->ranged_attack, player);
                pushLogQueue((attacker->type == ENTITY_BOSS) ? "Boss (B) shot you!"
                                                             : "Archer (A) shot you!",
                             logs);
                return true;
            }

            Entity* blocker = getEntityAt(nx, ny);
            if (blocker && blocker->type != ENTITY_ITEM)
            {
                hit_wall = true;
                break;
            }
        }
    }

    drawProjectilePath(attacker->pos, last_pos, dx, dy, is_player, false);
    if (is_player)
        pushLogQueue(hit_wall ? "Arrow hit a wall." : "Arrow out of range.", logs);
    return false;
}

/*
 * Aloca dinamicamente a entidade do jogador e a inicializa em start_pos como
 * ENTITY_PLAYER usando initEntity.
 * Chamada por: main (src/main.c) e tests/test_floor.c.
 */
Entity* createPlayer(Position start_pos)
{
    Entity* newPlayer = malloc(sizeof(Entity));
    if (!newPlayer)
        return NULL;

    initEntity(newPlayer, ENTITY_PLAYER, start_pos.x, start_pos.y);
    return newPlayer;
}

/*
 * Converte a tecla pressionada em movimentação (WASD/hjkl) via movePlayer ou em
 * ataque à distância nas setas direcionais via fireRangedAttack + passTime.
 * Chamada por: gameLoop (src/engine.c) e tests/test_floor.c.
 */
void handleInput(int input)
{
    if (!player || !player->active || player->hp <= 0)
        return;
    int      dxv[8] = {-1, 0, 1, -1, 1, -1, 0, 1};
    int      dyv[8] = {-1, -1, -1, 0, 0, 1, 1, 1};
    int      dx = 0, dy = 0;
    bool     ranged = false;
    Position newPos = player->pos;
    switch (input)
    {
    case 'q':
        newPos.y += dyv[0];
        newPos.x += dxv[0];
        break;
    case 'w':
        newPos.y += dyv[1];
        break;
    case 'e':
        newPos.y += dyv[2];
        newPos.x += dxv[2];
        break;
    case 'a':
        newPos.x += dxv[3];
        break;
    case 'd':
        newPos.x += dxv[4];
        newPos.y += dyv[4];
        break;
    case 'z':
        newPos.x += dxv[5];
        newPos.y += dyv[5];
        break;
    case 's':
        newPos.y += dyv[6];
        break;
    case 'c':
        newPos.x += dxv[7];
        newPos.y += dyv[7];
        break;

    case KEY_UP:
        dy     = -1;
        ranged = true;
        break;
    case KEY_DOWN:
        dy     = 1;
        ranged = true;
        break;
    case KEY_LEFT:
        dx     = -1;
        ranged = true;
        break;
    case KEY_RIGHT:
        dx     = 1;
        ranged = true;
        break;
    default:
        return;
    }

    if (ranged)
    {
        player_moved_turn = false;
        fireRangedAttack(player, dx, dy);
        passTime();
        return;
    }

    movePlayer(newPos);
}

static bool isFinalBossRoomCleared(void)
{
    if (curr_floor < LAST_FLOOR)
        return false;
    MapCell* cell     = &map[map_y][map_x];
    bool     has_boss = false;
    for (int i = 0; i < cell->entity_count; ++i)
    {
        Entity* e = &cell->entities[i];
        if (e->type == ENTITY_BOSS)
            has_boss = true;
        if (e->active && (e->type == ENTITY_ENEMY_MELEE || e->type == ENTITY_ENEMY_RANGED ||
                          e->type == ENTITY_BOSS))
            return false;
    }
    return has_boss;
}

/*
 * Move o jogador para newPos: troca de sala ao pisar em portas, coleta itens
 * (ENTITY_ITEM) ou ataca inimigos adjacentes (attackEntity), salvando a posição
 * anterior para a prioridade de primeiro ataque antes de chamar passTime.
 * Chamada por: handleInput (src/entity.c) e tests/test_floor.c.
 */
void movePlayer(Position newPos)
{
    if (!player || !player->active || player->hp <= 0 || !curr_room)
        return;

    int y = newPos.y, x = newPos.x;
    if (x >= ROOM_WIDTH || x < 0 || y >= ROOM_HEIGHT || y < 0)
        return;

    bool   room_changed = false;
    chtype dest_ch      = curr_room[newPos.y][newPos.x].ch;
    bool   is_door = (dest_ch == ACS_UARROW || dest_ch == ACS_DARROW || dest_ch == ACS_LARROW ||
                      dest_ch == ACS_RARROW);

    if (is_door && isFinalBossRoomCleared())
    {
        char ans =
            confirmationLog("You have defeated the final boss, press 'y' if you want to quit!");
        if (ans == 'y' || ans == 'Y')
            game_won = true;
        return;
    }

    /* Entrada e saida das salas */
    if (curr_room[newPos.y][newPos.x].ch == ACS_UARROW)
    {
        player->pos.y = ROOM_HEIGHT - 2;
        map_y--;
        room_changed = true;
    }
    else if (curr_room[newPos.y][newPos.x].ch == ACS_DARROW)
    {
        player->pos.y = 1;
        map_y++;
        room_changed = true;
    }
    else if (curr_room[newPos.y][newPos.x].ch == ACS_LARROW)
    {
        player->pos.x = ROOM_WIDTH - 2;
        map_x--;
        room_changed = true;
    }
    else if (curr_room[newPos.y][newPos.x].ch == ACS_RARROW)
    {
        player->pos.x = 1;
        map_x++;
        room_changed = true;
    }
    else if (curr_room[newPos.y][newPos.x].walkable)
    {
        prev_player_pos   = player->pos;
        player_moved_turn = false;

        Entity* target = getEntityAt(newPos.x, newPos.y);
        if (target != NULL)
        {
            if (target->type == ENTITY_ITEM || target->type == ENTITY_CONSUMABLE)
            {
                if (player->hp + target->hp >= player->max_hp)
                {
                    player->hp = player->max_hp;
                }
                else
                {
                    player->hp += target->hp;
                }
                if (target->max_hp > 0)
                {
                    player->max_hp += target->max_hp;
                    player->hp = player->max_hp;
                }
                player->attack += target->attack;
                player->defense += target->defense;
                target->active    = false;
                player->pos.y     = newPos.y;
                player->pos.x     = newPos.x;
                player_moved_turn = true;
            }
            else if (target->type == ENTITY_NEXT_FLOOR_DOOR)
            {
                if (confirmationLog("Do you want to go to the next floor? y to confirm") == 'y')
                {
                    newFloor();
                }
                else
                {
                    passTime();
                }
            }
            else
            {
                attackEntity(player, target);
                if (target->hp == 0)
                    pushLogQueue("The enemy died.", logs);
                else
                    pushLogQueue("You attacked.", logs);
            }
        }
        else
        {
            player->pos.y     = newPos.y;
            player->pos.x     = newPos.x;
            player_moved_turn = true;
        }

        passTime();
        player_moved_turn = false;
    }

    if (room_changed)
    {
        curr_room = map[map_y][map_x].tiles;
        updateFogOfWar();
        roomDoors();
    }
}

/*
 * Calcula o menor caminho até o jogador via busca em largura (BFS) com coordQueue:
 * se estiver adjacente (respeitando a prioridade do jogador), ataca com
 * attackEntity; caso contrário, avança um passo na direção do alvo.
 * Chamada por: passTime (src/time.c) e actBoss (src/entity.c).
 */
void chasePlayer(Entity* chaser)
{
    int     dx[]  = {0, 0, -1, 1};
    int     dy[]  = {-1, 1, 0, 0};
    MapCell cell  = map[map_y][map_x];
    Tile**  tiles = cell.tiles;
    // para o backtraking , -1 => indefinido
    Position prev[ROOM_HEIGHT][ROOM_WIDTH];
    // para o bfs, 0 => nao visitado, 1 => visitado
    int visited[ROOM_HEIGHT][ROOM_WIDTH];
    for (int y = 0; y < ROOM_HEIGHT; y++)
    {
        for (int x = 0; x < ROOM_WIDTH; x++)
        {
            prev[y][x].x  = -1;
            prev[y][x].y  = -1;
            visited[y][x] = 0;
        }
    }
    coordQueue* q = initializeCoordQueue();
    pushCoordQueue(chaser->pos.x, chaser->pos.y, q);
    visited[chaser->pos.y][chaser->pos.x] = true;

    bool foundPlayer = false;
    while (q->size > 0)
    {
        Position p = q->front->coord;
        popCoordQueue(q);
        int x = p.x;
        int y = p.y;
        for (int i = 0; i < 4; i++)
        {
            int nx = x + dx[i];
            int ny = y + dy[i];
            // sai da room, ou obstaculo
            if (nx >= ROOM_WIDTH - 1 || nx <= 0 || ny >= ROOM_HEIGHT - 1 || ny <= 0)
                continue;
            if (visited[ny][nx])
                continue;

            Entity* e = getEntityAt(nx, ny);
            // quer dizer que tem uma entidade em nx e ny
            if (e != NULL)
                continue;
            if (ny == player->pos.y && nx == player->pos.x)
            {
                foundPlayer    = true;
                prev[ny][nx].x = x;
                prev[ny][nx].y = y;
                break;
            }
            if (tiles[ny][nx].walkable == false)
                continue;

            visited[ny][nx] = 1;
            prev[ny][nx].x  = x;
            prev[ny][nx].y  = y;
            pushCoordQueue(nx, ny, q);
        }
        if (foundPlayer)
            break;
    }
    if (!foundPlayer)
    {
        eraseCoordQueue(q);
        return;
    }
    // no final vai ser o proximo movimento
    int prevY = prev[player->pos.y][player->pos.x].y;
    int prevX = prev[player->pos.y][player->pos.x].x;

    // quer dizer que ta do lado do player
    if (prevY == chaser->pos.y && prevX == chaser->pos.x)
    {
        // Prioridade de primeiro ataque do player ao entrar na linha de ataque
        if (player_moved_turn && !isCardinalAdjacent(chaser->pos, prev_player_pos))
        {
            eraseCoordQueue(q);
            return;
        }
        attackEntity(chaser, player);
        pushLogQueue("Goblin (E) hit you!", logs);
    }
    else
    {
        // se o proximo for -1 quer dizer que cheguei no inimigo original
        while (prev[prevY][prevX].x != chaser->pos.x || prev[prevY][prevX].y != chaser->pos.y)
        {
            int auxY = prevY, auxX = prevX;
            prevY = prev[auxY][auxX].y;
            prevX = prev[auxY][auxX].x;
        }
        // no final prevX e prevY deve conter as coordenadas para o proximo passo
        chaser->pos.x = prevX;
        chaser->pos.y = prevY;
    }
    eraseCoordQueue(q);
}

/*
 * Controla a IA do Goblin Arqueiro ('A'): decrementa ranged_cooldown se estiver
 * em recarga ou dispara em linha cardeal via fireRangedAttack (apenas mirando no
 * turno em que o jogador acabou de entrar na linha de tiro).
 * Chamada por: passTime (src/time.c).
 */
void actRangedEnemy(Entity* e)
{
    if (!e || !e->active || !player || !player->active)
        return;

    if (e->ranged_cooldown > 0)
    {
        e->ranged_cooldown--;
        return;
    }

    int dx = 0, dy = 0;
    if (canHitLineOfSight(e->pos, player->pos, e->ranged_range, false, &dx, &dy))
    {
        if (player_moved_turn &&
            !canHitLineOfSight(e->pos, prev_player_pos, e->ranged_range, false, NULL, NULL))
        {
            pushLogQueue("Archer (A) aims at you!", logs);
            return;
        }
        fireRangedAttack(e, dx, dy);
        e->ranged_cooldown = 1;
    }
}

/*
 * Calcula uma pontuação heurística para a posição pos usando distância de Chebyshev
 * até target, favorecendo manter-se dentro de range e alinhado para tiro.
 * Chamada por: evadePlayer (src/entity.c).
 */
static int evadeScore(Position pos, Position target, int range)
{
    int dx    = abs(pos.x - target.x);
    int dy    = abs(pos.y - target.y);
    int cheb  = (dx > dy) ? dx : dy;
    int score = (cheb <= range) ? (cheb * 4) : (range * 4 - (cheb - range));
    if (dx == 0 || dy == 0 || dx == dy)
        score += 1;
    return score;
}

/*
 * Avalia os 8 vizinhos caminháveis e livres ao redor de e usando evadeScore e
 * move a entidade para a posição que oferece melhor fuga/posicionamento à distância.
 * Chamada por: actBoss (src/entity.c).
 */
void evadePlayer(Entity* e)
{
    if (!e || !e->active || !player || !player->active || !curr_room)
        return;

    static const int dx[8] = {0, 0, -1, 1, -1, 1, -1, 1};
    static const int dy[8] = {-1, 1, 0, 0, -1, -1, 1, 1};

    int      best_score = evadeScore(e->pos, player->pos, e->ranged_range);
    Position best_pos   = e->pos;

    for (int i = 0; i < 8; ++i)
    {
        int nx = e->pos.x + dx[i];
        int ny = e->pos.y + dy[i];

        if (nx <= 0 || nx >= ROOM_WIDTH - 1 || ny <= 0 || ny >= ROOM_HEIGHT - 1)
            continue;
        if (!curr_room[ny][nx].walkable)
            continue;
        if (nx == player->pos.x && ny == player->pos.y)
            continue;
        if (getEntityAt(nx, ny) != NULL)
            continue;

        Position cand  = {nx, ny};
        int      score = evadeScore(cand, player->pos, e->ranged_range);
        if (score > best_score)
        {
            best_score = score;
            best_pos   = cand;
        }
    }

    e->pos = best_pos;
}

/*
 * Executa a IA do Boss ('B') em duas fases: acima de 50% de HP persegue com
 * chasePlayer e ataca corpo a corpo; com 50% ou menos prioriza tiros em 8
 * direções (cooldown 1), recua com evadePlayer na recarga e ataca melee se encurralado.
 * Chamada por: passTime (src/time.c).
 */
void actBoss(Entity* e)
{
    if (!e || !e->active || !player || !player->active)
        return;

    bool adjacent = isCardinalAdjacent(e->pos, player->pos);

    if (e->hp > e->max_hp / 2)
    {
        if (adjacent)
        {
            if (player_moved_turn && !isCardinalAdjacent(e->pos, prev_player_pos))
                return;
            attackEntity(e, player);
            pushLogQueue("Boss (B) hit you!", logs);
        }
        else
        {
            chasePlayer(e);
        }
        return;
    }

    int dx = 0, dy = 0;
    if (e->ranged_cooldown == 0 &&
        canHitLineOfSight(e->pos, player->pos, e->ranged_range, true, &dx, &dy))
    {
        if (player_moved_turn &&
            !canHitLineOfSight(e->pos, prev_player_pos, e->ranged_range, true, NULL, NULL))
        {
            pushLogQueue("Boss (B) aims at you!", logs);
            return;
        }
        fireRangedAttack(e, dx, dy);
        e->ranged_cooldown = 1;
        return;
    }

    if (e->ranged_cooldown > 0)
        e->ranged_cooldown--;

    Position old_pos = e->pos;
    evadePlayer(e);
    if (e->pos.x == old_pos.x && e->pos.y == old_pos.y && adjacent)
    {
        if (player_moved_turn && !isCardinalAdjacent(e->pos, prev_player_pos))
            return;
        attackEntity(e, player);
        pushLogQueue("Boss (B) hit you!", logs);
    }
}
