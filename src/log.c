#include "rogue.h"

void logDefault(const char* msg)
{
    int row = LINES - 2;
    int col = ((COLS - strlen(msg)) / 2);
    if (col < 0)
        col = 0;
    if (row < 0)
        row = 0;
    move(row, 0);
    clrtoeol();
    mvprintw(row, col, "%s", msg);
}
