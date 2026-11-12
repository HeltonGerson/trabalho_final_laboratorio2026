#include "rogue.h"

void cursesSetup()
{
    initscr();            /* Inicia o modo curses */
    noecho();             /* Faz com que eh digitado nao apareca no term */
    cbreak();             /* Comandos nao sao executados (ex: Ctrl-C) */
    keypad(stdscr, TRUE); /* Permite pegar caracteres como as setas e teclas F(n) */
    curs_set(0);          /* Cursor nao aparece */
}

void gameLoop()
{
    int ch;

    drawEverything();

    while ((ch = getch()) != 'q')
    {
        if (ch == KEY_RESIZE)
        {
            layoutWindows();
            erase();
            refresh();
        }
        else
        {
            handleInput(ch);
        }
        drawEverything();
    }
}

/* Termina o jogo, libera memoria */
void closeGame(void)
{
    if (win_minimapa)
        delwin(win_minimapa);
    if (win)
        delwin(win);
    endwin();

    free(player);
    player = NULL;

    freeMap();
}
