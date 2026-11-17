#include "rogue.h"

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
        e->hp      = 10;
        e->max_hp  = 10;
        e->attack  = 4;
        e->defense = 0;
        break;
    case ENTITY_ENEMY_MELEE:
        e->ch      = 'E';
        e->hp      = 8;
        e->max_hp  = 8;
        e->attack  = 3;
        e->defense = 0;
        break;
    case ENTITY_ENEMY_RANGED:
        e->ch      = 'A';
        e->hp      = 8;
        e->max_hp  = 8;
        e->attack  = 3;
        e->defense = 0;
        break;
    case ENTITY_BOSS:
        e->ch      = 'B';
        e->hp      = 50;
        e->max_hp  = 50;
        e->attack  = 20;
        e->defense = 10;
        break;
    case ENTITY_ITEM:
        e->ch      = 'I';
        e->hp      = 0;
        e->max_hp  = 0;
        e->attack  = 0;
        e->defense = 0;
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
                // shortcut: apenas desaparece do mapa ate implementar inventario/buffs
                target->active = false;
                player->pos.y  = newPos.y;
                player->pos.x  = newPos.x;
            }
            else
            {
                attackEntity(player, target);
            }
        }
        else
        {
            player->pos.y = newPos.y;
            player->pos.x = newPos.x;
        }
    }

    if (room_changed)
    {
        curr_room = map[map_y][map_x].tiles;
        updateFogOfWar();
        roomDoors();
    }
}
