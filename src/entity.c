#include "rogue.h"

static bool     player_moved_turn = false;
static Position prev_player_pos   = {0, 0};

/*
 * Vê se duas posições estão coladinhas uma na outra (nos lados ou nas diagonais).
 * Onde é chamada: chasePlayer e actBossI (src/entity.c).
 */
static bool isCardinalAdjacent(Position a, Position b)
{
    // O nome antigo foi mantido para não quebrar outras partes, mas ela confere as 8 direções ao
    // redor
    int dx = abs(a.x - b.x);
    int dy = abs(a.y - b.y);
    return dx <= 1 && dy <= 1 && (dx != 0 || dy != 0);
}

/*
 * Descobre qual personagem ou item cada letra do desenho da sala representa
 * ('E' goblin de perto, 'A' arqueiro, 'B' chefão, 'I' item); avisa com falso
 * se a letra não for nenhum deles.
 * Onde é chamada: spawnRoomEntities (src/entity.c).
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
 * Prepara um personagem ou item na posição (x, y) com seus valores iniciais
 * (vida, ataque de perto, ataque à distância, alcance, defesa e chance de deixar
 * um item de cura quando morrer); se for um item, chama chooseItem para sortear.
 * Onde é chamada: spawnRoomEntities, createPlayer e dealDamage (src/entity.c),
 * e tests/test_floor.c.
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
    e->aim_turns       = 0;
    e->active          = true;
    e->healingItem     = 0;
    int healing        = rand() % 10 + 1;

    switch (type)
    {
    case ENTITY_PLAYER:
        e->ch            = '@';
        e->max_hp        = 20;
        e->hp            = e->max_hp;
        e->attack        = 4;
        e->ranged_attack = 2;
        e->ranged_range  = 8;
        e->defense       = 1;
        break;
    case ENTITY_ENEMY_MELEE:
        e->ch      = 'E';
        e->max_hp  = 6;
        e->hp      = e->max_hp;
        e->attack  = 4;
        e->defense = 0;
        if (healing == 1)
            e->healingItem = 4;
        else if (healing == 2)
            e->healingItem = 7;
        break;
    case ENTITY_ENEMY_RANGED:
        e->ch            = 'A';
        e->max_hp        = 6;
        e->hp            = e->max_hp;
        e->attack        = 3;
        e->ranged_attack = 2;
        e->ranged_range  = 5;
        e->defense       = 0;
        if (healing == 1)
            e->healingItem = 4;
        else if (healing == 2)
            e->healingItem = 7;
        break;
    case ENTITY_BOSS:
        e->ch            = 'B';
        e->max_hp        = 35;
        e->hp            = e->max_hp;
        e->attack        = 7;
        e->ranged_attack = 6;
        e->ranged_range  = 6;
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

    // shortcut: usa 0.35f local para nao alterar ENEMIES_MULTIPLIER de outro autor em main.c
    if (curr_floor > 1 &&
        (type == ENTITY_ENEMY_MELEE || type == ENTITY_ENEMY_RANGED || type == ENTITY_BOSS))
    {
        int   extra      = curr_floor - 1;
        float mult       = 1.0f + extra * 0.35f;
        e->max_hp        = (int)(e->max_hp * mult + 0.5f);
        e->hp            = e->max_hp;
        e->attack        = (int)(e->attack * mult + 0.5f);
        e->ranged_attack = (int)(e->ranged_attack * mult + 0.5f);
        if (type != ENTITY_ENEMY_RANGED)
            e->defense += extra;
    }
}

/*
 * Limpa do desenho da sala as letras antigas de inimigos e itens, deixando só
 * chão ('.') e paredes ('#'), para usar quando carrega um jogo salvo.
 * Onde é chamada: continueGame (src/engine.c).
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
 * Configura um item ('I'), sorteando um bônus de vida máxima, ataque ou defesa,
 * ou marca como item de cura ('C').
 * Onde é chamada: initEntity (src/entity.c).
 */
void chooseItem(Entity* e, char type)
{
    e->hp            = 0;
    e->max_hp        = 0;
    e->attack        = 0;
    e->ranged_attack = 0;
    e->defense       = 0;
    if (type == 'i')
    {
        int item = rand() % 8 + 1;
        e->ch    = 'I';
        switch (item)
        {       // No futuro dá para ler esses itens de um arquivo de texto
        case 1: // Aumenta a vida máxima
            e->max_hp = 4;
            break;
        case 2:
            e->max_hp = 7;
            break;
        case 3: // Aumenta o ataque
            e->attack = 1;
            break;
        case 4:
            e->attack = 2;
            break;
        case 5: // Aumenta a defesa
            e->defense = 1;
            break;
        case 6:
            e->defense = 2;
            break;
        case 7: // Aumenta o ataque do arco
            e->ranged_attack = 1;
            break;
        case 8:
            e->ranged_attack = 2;
            break;
        }
    }
    else if (type == 'c')
    {
        e->ch = 'C';
    }
}

/*
 * Procura no desenho da sala todas as letras de inimigos e itens, cria a lista
 * com todos eles na posição certa e troca essas letras por chão comum ('.').
 * Onde é chamada: setRoom (src/room.c).
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
 * Limpa da memória a lista de inimigos, itens e portas especiais de uma sala.
 * Onde é chamada: spawnRoomEntities (src/entity.c), freeMap (src/map.c) e
 * tests/test_floor.c.
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
 * Procura se existe algum personagem, item ou porta especial ativo naquela
 * posição (x, y) da sala atual, ou avisa que o espaço está livre.
 * Onde é chamada: canHitLineOfSight, fireRangedAttack, movePlayer, chasePlayer,
 * hasBresenhamLineOfSight e evadePlayer (src/entity.c), e tests/test_floor.c.
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
 * Calcula o dano (ataque menos defesa, sempre tirando pelo menos 1), faz piscar
 * o 'X' vermelho no alvo, diminui a vida dele e avisa nas mensagens. Se o alvo
 * morrer, desativa ele; se for o chefão, faz aparecer a passagem para o próximo
 * andar; se ele carregava cura, deixa um item de cura no chão; e se era o
 * último inimigo da sala, destranca as portas.
 * Onde é chamada: attackEntity, fireRangedAttack e actRangedEnemy (src/entity.c),
 * e tests/test_floor.c.
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
    if (target->hp < 0)
        target->hp = 0;

    if (target->type == ENTITY_ENEMY_MELEE || target->type == ENTITY_ENEMY_RANGED ||
        target->type == ENTITY_BOSS)
    {
        // Usa um espaço fixo para a mensagem porque acontece no máximo um golpe do jogador por
        // turno antes de mostrar na tela
        static char combat_log[64];
        const char* name = (target->type == ENTITY_BOSS)           ? "Boss (B)"
                           : (target->type == ENTITY_ENEMY_RANGED) ? "Archer (A)"
                                                                   : "Goblin (E)";
        if (target->hp == 0)
            snprintf(combat_log, sizeof(combat_log), "You killed %s! HP: 0/%d", name,
                     target->max_hp);
        else
            snprintf(combat_log, sizeof(combat_log), "You hit %s! HP: %d/%d", name, target->hp,
                     target->max_hp);
        pushLogQueue(combat_log, logs);
    }

    if (target->hp <= 0)
    {
        int        heal_amount = target->healingItem;
        Position   drop_pos    = target->pos;
        EntityType target_type = target->type;

        target->hp     = 0;
        target->active = false;

        if (target_type == ENTITY_BOSS)
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
        if ((target_type == ENTITY_ENEMY_MELEE || target_type == ENTITY_ENEMY_RANGED ||
             target_type == ENTITY_BOSS) &&
            !roomHasAliveEnemies())
        {
            roomDoors();
        }
    }
}
/*
 * Faz um personagem dar um golpe de perto em outro usando a força do seu ataque.
 * Onde é chamada: movePlayer, chasePlayer e actBossI (src/entity.c), e tests/test_floor.c.
 */
void attackEntity(Entity* attacker, Entity* target)
{
    if (!attacker || !attacker->active)
        return;

    dealDamage(attacker->attack, target);
}

/*
 * Confere se o alvo está alinhado em linha reta (nos lados, ou também nas
 * diagonais se permitido), dentro do alcance e sem paredes ou obstáculos no
 * meio; se puder acertar, guarda a direção do tiro.
 * Onde é chamada: actBossI (src/entity.c).
 */
bool canHitLineOfSight(Position origin, Position target, int range, bool allow_diag, int* out_dx, int* out_dy)
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
        if (blocker != NULL && blocker->type != ENTITY_ITEM &&
            blocker->type != ENTITY_CONSUMABLE && blocker->type != ENTITY_NEXT_FLOOR_DOOR)
            return false;
    }

    if (out_dx)
        *out_dx = step_x;
    if (out_dy)
        *out_dy = step_y;
    return true;
}

/*
 * Dispara uma flecha ou tiro em linha reta até atingir o alcance máximo, uma
 * parede ou alguém; mostra a animação do tiro, aplica dano se acertar e manda
 * uma mensagem contando o que aconteceu.
 * Onde é chamada: handleInput e actBossI (src/entity.c), e tests/test_floor.c.
 */
bool fireRangedAttack(Entity* attacker, int dx, int dy)
{
    // O tiro acerta na mesma hora em linha reta; no futuro pode virar um projétil que anda turno a
    // turno
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
            if (!target || target->type == ENTITY_ITEM || target->type == ENTITY_CONSUMABLE ||
                target->type == ENTITY_NEXT_FLOOR_DOOR)
                continue;

            drawProjectilePath(attacker->pos, last_pos, dx, dy, true, false);
            dealDamage(attacker->ranged_attack, target);
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
            if (blocker && blocker->type != ENTITY_ITEM && blocker->type != ENTITY_CONSUMABLE &&
                blocker->type != ENTITY_NEXT_FLOOR_DOOR)
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
 * Cria o jogador na memória e o coloca na posição inicial da sala.
 * Onde é chamada: main (src/main.c) e tests/test_floor.c.
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
 * Lê a tecla que o jogador apertou: as teclas w/a/s/d e q/e/z/c fazem o
 * jogador andar nas 8 direções, e as setas do teclado disparam flechas e fazem
 * o turno passar.
 * Onde é chamada: gameLoop (src/engine.c) e tests/test_floor.c.
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

/*
 * Confere se ainda existe algum inimigo ou chefão vivo na sala atual.
 * Onde é chamada: dealDamage e movePlayer (src/entity.c), roomDoors (src/room.c)
 * e tests/test_floor.c.
 */
bool roomHasAliveEnemies(void)
{
    if (!map || map_x < 0 || map_x >= MAP_WIDTH || map_y < 0 || map_y >= MAP_HEIGHT)
        return false;

    MapCell* cell = &map[map_y][map_x];
    for (int i = 0; i < cell->entity_count; ++i)
    {
        Entity* e = &cell->entities[i];
        if (e->active && e->hp > 0 &&
            (e->type == ENTITY_ENEMY_MELEE || e->type == ENTITY_ENEMY_RANGED ||
             e->type == ENTITY_BOSS))
            return true;
    }
    return false;
}

/*
 * Tenta mover o jogador para a nova posição: se pisar numa porta, muda de sala
 * e atualiza o minimapa e as portas; se houver item ou cura, pega o bônus; se
 * for a passagem de andar, pergunta se quer subir de andar ou finalizar a
 * vitória no último andar; se houver inimigo, ataca de perto; se estiver livre,
 * anda para lá (guardando onde estava antes para ter a chance de atacar primeiro)
 * e passa o turno.
 * Onde é chamada: handleInput (src/entity.c) e tests/test_floor.c.
 */
void movePlayer(Position newPos)
{
    if (!player || !player->active || player->hp <= 0 || !curr_room)
        return;

    int y = newPos.y, x = newPos.x;
    if (x >= ROOM_WIDTH || x < 0 || y >= ROOM_HEIGHT || y < 0)
        return;

    bool room_changed = false;

    /* Verifica se o jogador pisou em uma das portas para trocar de sala */
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
                    player->hp += target->max_hp;
                }
                player->attack += target->attack;
                player->ranged_attack += target->ranged_attack;
                player->defense += target->defense;
                target->active    = false;
                player->pos.y     = newPos.y;
                player->pos.x     = newPos.x;
                player_moved_turn = true;
            }
            else if (target->type == ENTITY_NEXT_FLOOR_DOOR)
            {
                if (curr_floor >= LAST_FLOOR)
                {
                    if (roomHasAliveEnemies())
                    {
                        pushLogQueue("Defeat all enemies in the room first!", logs);
                    }
                    else
                    {
                        char ans = confirmationLog(
                            "You have defeated the final boss, press 'y' if you want to quit!");
                        if (ans == 'y' || ans == 'Y')
                        {
                            game_won = true;
                            return;
                        }
                    }
                }
                else if (confirmationLog("Do you want to go to the next floor? y to confirm") ==
                         'y')
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
        for (int i = 0; i < map[map_y][map_x].entity_count; i++)
        {
            if (map[map_y][map_x].entities[i].ch == 'B' &&
                map[map_y][map_x].entities[i].active == true)
            {
                pushLogQueue("You entered the boss room", logs);
                break;
            }
            else if (map[map_y][map_x].entities[i].ch == 'I' &&
                     map[map_y][map_x].entities[i].active == true)
            {
                pushLogQueue("You entered a item room", logs);
                break;
            }
        }
        updateFogOfWar();
        roomDoors();
    }
}

/*
 * Faz o inimigo procurar o caminho mais curto até o jogador desviando de
 * paredes e outros personagens. Se já estiver colado no jogador, ataca (a
 * menos que o jogador tenha acabado de se aproximar nesse turno, dando a chance
 * do jogador bater primeiro); senão, dá um passo na direção dele.
 * Onde é chamada: passTime (src/time.c) e actBossI (src/entity.c).
 */
void chasePlayer(Entity* chaser)
{
    int     dx[]  = {0, 0, -1, 1, -1, 1, -1, 1};
    int     dy[]  = {-1, 1, 0, 0, -1, -1, 1, 1};
    MapCell cell  = map[map_y][map_x];
    Tile**  tiles = cell.tiles;
    // Guarda de onde viemos para reconstruir o caminho de volta (-1 significa ainda não preenchido)
    Position prev[ROOM_HEIGHT][ROOM_WIDTH];
    // Marca quais quadradinhos já foram olhados (0 = ainda não, 1 = já olhado)
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
        for (int i = 0; i < 8; i++)
        {
            int nx = x + dx[i];
            int ny = y + dy[i];
            // Ignora as paredes da borda da sala
            if (nx >= ROOM_WIDTH - 1 || nx <= 0 || ny >= ROOM_HEIGHT - 1 || ny <= 0)
                continue;
            if (visited[ny][nx])
                continue;

            Entity* e = getEntityAt(nx, ny);
            // Ignora se já tiver outro personagem ou item parado nesse lugar
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
    // Posição logo antes do jogador no caminho encontrado
    int prevY = prev[player->pos.y][player->pos.x].y;
    int prevX = prev[player->pos.y][player->pos.x].x;

    // Se essa posição já for a do próprio inimigo, quer dizer que ele está colado no jogador
    if (prevY == chaser->pos.y && prevX == chaser->pos.x)
    {
        // Se o jogador acabou de se aproximar nesse turno, o inimigo espera para o jogador ter a
        // chance de bater primeiro
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
        // Volta pelo caminho guardado até achar o primeiro passo saindo do inimigo
        while (prev[prevY][prevX].x != chaser->pos.x || prev[prevY][prevX].y != chaser->pos.y)
        {
            int auxY = prevY, auxX = prevX;
            prevY = prev[auxY][auxX].y;
            prevX = prev[auxY][auxX].x;
        }
        // Agora prevX e prevY têm o próximo quadradinho para onde o inimigo deve andar
        chaser->pos.x = prevX;
        chaser->pos.y = prevY;
    }
    eraseCoordQueue(q);
}

/*
 * Traça uma linha reta em qualquer ângulo para ver se o inimigo consegue
 * enxergar o jogador dentro do alcance, sem paredes nem outros inimigos no
 * caminho.
 * Onde é chamada: actRangedEnemy (src/entity.c).
 */
static bool hasBresenhamLineOfSight(Position origin, Position target, int range, const Entity* ignore)
{
    if (!curr_room || range <= 0)
        return false;
    if (origin.x < 0 || origin.x >= ROOM_WIDTH || origin.y < 0 || origin.y >= ROOM_HEIGHT)
        return false;
    if (target.x < 0 || target.x >= ROOM_WIDTH || target.y < 0 || target.y >= ROOM_HEIGHT)
        return false;

    int dx      = target.x - origin.x;
    int dy      = target.y - origin.y;
    int dist_sq = dx * dx + dy * dy;
    if (dist_sq <= 0 || dist_sq > range * range)
        return false;

    int x   = origin.x;
    int y   = origin.y;
    int sx  = (origin.x < target.x) ? 1 : -1;
    int sy  = (origin.y < target.y) ? 1 : -1;
    int adx = abs(dx);
    int ady = abs(dy);
    int err = adx - ady;

    while (x != target.x || y != target.y)
    {
        int e2 = 2 * err;
        if (e2 > -ady)
        {
            err -= ady;
            x += sx;
        }
        if (e2 < adx)
        {
            err += adx;
            y += sy;
        }
        if (x == target.x && y == target.y)
            break;
        if (x < 0 || x >= ROOM_WIDTH || y < 0 || y >= ROOM_HEIGHT || !curr_room[y][x].walkable)
            return false;

        Entity* blocker = getEntityAt(x, y);
        if (blocker && blocker != ignore && blocker->type != ENTITY_ITEM &&
            blocker->type != ENTITY_CONSUMABLE && blocker->type != ENTITY_NEXT_FLOOR_DOOR)
            return false;
    }

    return true;
}

/*
 * Controla o arqueiro ('A'): se ele acabou de atirar, espera os turnos de
 * descanso passarem; se enxergar o jogador em 360°, mira por 2 turnos avisando
 * antes, e no terceiro dispara uma flecha e descansa por 3 turnos; se não
 * enxergar, anda em 8 direções via BFS até a casa mais próxima com visão 360°
 * (desempatando pela maior distância do jogador).
 * Onde é chamada: passTime (src/time.c).
 */
void actRangedEnemy(Entity* e)
{
    if (!e || !e->active || !player || !player->active)
        return;

    if (e->ranged_cooldown > 0)
    {
        e->ranged_cooldown--;
        e->aim_turns = 0;
        return;
    }

    if (hasBresenhamLineOfSight(e->pos, player->pos, e->ranged_range, e))
    {
        if (e->aim_turns == 0)
        {
            e->aim_turns = 1;
            pushLogQueue("Archer (A) aims at you!", logs);
            return;
        }

        if (e->aim_turns == 1)
        {
            e->aim_turns = 2;
            pushLogQueue("Archer (A) is about to shoot!", logs);
            return;
        }

        drawBresenhamProjectilePath(e->pos, player->pos, false, false);
        dealDamage(e->ranged_attack, player);
        pushLogQueue("Archer (A) shot you!", logs);
        e->aim_turns       = 0;
        e->ranged_cooldown = 3;
        return;
    }

    e->aim_turns = 0;

    if (!map || map_x < 0 || map_x >= MAP_WIDTH || map_y < 0 || map_y >= MAP_HEIGHT)
        return;

    // Reposicionamento via BFS por camadas:
    // Regra 1: menor numero de passos ate uma casa com linha de tiro valida.
    // Regra 2: entre casas com o mesmo numero minimo de passos, escolher a mais distante do player.
    MapCell cell  = map[map_y][map_x];
    Tile**  tiles = cell.tiles;
    if (!tiles)
        return;

    int      dir_x[8] = {0, 0, -1, 1, -1, 1, -1, 1};
    int      dir_y[8] = {-1, 1, 0, 0, -1, -1, 1, 1};
    Position prev[ROOM_HEIGHT][ROOM_WIDTH];
    int      dist[ROOM_HEIGHT][ROOM_WIDTH];

    for (int y = 0; y < ROOM_HEIGHT; y++)
    {
        for (int x = 0; x < ROOM_WIDTH; x++)
        {
            prev[y][x].x = -1;
            prev[y][x].y = -1;
            dist[y][x]   = -1;
        }
    }

    coordQueue* q = initializeCoordQueue();
    pushCoordQueue(e->pos.x, e->pos.y, q);
    dist[e->pos.y][e->pos.x] = 0;

    int      best_steps        = -1;
    int      best_player_dist2 = -1;
    Position best_target       = {-1, -1};

    while (q->size > 0)
    {
        Position p = q->front->coord;
        popCoordQueue(q);

        int curr_dist = dist[p.y][p.x];

        // Ja encontramos todas as casas possiveis na camada minima; encerra o BFS
        if (best_steps != -1 && curr_dist >= best_steps)
            break;

        for (int i = 0; i < 8; i++)
        {
            int nx = p.x + dir_x[i];
            int ny = p.y + dir_y[i];

            if (nx <= 0 || nx >= ROOM_WIDTH - 1 || ny <= 0 || ny >= ROOM_HEIGHT - 1)
                continue;
            if (dist[ny][nx] != -1)
                continue;
            if (!tiles[ny][nx].walkable)
                continue;
            if (ny == player->pos.y && nx == player->pos.x)
                continue;
            if (getEntityAt(nx, ny) != NULL)
                continue;

            int next_dist = curr_dist + 1;
            dist[ny][nx]  = next_dist;
            prev[ny][nx]  = p;

            Position cand = {nx, ny};
            if (hasBresenhamLineOfSight(cand, player->pos, e->ranged_range, e))
            {
                int pdx   = nx - player->pos.x;
                int pdy   = ny - player->pos.y;
                int dist2 = pdx * pdx + pdy * pdy;

                if (best_steps == -1 || dist2 > best_player_dist2)
                {
                    best_steps        = next_dist;
                    best_player_dist2 = dist2;
                    best_target       = cand;
                }
            }
            else if (best_steps == -1)
            {
                pushCoordQueue(nx, ny, q);
            }
        }
    }

    // Nenhuma casa com linha de tiro alcancavel
    if (best_steps == -1)
    {
        eraseCoordQueue(q);
        return;
    }

    // Backtracking de best_target ate o primeiro passo adjacente ao arqueiro
    int stepX = best_target.x;
    int stepY = best_target.y;
    while (prev[stepY][stepX].x != e->pos.x || prev[stepY][stepX].y != e->pos.y)
    {
        int auxX = stepX;
        int auxY = stepY;
        stepX    = prev[auxY][auxX].x;
        stepY    = prev[auxY][auxX].y;
    }

    e->pos.x = stepX;
    e->pos.y = stepY;
    eraseCoordQueue(q);
}

/*
 * Dá uma nota para uma posição da sala dizendo o quão boa ela é para fugir do
 * jogador sem sair do alcance do tiro, preferindo ficar alinhado para atirar.
 * Onde é chamada: evadePlayer (src/entity.c).
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
 * Olha os 8 espaços em volta do inimigo e move ele para o espaço livre que der
 * a melhor nota de fuga.
 * Onde é chamada: guardada como função de apoio (não é chamada durante a partida).
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
 * Controla o chefão ('B'): se estiver em descanso do tiro, diminui o contador;
 * se estiver colado no jogador, dá um golpe de perto (respeitando a chance do
 * jogador bater primeiro quando acaba de se aproximar); senão, se não estiver
 * em descanso e enxergar o jogador em 360° dentro do alcance, mira parado por
 * 2 turnos avisando antes e dispara no terceiro entrando em descanso de 3
 * turnos; se perder a visão enquanto mirava, espera 1 turno parado; caso
 * contrário, persegue o jogador pela sala.
 * Onde é chamada: passTime (src/time.c).
 */
void actBossI(Entity* e)
{
    if (!e || !e->active || !player || !player->active)
        return;

    bool on_ranged_cooldown = (e->ranged_cooldown > 0);
    if (on_ranged_cooldown)
    {
        e->ranged_cooldown--;
        e->aim_turns = 0;
    }

    bool adjacent = isCardinalAdjacent(e->pos, player->pos);

    if (adjacent)
    {
        e->aim_turns = 0;
        if (player_moved_turn && !isCardinalAdjacent(e->pos, prev_player_pos))
            return;

        attackEntity(e, player);
        pushLogQueue("Boss (B) hit you!", logs);
        return;
    }

    if (!on_ranged_cooldown && hasBresenhamLineOfSight(e->pos, player->pos, e->ranged_range, e))
    {
        if (e->aim_turns == 0)
        {
            e->aim_turns = 1;
            pushLogQueue("Boss (B) aims at you!", logs);
            return;
        }

        if (e->aim_turns == 1)
        {
            e->aim_turns = 2;
            pushLogQueue("Boss (B) is about to shoot!", logs);
            return;
        }

        drawBresenhamProjectilePath(e->pos, player->pos, false, false);
        dealDamage(e->ranged_attack, player);
        pushLogQueue("Boss (B) shot you!", logs);
        e->aim_turns       = 0;
        e->ranged_cooldown = 3;
        return;
    }

    if (e->aim_turns > 0)
    {
        e->aim_turns = 0;
        return;
    }

    // Não deixa chasePlayer atacar na diagonal se o chefão estiver a 1 passo na diagonal
    int dist_x = abs(e->pos.x - player->pos.x);
    int dist_y = abs(e->pos.y - player->pos.y);
    if (dist_x == 1 && dist_y == 1)
        return;

    chasePlayer(e);
}
