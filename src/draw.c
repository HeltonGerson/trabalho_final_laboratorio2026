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
    if (!win || !curr_room)
        return;

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
}

void drawEntity(Entity* e)
{
    if (!win || !e || !e->active)
        return;

    mvwaddch(win, e->pos.y, e->pos.x, e->ch);
}

void drawRoomEntities(void)
{
    if (!win || !map)
        return;

    MapCell* cell = &map[map_y][map_x];
    for (int i = 0; i < cell->entity_count; ++i)
    {
        drawEntity(&cell->entities[i]);
    }
}

void drawHud(void)
{
    if (!win_hud || !player)
        return;

    int filled = (player->max_hp > 0) ? (player->hp * 8) / player->max_hp : 0;
    if (player->hp > 0 && filled == 0)
        filled = 1;
    if (filled < 0)
        filled = 0;
    if (filled > 8)
        filled = 8;

    char bar[9] = "--------";
    for (int i = 0; i < filled; ++i)
        bar[i] = '#';

    char hp_text[16];
    int  len  = snprintf(hp_text, sizeof(hp_text), "%d / %d", player->hp, player->max_hp);
    int  hp_x = 11 + (10 - len) / 2;
    if (hp_x < 1)
        hp_x = 1;

    werase(win_hud);
    box(win_hud, 0, 0);
    mvwprintw(win_hud, 0, 3, " STATUS DO JOGADOR ");
    mvwprintw(win_hud, 2, 3, "♥ VIDA: [%s]", bar);
    mvwprintw(win_hud, 3, hp_x, "%s", hp_text);
    mvwprintw(win_hud, 6, 3, "⚔ ATAQUE: %d", player->attack);
    mvwprintw(win_hud, 8, 3, "⛨ DEFESA: %d", player->defense);
}

void drawEverything(void)
{
    werase(stdscr);
    wnoutrefresh(stdscr);

    if (COLS < 77 || LINES < ROOM_HEIGHT || !win || !win_minimapa || !win_hud)
    {
        if (LINES > 0 && COLS > 0)
        {
            const char* msg = "Terminal pequeno! Min: 77x11";
            int         y   = LINES / 2;
            int         x   = (COLS - (int)strlen(msg)) / 2;
            if (x < 0)
                x = 0;
            mvaddnstr(y, x, msg, COLS - x);
            wnoutrefresh(stdscr);
        }
        doupdate();
        return;
    }

    drawRoom();
    drawRoomEntities();
    drawEntity(player);
    drawMinimap();
    drawHud();

    touchwin(win);
    touchwin(win_minimapa);
    touchwin(win_hud);
    wnoutrefresh(win);
    wnoutrefresh(win_minimapa);
    wnoutrefresh(win_hud);
    doupdate();
}
