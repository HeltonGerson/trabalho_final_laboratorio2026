#include "rogue.h"

// declaracoes globais
Entity*   player;
Tile**    curr_room;
WINDOW*   win;
WINDOW*   win_minimapa;
MapCell** map;
logQueue* logs;

// indices atuais no mapa
int       map_y       = 3;
int       map_x       = 3;
const int ROOM_WIDTH  = 31;
const int ROOM_HEIGHT = 11;
const int MAP_SIZE    = 7;
const int MAP_WIDTH   = 7;
const int MAP_HEIGHT  = 7;

int main(void)
{
    cursesSetup();

    if (showMainMenu() == MENU_QUIT)
    {
        endwin();
        return 0;
    }

    srand(time(NULL));
    int number_of_rooms = rand() % 8 + 8;
    logs = initializeLogQueue();
    
    layoutWindows();

    map = createMap();
    setupFloor(map_x, map_y, number_of_rooms);

    curr_room = map[map_y][map_x].tiles;

    Position start_pos = {5, 5};
    player             = createPlayer(start_pos);

    roomDoors();
    gameLoop();

    closeGame();
    return 0;
}

WINDOW* create_newwin(int height, int width, int starty, int startx)
{
    return newwin(height, width, starty, startx);
}

void destroyWindows(void)
{
    if (win_minimapa)
    {
        delwin(win_minimapa);
        win_minimapa = NULL;
    }
    if (win)
    {
        delwin(win);
        win = NULL;
    }
}

void layoutWindows(void)
{
    int minimap_w = 15;
    int minimap_h = 7;
    int spacing   = 3;
    int total_w   = minimap_w + spacing + ROOM_WIDTH;

    // Recria as janelas para impedir que resizeterm() mantenha dimensoes cortadas
    destroyWindows();

    if (COLS < total_w || LINES < ROOM_HEIGHT)
        return;

    int minimap_x = (COLS - total_w) / 2;
    int room_x    = minimap_x + minimap_w + spacing;
    int room_y    = (LINES - ROOM_HEIGHT) / 2;
    int minimap_y = room_y + (ROOM_HEIGHT - minimap_h) / 2;

    win_minimapa = create_newwin(minimap_h, minimap_w, minimap_y, minimap_x);
    win          = create_newwin(ROOM_HEIGHT, ROOM_WIDTH, room_y, room_x);
    if (!win_minimapa || !win)
        destroyWindows();
}
