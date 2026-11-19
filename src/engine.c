#include "rogue.h"
#include <locale.h>

/*
 * Configura o locale UTF-8, inicializa o modo ncurses (sem eco, cbreak, keypad,
 * cursor oculto) e registra os pares de cores usados em entidades, tiros e dano.
 * Chamada por: main (src/main.c).
 */
void cursesSetup(void)
{
    setlocale(LC_ALL, "");
    initscr();            /* Inicia o modo curses */
    noecho();             /* Faz com que eh digitado nao apareca no term */
    cbreak();             /* Comandos nao sao executados (ex: Ctrl-C) */
    keypad(stdscr, TRUE); /* Permite pegar caracteres como as setas e teclas F(n) */
    curs_set(0);          /* Cursor nao aparece */
    if (has_colors())
    {
        start_color();
        use_default_colors();
        init_pair(COLOR_PAIR_PLAYER, COLOR_GREEN, -1);
        init_pair(COLOR_PAIR_ENEMY, COLOR_RED, -1);
        init_pair(COLOR_PAIR_ARCHER, COLOR_YELLOW, -1);
        init_pair(COLOR_PAIR_BOSS, COLOR_MAGENTA, -1);
        init_pair(COLOR_PAIR_ITEM, COLOR_CYAN, -1);
        init_pair(COLOR_PAIR_SHOT_PLAYER, COLOR_CYAN, -1);
        init_pair(COLOR_PAIR_SHOT_ENEMY, COLOR_RED, -1);
        init_pair(COLOR_PAIR_HIT, COLOR_WHITE, COLOR_RED);
    }
}

/*
 * Verifica se o arquivo de save (SAVE_PATH) existe e pode ser aberto para leitura.
 * Chamada por: showMainMenu (src/engine.c).
 */
bool saveExists(void)
{
    FILE* f = fopen(SAVE_PATH, "rb");
    if (!f)
        return false;
    fclose(f);
    return true;
}

/*
 * Exibe o menu principal centralizado (Start New Game, Continue se saveExists()
 * for verdadeiro, e Quit), trata a navegação por teclado e retorna a MenuOption.
 * Chamada por: main (src/main.c).
 */
MenuOption showMainMenu(void)
{
    const char* labels[3];
    MenuOption  opts[3];
    int         count    = 0;
    int         selected = 0;

    labels[count] = "Start New Game";
    opts[count++] = MENU_NEW_GAME;
    if (saveExists())
    {
        labels[count] = "Continue";
        opts[count++] = MENU_CONTINUE;
    }
    labels[count] = "Quit";
    opts[count++] = MENU_QUIT;

    while (1)
    {
        werase(stdscr);
        int start_y = (LINES - (count + 3)) / 2;
        if (start_y < 0)
            start_y = 0;

        const char* title = "===  DUNGEON C ===";
        int         tx    = (COLS - (int)strlen(title)) / 2;
        if (tx < 0)
            tx = 0;
        mvprintw(start_y, tx, "%s", title);

        for (int i = 0; i < count; ++i)
        {
            int y = start_y + 2 + i;
            if (y >= LINES)
                break;
            int x = (COLS - (int)strlen(labels[i]) - 5) / 2;
            if (x < 0)
                x = 0;
            if (i == selected)
                attron(A_REVERSE);
            mvprintw(y, x, " %s %s  ", (i == selected) ? ">" : " ", labels[i]);
            if (i == selected)
                attroff(A_REVERSE);
        }
        refresh();

        int ch = getch();
        if (ch == KEY_UP || ch == 'w' || ch == 'k')
            selected = (selected - 1 + count) % count;
        else if (ch == KEY_DOWN || ch == 's' || ch == 'j')
            selected = (selected + 1) % count;
        else if (ch == '\n' || ch == '\r' || ch == KEY_ENTER)
            return opts[selected];
        else if (ch == 'q')
            return MENU_QUIT;
    }
}

/*
 * Limpa o buffer de entrada (flushinp), exibe a tela de Game Over com as opções
 * "Voltar ao Menu" e "Sair" e retorna true para reiniciar ou false para sair.
 * Chamada por: main (src/main.c).
 */
bool showGameOver(void)
{
    const char* labels[2] = {"Voltar ao Menu", "Sair"};
    int         count     = 2;
    int         selected  = 0;

    flushinp();

    while (1)
    {
        werase(stdscr);
        int start_y = (LINES - (count + 3)) / 2;
        if (start_y < 0)
            start_y = 0;

        const char* title = "=== GAME OVER ===";
        int         tx    = (COLS - (int)strlen(title)) / 2;
        if (tx < 0)
            tx = 0;
        mvprintw(start_y, tx, "%s", title);

        for (int i = 0; i < count; ++i)
        {
            int y = start_y + 2 + i;
            if (y >= LINES)
                break;
            int x = (COLS - (int)strlen(labels[i]) - 5) / 2;
            if (x < 0)
                x = 0;
            if (i == selected)
                attron(A_REVERSE);
            mvprintw(y, x, " %s %s  ", (i == selected) ? ">" : " ", labels[i]);
            if (i == selected)
                attroff(A_REVERSE);
        }
        refresh();

        int ch = getch();
        if (ch == KEY_UP || ch == 'w' || ch == 'k')
            selected = (selected - 1 + count) % count;
        else if (ch == KEY_DOWN || ch == 's' || ch == 'j')
            selected = (selected + 1) % count;
        else if (ch == '\n' || ch == '\r' || ch == KEY_ENTER)
            return selected == 0;
        else if (ch == 'q')
            return false;
    }
}

/*
 * Executa o loop principal da partida enquanto o jogador estiver ativo e vivo:
 * lê comandos, trata saída ('q'), redimensionamento (KEY_RESIZE) ou ações
 * (handleInput), redesenha a tela e retorna true se o player morreu ou false se saiu.
 * Chamada por: main (src/main.c).
 */
bool gameLoop(void)
{
    int ch;

    drawEverything();

    while (player && player->active && player->hp > 0)
    {
        ch = getch();
        if (ch == 'q')
            return false;

        if (ch == KEY_RESIZE)
            layoutWindows();
        else if (win && win_minimapa && win_hud)
            handleInput(ch);

        drawEverything();

        if (!player || !player->active || player->hp <= 0)
            return true;
    }

    return true;
}

/*
 * Libera todos os recursos alocados para uma partida (janelas, player, fila logs,
 * matriz map e zera curr_room).
 * Chamada por: main (src/main.c) e closeGame (src/engine.c).
 */
void cleanupRun(void)
{
    destroyWindows();

    free(player);
    player = NULL;

    if (logs)
    {
        eraseLogQueue(logs);
        logs = NULL;
    }

    freeMap();
    curr_room = NULL;
}

/*
 * Termina o jogo: libera toda a memória da partida via cleanupRun e encerra o
 * modo ncurses com endwin().
 * Chamada por: main (src/main.c).
 */
void closeGame(void)
{
    cleanupRun();
    endwin();
}
