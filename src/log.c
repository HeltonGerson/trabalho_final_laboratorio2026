#include "rogue.h"

/*
 * Limpa a penúltima linha da tela e escreve uma mensagem bem centralizada ali.
 * Onde é chamada: drawLogs (src/draw.c) e confirmationLog (src/log.c).
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

/*
 * Mostra uma pergunta na parte de baixo da tela e espera o jogador apertar uma
 * tecla para responder.
 * Onde é chamada: movePlayer (src/entity.c).
 */
char confirmationLog(char* msg)
{
    logDefault(msg);
    char resposta = getch();
    return resposta;
}
