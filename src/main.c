#include "rogue.h"
#include <curses.h>

// declaracoes globais
Entity*   player;
Tile**    curr_room;
WINDOW*   win;
WINDOW*   win_minimapa;
WINDOW*   win_hud;
MapCell** map;
logQueue* logs;

// Variáveis globais
int       map_y       = 3;
int       map_x       = 3;
int       curr_floor  = 1;
int       number_of_rooms = 0;
const int ROOM_WIDTH  = 31;
const int ROOM_HEIGHT = 11;
const int MAP_SIZE    = 7;
const int MAP_WIDTH   = 7;
const int MAP_HEIGHT  = 7;
const int LAST_FLOOR   = 2;

/*
 * Ponto de entrada do jogo: inicializa o ncurses e a semente aleatória, executa
 * o loop de partidas (menu inicial, geração do andar, gameLoop, limpeza e Game Over)
 * e encerra a aplicação com closeGame.
 * Chamada por: sistema operacional ao executar o binário principal.
 */
int main(void)
{
    cursesSetup();
    srand((unsigned int)time(NULL));
    initscr();
    set_escdelay(0); 

    while (1)
    {
        MenuOption choice = showMainMenu();
        if (choice == MENU_QUIT)
            break;

        map_x = 3;
        map_y = 3;
        logs  = initializeLogQueue();

        layoutWindows();

        map = createMap();
        if (choice == MENU_CONTINUE)
        {
            if (!continueGame())
            {
                cleanupRun();
                continue;
            }
        }
        else
        {
            number_of_rooms = rand() % 8 + 8;
            setupFloor(map_x, map_y, number_of_rooms);
            Position start_pos = {15, 5};
            player             = createPlayer(start_pos);
        }

        curr_room = map[map_y][map_x].tiles;

        roomDoors();

        bool game_over = gameLoop();
        if (!game_over && player && player->active && player->hp > 0)
            save(player);

        cleanupRun();

        if (game_over)
        {
            if (!showGameOver())
                break;
            eraseSave();
        }
        else
        {
            break;
        }
    }
    closeGame();
    return 0;
}

/*
 * Wrapper de newwin() que cria e retorna uma janela ncurses com dimensões
 * (height x width) na posição (starty, startx).
 * Chamada por: layoutWindows (src/main.c).
 */
WINDOW* create_newwin(int height, int width, int starty, int startx)
{
    return newwin(height, width, starty, startx);
}

/*
 * Libera com delwin() e anula os ponteiros das janelas win_minimapa, win e win_hud
 * caso estejam alocadas.
 * Chamada por: layoutWindows (src/main.c) e cleanupRun (src/engine.c).
 */
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
    if (win_hud)
    {
        delwin(win_hud);
        win_hud = NULL;
    }
}

/*
 * Calcula o posicionamento centralizado das janelas (minimapa, sala e HUD) com base
 * em COLS e LINES e as recria (ou mantém destruídas se o terminal for pequeno demais).
 * Chamada por: main (src/main.c) e gameLoop (src/engine.c, em KEY_RESIZE).
 */
void layoutWindows(void)
{
    int minimap_w = 15, minimap_h = 7;
    int hud_w = 25, hud_h = ROOM_HEIGHT;
    int spacing = 3;
    int total_w = minimap_w + spacing + ROOM_WIDTH + spacing + hud_w;

    // Recria as janelas para impedir que resizeterm() mantenha dimensoes cortadas
    destroyWindows();

    if (COLS < total_w || LINES < ROOM_HEIGHT + 3)
        return;

    int minimap_x = (COLS - total_w) / 2;
    int room_x    = minimap_x + minimap_w + spacing;
    int hud_x     = room_x + ROOM_WIDTH + spacing;
    int room_y    = (LINES - ROOM_HEIGHT) / 2;
    int minimap_y = room_y + (ROOM_HEIGHT - minimap_h) / 2;
    int hud_y     = room_y + (ROOM_HEIGHT - hud_h) / 2;

    win_minimapa = create_newwin(minimap_h, minimap_w, minimap_y, minimap_x);
    win          = create_newwin(ROOM_HEIGHT, ROOM_WIDTH, room_y, room_x);
    win_hud      = create_newwin(hud_h, hud_w, hud_y, hud_x);
    if (!win_minimapa || !win || !win_hud)
        destroyWindows();
}
