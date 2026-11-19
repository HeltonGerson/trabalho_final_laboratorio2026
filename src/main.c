#include "rogue.h"
#include <curses.h>

// declaracoes globais
Entity*   player;
Tile**    curr_room;
WINDOW*   win;
WINDOW*   win_minimapa;
MapCell** map;

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
    int mReturn = showMainMenu();
    if (mReturn == 3)
    {
        endwin();
        return 0;
    }
    srand(time(NULL));
    layoutWindows();
    map = createMap();


    if(mReturn == 0)
    {
        int number_of_rooms = rand() % 8 + 8;
        setupFloor(map_x, map_y, number_of_rooms);
        Position start_pos = {5, 5};
        player             = createPlayer(start_pos);

    }else if(mReturn == 1)
    {
        continueGame();
    }
    printw("%i %i", map_y, map_x);
    getch();
    curr_room = map[map_y][map_x].tiles;
    curr_room[0][0].ch = 0;
    printw("aaaaaaaaaaaaaa");
    getch();
    printw("%i %i ", map_y, map_x);

    roomDoors();



    gameLoop();
    save(player);
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
