#include "rogue.h"
#include <curses.h>
#include <stdio.h>
#include <stdlib.h>

void cursesSetup()
{
    initscr();            /* Inicia o modo curses */
    noecho();             /* Faz com que eh digitado nao apareca no term */
    cbreak();             /* Comandos nao sao executados (ex: Ctrl-C) */
    keypad(stdscr, TRUE); /* Permite pegar caracteres como as setas e teclas F(n) */
    curs_set(0);          /* Cursor nao aparece */
}

bool saveExists(void) // acho que não precisa
{
    FILE* f = fopen(SAVE_PATH, "r");
    if (!f)
        return false;
    fclose(f);
    return true;
}

void save(Entity *player)
{
    FILE *save = fopen("data/save/save.txt", "w");
    if(!save){
        return;
    }
    int somaEntidades = 0;
    for(int i = 0; i < MAP_WIDTH; i++)
    {
        for(int j = 0; j < MAP_HEIGHT; j++)
        {
            somaEntidades += map[j][i].entity_count;
        }
    }
    fprintf(save, "%i", somaEntidades);
    fprintf(save, "\n");
    
    for(int i = 0; i < MAP_WIDTH; i++)
    {
        for(int j = 0; j < MAP_HEIGHT; j++)
        {
            fprintf(save, "%i %i \n", map[j][i].id, map[j][i].entity_count);
            for(int t = 0; t < map[j][i].entity_count; t++)
            {
                fprintf(save, "%i %c %i %i %i %i %i %i %i \n", map[j][i].entities[t].active,  map[j][i].entities[t].ch, map[j][i].entities[t].type, map[j][i].entities[t].attack,  map[j][i].entities[t].defense,  map[j][i].entities[t].hp,  map[j][i].entities[t].max_hp,  map[j][i].entities[t].pos.x,  map[j][i].entities[t].pos.y);
            }
        }
    }
    fprintf(save, "%i %i %i %i %i %i %i %i\n", player->hp, player->max_hp, player->attack, player->defense, player->pos.x, player->pos.y, map_y, map_x);
    fclose(save);
}

void continueGame()
{
    if(!saveExists()) 
        return;
    
    FILE *save = fopen(SAVE_PATH, "r");
    int countRoom = 0, countEntity = 0, totalEntities = 0, idx = 0;
    fscanf(save, "%i", &totalEntities);
    for(int i = 0; i < MAP_WIDTH; i++)
    {
        for(int j = 0; j < MAP_HEIGHT; j++)
        {
            fscanf(save, "%i", &countRoom);
            fscanf(save, "%i", &countEntity);
            map[j][i].id = countRoom;
            map[j][i].entity_count = countEntity;
            map[j][i].tiles = readRoom(countRoom);
            if(countRoom != 0 && countRoom != -1)
            { 
                Entity *auxEntity = malloc(sizeof(Entity));
                map[j][i].entities = malloc(countEntity * sizeof(Entity));
                for(int t = 0; t < countEntity; t++)
                {
                    int auxB, auxT;
                    char auxC;
                    fscanf(save, "%i", &auxB);
                    auxEntity->active = auxB;
                    fscanf(save, " %c", &auxC);
                    auxEntity->ch = (chtype)auxC;
                    fscanf(save, "%i", &auxT);
                    auxEntity->type = (EntityType)auxT;
                    fscanf(save, "%i %i %i %i %i %i", &auxEntity->attack, &auxEntity->defense, &auxEntity->hp, &auxEntity->max_hp, &auxEntity->pos.x, &auxEntity->pos.y);
                    map[j][i].entities[idx] = *auxEntity;
                    idx++;
                }
                idx = 0;
            }
        }
    }
    player = malloc(sizeof(Entity));
    player->active = 1;
    fscanf(save, "%i %i %i %i %i %i %i %i\n", &player->hp, &player->max_hp, &player->attack, &player->defense, &player->pos.x, &player->pos.y, &map_y, &map_x);
    fclose(save);
}

MenuOption showMainMenu(void)
{
    const char* labels[3];
    MenuOption  opts[3];
    int         count    = 0;
    int         selected = 0;

    labels[count] = "Start New Game";
    opts[count++] = MENU_NEW_GAME;
    if (saveExists())
    {
        labels[count] = "Continue";
        opts[count++] = MENU_CONTINUE;
    }
    labels[count] = "Quit";
    opts[count++] = MENU_QUIT;

    while (1)
    {
        werase(stdscr);
        int start_y = (LINES - (count + 3)) / 2;
        if (start_y < 0)
            start_y = 0;

        const char* title = "===  DUNGEON C ===";
        int         tx    = (COLS - (int)strlen(title)) / 2;
        if (tx < 0)
            tx = 0;
        mvprintw(start_y, tx, "%s", title);

        for (int i = 0; i < count; ++i)
        {
            int y = start_y + 2 + i;
            if (y >= LINES)
                break;
            int x = (COLS - (int)strlen(labels[i]) - 5) / 2;
            if (x < 0)
                x = 0;
            if (i == selected)
                attron(A_REVERSE);
            mvprintw(y, x, " %s %s  ", (i == selected) ? ">" : " ", labels[i]);
            if (i == selected)
                attroff(A_REVERSE);
        }
        refresh();

        int ch = getch();
        if (ch == KEY_UP || ch == 'w' || ch == 'k')
            selected = (selected - 1 + count) % count;
        else if (ch == KEY_DOWN || ch == 's' || ch == 'j')
            selected = (selected + 1) % count;
        else if (ch == '\n' || ch == '\r' || ch == KEY_ENTER){
            return opts[selected];
        }
        else if (ch == 'q')
            return MENU_QUIT;
    }
}

void gameLoop()
{
    int ch;

    drawEverything();

    while ((ch = getch()) != 'q')
    {
        if (ch == KEY_RESIZE)
            layoutWindows();
        else if (win && win_minimapa)
            handleInput(ch);

        drawEverything();
    }
}

/* Termina o jogo, libera memoria */
void closeGame(void)
{
    destroyWindows();
    endwin();

    free(player);
    player = NULL;

    freeMap();
}
