#include "rogue.h"

static char roomGlyph(int id)
{
    if (id == -1)
        return 'S';
    if (id == 1)
        return 'B';
    if (id == 2 || id == 3)
        return 'I';
    return 'X';
}

// desenha a room atual em escala 1:1
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
    if (!win_minimapa || !map)
        return;

    werase(win_minimapa);
    box(win_minimapa, 0, 0);
    mvwprintw(win_minimapa, 0, 3, " MINIMAP ");
    mvwprintw(win_minimapa, 6, 4, " (%d,%d) ", map_x, map_y);

    for (int dy = -1; dy <= 1; ++dy)
    {
        for (int dx = -1; dx <= 1; ++dx)
        {
            int ny     = map_y + dy;
            int nx     = map_x + dx;
            int draw_y = 1 + (dy + 1) * 2;
            int draw_x = 2 + (dx + 1) * 4;

            if (ny < 0 || ny >= MAP_HEIGHT || nx < 0 || nx >= MAP_WIDTH)
            {
                mvwprintw(win_minimapa, draw_y, draw_x, "###");
                continue;
            }

            if (ny == map_y && nx == map_x)
                mvwprintw(win_minimapa, draw_y, draw_x, "[@]");
            else if (map[ny][nx].visited)
                mvwprintw(win_minimapa, draw_y, draw_x, "[%c]", roomGlyph(map[ny][nx].id));
            else if (map[ny][nx].discovered)
                mvwprintw(win_minimapa, draw_y, draw_x, "[?]");
            else
                mvwprintw(win_minimapa, draw_y, draw_x, "   ");

            if (dx < 1 && nx + 1 < MAP_WIDTH && map[ny][nx].id != 0 && map[ny][nx + 1].id != 0 &&
                (map[ny][nx].visited || map[ny][nx + 1].visited))
            {
                mvwaddch(win_minimapa, draw_y, draw_x + 3, '-');
            }

            if (dy < 1 && ny + 1 < MAP_HEIGHT && map[ny][nx].id != 0 && map[ny + 1][nx].id != 0 &&
                (map[ny][nx].visited || map[ny + 1][nx].visited))
            {
                mvwaddch(win_minimapa, draw_y + 1, draw_x + 1, '|');
            }
        }
    }
    wrefresh(win_minimapa);
}

void drawEntity(Entity* e) { mvwaddch(win, e->pos.y, e->pos.x, e->ch); }

void drawEverything(void)
{
    if (COLS < 50 || LINES < ROOM_HEIGHT || !win || !win_minimapa)
    {
        const char* msg = "Terminal pequeno! Min: 50x10";
        int         y   = LINES / 2;
        int         x   = (COLS - (int)strlen(msg)) / 2;
        if (x < 0)
            x = 0;
        erase();
        mvprintw(y, x, "%s", msg);
        refresh();
        return;
    }

    drawRoom();
    drawEntity(player);
    wrefresh(win);
    drawMinimap();
}
