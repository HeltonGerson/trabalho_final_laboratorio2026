#include "rogue.h"

// desenha a room atual;
void drawRoom()
{
    for (int y = 0; y < ROOM_HEIGHT; ++y)
    {
        for (int x = 0; x < ROOM_WIDTH; ++x)
        {
            mvwaddch(win, y, x, curr_room[y][x].ch);
        }
    }
}

void drawEntity(Entity* e)
{
    mvwprintw(win, e->pos.y, e->pos.x, "%c", e->ch);
}

void drawEverything()
{
    //clear();
    drawRoom();
    drawEntity(player);
    wrefresh(win);
}
