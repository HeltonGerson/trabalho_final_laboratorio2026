#include "rogue.h"

void cursesSetup()
{
    initscr(); /* Inicia o modo curses */
    noecho();  /* Faz com que eh digitado nao apareca no term */
    cbreak();  /* Comandos nao sao executados (ex: Ctrl-C) */
    keypad(stdscr, TRUE); /* Permite pegar caracteres como as setas e teclas F(n) */
    curs_set(0); /* Cursor nao aparece */
}


void gameLoop()
{
    int ch;

    drawEverything();

    while((ch = getch()) != 'q')
    { 
        handleInput(ch);
        drawEverything();
    }
}

/* Termina o jogo, libera memoria */
void closeGame()
{
    endwin();
    free(player);
    freeRoom();
    freeMap();
}

