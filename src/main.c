#include "rogue.h"
#include <curses.h>

// Informações principais usadas pelo jogo inteiro
Entity*   player;
Tile**    curr_room;
WINDOW*   win;
WINDOW*   win_minimapa;
WINDOW*   win_hud;
MapCell** map;
logQueue* logs;

// Posição, andar atual e tamanhos fixos das salas e do mapa
int         map_y              = 3;
int         map_x              = 3;
int         curr_floor         = 1;
bool        game_won           = false;
int         number_of_rooms    = 0;
const int   ROOM_WIDTH         = 31;
const int   ROOM_HEIGHT        = 11;
const int   MAP_SIZE           = 7;
const int   MAP_WIDTH          = 7;
const int   MAP_HEIGHT         = 7;
const int   LAST_FLOOR         = 2;
const float ENEMIES_MULTIPLIER = 0.15;

/*
 * É por aqui que o jogo começa: prepara a tela e os sorteios, mostra o menu
 * inicial, monta ou carrega a partida, roda o jogo, salva o progresso se o
 * jogador sair vivo, mostra a tela de vitória ou de fim de jogo e fecha tudo
 * quando o jogador decide sair.
 * Onde é chamada: pelo próprio computador quando o jogador abre o jogo.
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

        game_won = false;
        map_x    = 3;
        map_y    = 3;
        logs     = initializeLogQueue();

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
            curr_floor      = 1;
            number_of_rooms = rand() % 8 + 8;
            setupFloor(map_x, map_y, number_of_rooms);
            Position start_pos = {15, 5};
            player             = createPlayer(start_pos);
            pushLogQueue("You must escape the dungeon", logs);
        }

        curr_room = map[map_y][map_x].tiles;

        roomDoors();

        bool game_over = gameLoop();
        if (!game_over && !game_won && player && player->active && player->hp > 0)
            save(player);

        if (game_won)
        {
            bool back_to_menu = winGame();
            cleanupRun();
            if (!back_to_menu)
                break;
        }
        else
        {
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
    }
    closeGame();
    return 0;
}

/*
 * Cria uma janelinha nova na tela com a altura, largura e posição escolhidas.
 * Onde é chamada: layoutWindows (src/main.c).
 */
WINDOW* create_newwin(int height, int width, int starty, int startx)
{
    return newwin(height, width, starty, startx);
}

/*
 * Fecha e limpa da memória as três janelinhas do jogo (minimapa, sala e painel
 * de status) se elas estiverem abertas.
 * Onde é chamada: layoutWindows (src/main.c) e cleanupRun (src/engine.c).
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
 * Mede o tamanho da tela do terminal para deixar o minimapa, a sala e o painel
 * de status bem centralizados e recria essas três janelinhas (ou deixa fechadas
 * se a janela do terminal estiver pequena demais).
 * Onde é chamada: main (src/main.c) e gameLoop (src/engine.c, quando o terminal muda de tamanho).
 */
void layoutWindows(void)
{
    int minimap_w = 15, minimap_h = 7;
    int hud_w = 25, hud_h = ROOM_HEIGHT;
    int spacing = 3;
    int total_w = minimap_w + spacing + ROOM_WIDTH + spacing + hud_w;

    // Fecha e cria as janelinhas de novo para não ficarem cortadas se a tela mudar de tamanho
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
