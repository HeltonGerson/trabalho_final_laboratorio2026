#include "rogue.h"
#include <curses.h>
#include <stdio.h>

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

void initEntity(Entity* e, EntityType type, int x, int y)
{
    if (!e)
        return;

    e->pos.x  = x;
    e->pos.y  = y;
    e->type   = type;
    e->active = true;

    switch (type)
    {
    case ENTITY_PLAYER:
        e->ch      = '@';
        e->max_hp  = 10;
        e->hp      = e->max_hp;
        e->attack  = 4;
        e->defense = 0;
        break;
    case ENTITY_ENEMY_MELEE:
        e->ch      = 'E';
        e->max_hp  = 8;
        e->hp      = e->max_hp;
        e->attack  = 3;
        e->defense = 0;
        break;
    case ENTITY_ENEMY_RANGED:
        e->ch      = 'A';
        e->max_hp  = 8;
        e->hp      = e->max_hp;
        e->attack  = 3;
        e->defense = 0;
        break;
    case ENTITY_BOSS:
        e->ch      = 'B';
        e->max_hp  = 50;
        e->hp      = e->max_hp;
        e->attack  = 20;
        e->defense = 10;
        break;
    case ENTITY_ITEM:
        chooseItem(e);
        break;
    }
}

void chooseItem(Entity* e)
{
    int item   = rand() % 8;
    e->ch      = 'I';
    e->hp      = 0;
    e->max_hp  = 0;
    e->attack  = 0;
    e->defense = 0;
    switch (item)
    {       // mudar depois, talvez um txt
    case 1: // hp
        break;
    case 2:
        e->hp = 20;
        break;
    case 3: // healing
        e->max_hp = 5;
        break;
    case 4:
        e->max_hp = 10;
        break;
    case 5: // dano
        e->attack = 5;
        break;
    case 6:
        e->attack = 10;
        break;
    case 7: // defesa
        e->defense = 5;
        break;
    case 8:
        e->defense = 10;
        break;
    }
}

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

void freeRoomEntities(MapCell* cell)
{
    if (!cell)
        return;

    free(cell->entities);
    cell->entities     = NULL;
    cell->entity_count = 0;
}

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

void attackEntity(Entity* attacker, Entity* target)
{
    if (!attacker || !target || !attacker->active || !target->active)
        return;

    int damage = attacker->attack - target->defense;
    if (damage < 1)
        damage = 1;

    target->hp -= damage;
    if (target->hp <= 0)
    {
        target->hp     = 0;
        target->active = false;
    }
}

Entity* createPlayer(Position start_pos)
{
    Entity* newPlayer = malloc(sizeof(Entity));
    if (!newPlayer)
        return NULL;

    initEntity(newPlayer, ENTITY_PLAYER, start_pos.x, start_pos.y);
    return newPlayer;
}

void handleInput(int input)
{
    Position newPos = player->pos;
    switch (input)
    {
    case KEY_UP:
    case 'w':
    case 'k':
        newPos.y--;
        break;
    case KEY_DOWN:
    case 's':
    case 'j':
        newPos.y++;
        break;
    case KEY_LEFT:
    case 'a':
    case 'h':
        newPos.x--;
        break;
    case KEY_RIGHT:
    case 'd':
    case 'l':
        newPos.x++;
        break;
    default:
        return;
    }
    movePlayer(newPos);
}

void movePlayer(Position newPos)
{
    int y = newPos.y, x = newPos.x;
    if (x >= ROOM_WIDTH || x < 0 || y >= ROOM_HEIGHT || y < 0)
        return;

    bool room_changed = false;

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
        Entity* target = getEntityAt(newPos.x, newPos.y);
        if (target != NULL)
        {
            if (target->type == ENTITY_ITEM)
            {
                if (player->hp + target->hp >= player->max_hp)
                {
                    player->hp = player->max_hp;
                }
                else
                {
                    player->hp = target->hp;
                }
                if (target->max_hp > 0)
                {
                    player->max_hp = target->max_hp;
                    player->hp     = player->max_hp;
                }
                player->attack += target->attack;
                player->defense += target->defense;
                target->active = false;
                player->pos.y  = newPos.y;
                player->pos.x  = newPos.x;
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
            player->pos.y = newPos.y;
            player->pos.x = newPos.x;
        }

        passTime();
    }

    if (room_changed)
    {
        curr_room = map[map_y][map_x].tiles;
        updateFogOfWar();
        roomDoors();
    }
}

void chasePlayer(Entity* chaser)
{
    int     dx[]  = {0, 0, -1, 1, -1, 1, -1, 1};
    int     dy[]  = {-1, 1, 0, 0, -1, -1, 1, 1};
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
        for (int i = 0; i < 8; i++)
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
        attackEntity(chaser, player);
        pushLogQueue("The goblin attacked you.", logs);
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
