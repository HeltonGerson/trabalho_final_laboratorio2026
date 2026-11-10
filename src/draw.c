#include "rogue.h"

// desenha a room atual
void drawRoom(void)
{
    for (int y = 0; y < ROOM_HEIGHT; ++y)
    {
        for (int x = 0; x < ROOM_WIDTH; ++x)
        {
            mvwaddch(win, y, x, curr_room[y][x].ch);
        }
    }
}

void drawMinimap(void)
{
    werase(win_minimapa);
    box(win_minimapa, 0, 0);
    mvwprintw(win_minimapa, 0, 4, " MINIMAP ");

    for (int y = 0; y < MAP_SIZE; ++y)
    {
        for (int x = 0; x < MAP_SIZE; ++x)
        {
            int draw_y = 2 + y;
            int draw_x = 3 + x * 4;

            if (y == map_y && x == map_x)
            {
                mvwprintw(win_minimapa, draw_y, draw_x, "[@]");
            }
            else if (map[y][x].visited)
            {
                mvwprintw(win_minimapa, draw_y, draw_x, "[X]");
            }
            else if (map[y][x].discovered)
            {
                mvwprintw(win_minimapa, draw_y, draw_x, "[?]");
            }
            else
            {
                mvwprintw(win_minimapa, draw_y, draw_x, "   ");
            }
        }
    }
    wrefresh(win_minimapa);
}

void drawEntity(Entity* e) { mvwprintw(win, e->pos.y, e->pos.x, "%c", e->ch); }

void drawEverything(void)
{
    drawRoom();
    drawEntity(player);
    wrefresh(win);
    drawMinimap();
}
