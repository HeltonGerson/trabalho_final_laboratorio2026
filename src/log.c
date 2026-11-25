#include "rogue.h"
#include <curses.h>

/*
 * Centraliza e imprime a mensagem msg na penúltima linha do terminal (LINES - 2),
 * limpando o conteúdo anterior da linha.
 * Chamada por: drawLogs (src/draw.c).
 */
void logDefault(const char* msg)
{
    int row = LINES - 2;
    int col = (COLS - (int)strlen(msg)) / 2;
    if (col < 0)
        col = 0;
    if (row < 0)
        row = 0;
    move(row, 0);
    clrtoeol();
    mvprintw(row, col, "%s", msg);
}

char confirmationLog(char* msg)
{
    logDefault(msg);
    char resposta = getch();
    return resposta;
}
