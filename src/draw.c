#include "rogue.h"

void drawMap()
{
    for (int y = 0; y < MAP_HEIGHT; ++y)
    {
        for (int x = 0; x < MAP_WIDTH; ++x)
        {
            mvwprintw(win, y, x, "%c", room[y][x].ch);
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
    drawMap();
    drawEntity(player);
    wrefresh(win);
}
