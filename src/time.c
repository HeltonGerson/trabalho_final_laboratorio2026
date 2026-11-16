#include "rogue.h"
/*
 * TEMPO:
 * 1: Ver quais acoes passam o tempo, Acoes do player validas
 * 2: Fazer um vetor / fila com todas as entidades.
 * 3: Cada entidade vai ter sua acao
 * 3.1: Os inimigos vao perseguir o player
 * 3.2(depois): Os projeteis seguem o seu percurso
 *
*/

void passTime()
{

    MapCell* cell = &map[map_y][map_x];
    Entity* entities = cell->entities;
    int ent_count = cell->entity_count;
    for (int i = 0; i < ent_count; i++)
    {
        Entity e = entities[i];
        if (e.type == ENTITY_ENEMY_MELEE)
            chasePlayer(&e);
    }
}
