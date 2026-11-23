#include "rogue.h"
#include <locale.h>

void cursesSetup()
{
    setlocale(LC_ALL, "");
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
    FILE *save = fopen(SAVE_PATH, "w");
    if(!save){
        return;
    }
    int somaEntidades = 0;
    for(int i = 0; i < MAP_HEIGHT; i++)
    {
        for(int j = 0; j < MAP_WIDTH; j++)
        {
            somaEntidades += map[i][j].entity_count;
        }
    }
    fprintf(save, "%i", somaEntidades);
    fprintf(save, "\n");

    for(int i = 0; i < MAP_HEIGHT; i++)
    {
        for(int j = 0; j < MAP_WIDTH; j++)
        {
            fprintf(save, "%i %i %i \n", map[i][j].id, map[i][j].entity_count, (int)map[i][j].visited);
            for(int t = 0; t < map[i][j].entity_count; t++)
            {
                fprintf(save, "%i %c %i %i %i %i %i %i %i \n",
                    map[i][j].entities[t].active,
                    (char)map[i][j].entities[t].ch,
                    map[i][j].entities[t].type,
                    map[i][j].entities[t].attack,
                    map[i][j].entities[t].defense,
                    map[i][j].entities[t].hp,
                    map[i][j].entities[t].max_hp,
                    map[i][j].entities[t].pos.x,
                    map[i][j].entities[t].pos.y);
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
    if(!save)
        return;

    int countRoom = 0, countEntity = 0, auxVisited = 0, totalEntities = 0;
    fscanf(save, "%i", &totalEntities);
    for(int i = 0; i < MAP_HEIGHT; i++)
    {
        for(int j = 0; j < MAP_WIDTH; j++)
        {
            fscanf(save, "%i %i %i", &countRoom, &countEntity, &auxVisited);
            map[i][j].id = countRoom;
            map[i][j].entity_count = countEntity;
            map[i][j].visited = (auxVisited != 0);
            if(countRoom != 0)
            {
                map[i][j].tiles = readRoom(countRoom);
                if(map[i][j].tiles)
                    removeEntityChars(&map[i][j]);

                if(countEntity > 0){
                    Entity auxEntity;
                    map[i][j].entities = malloc(countEntity * sizeof(Entity));
                    for(int t = 0; t < countEntity; t++)
                    {
                        int auxB, auxT;
                        char auxC;
                        fscanf(save, "%i", &auxB);
                        auxEntity.active = auxB;
                        fscanf(save, " %c", &auxC);
                        auxEntity.ch = (chtype)auxC;
                        fscanf(save, "%i", &auxT);
                        auxEntity.type = (EntityType)auxT;
                        fscanf(save, "%i %i %i %i %i %i", &auxEntity.attack, &auxEntity.defense, &auxEntity.hp, &auxEntity.max_hp, &auxEntity.pos.x, &auxEntity.pos.y);
                        map[i][j].entities[t] = auxEntity;
                    } 
                }
            }
        }
    }
    player = malloc(sizeof(Entity));
    player->active = 1;
    player->ch = '@';
    player->type = ENTITY_PLAYER;
    fscanf(save, "%i %i %i %i %i %i %i %i\n", &player->hp, &player->max_hp, &player->attack, &player->defense, &player->pos.x, &player->pos.y, &map_y, &map_x);
    fclose(save);

    updateFogOfWar();
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
        else if (ch == 'q'){
            
        
            return MENU_QUIT;
        }
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
        else if (win && win_minimapa && win_hud)
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

    if (logs)
    {
        eraseLogQueue(logs);
        logs = NULL;
    }

    freeMap();
}
