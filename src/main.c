#include "rogue.h"

// declaracoes globais
Entity*   player;
Tile**    curr_room;
WINDOW*   win;
WINDOW*   win_minimapa;
MapCell** map;

// indices atuais no mapa
int map_y = 3;
int map_x = 3;
const int ROOM_WIDTH  = 32;
const int ROOM_HEIGHT = 10;
const int MAP_SIZE    = 7;
const int MAP_WIDTH   = 7;
const int MAP_HEIGHT  = 7;

int main(void)
{
    cursesSetup();

    srand(time(NULL));
    int number_of_rooms = rand() % 45 + 4;
    int minimap_w = 17;
    int minimap_h = 7;
    int spacing = 3;
    int total_w = minimap_w + spacing + ROOM_WIDTH;
    int start_x = (COLS - total_w) / 2;
    int start_y = (LINES - ROOM_HEIGHT) / 2;
    
    win_minimapa = create_newwin(minimap_h, minimap_w, start_y + (ROOM_HEIGHT - minimap_h) / 2, start_x);
    win = create_newwin(ROOM_HEIGHT, ROOM_WIDTH, start_y, start_x + minimap_w + spacing);

    map = createMap();
    setupFloor(map_x, map_y, number_of_rooms);

    curr_room = map[map_y][map_x].tiles;

    Position start_pos = {5, 5};
    player = createPlayer(start_pos);

    roomDoors();
    refresh();
    gameLoop();

    closeGame();
    return 0;
}

Entity* createPlayer(Position start_pos)
{
    Entity* newPlayer = malloc(sizeof(Entity));
    if (!newPlayer)
        return NULL;

    newPlayer->pos.y = start_pos.y;
    newPlayer->pos.x = start_pos.x;
    newPlayer->ch = '@';

    return newPlayer;
}

void handleInput(int input)
{
    Position newPos = player->pos;
    switch(input)
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
        player->pos.y = newPos.y;
        player->pos.x = newPos.x;
    }

    if (room_changed)
    {
        curr_room = map[map_y][map_x].tiles;
        updateFogOfWar();
        roomDoors();
    }
}

WINDOW *create_newwin(int height, int width, int starty, int startx)
{
    WINDOW *local_win;
    local_win = newwin(height, width, starty, startx);
    wrefresh(local_win);
    return local_win;
}
