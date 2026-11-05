#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MINIMAP_SIZE 7
#define ROOM_WIDTH 48
#define ROOM_HEIGHT 13
#define MAX_ITEMS 5
#define ITEM_NAME_LEN 32

/* Estados de cada sala no minimapa */
typedef enum {
  SALA_VAZIA,
  SALA_VISITADA,
  SALA_ATUAL,
  SALA_INEXPLORADA,
  SALA_CHEFE
} TipoSalaMinimapa;

/* Estrutura do Jogador */
typedef struct {
  int hp;
  int hp_max;
  int ataque;
  int defesa;
  char itens[MAX_ITEMS][ITEM_NAME_LEN];
  int qtd_itens;
} Jogador;

/* Estrutura da Sala com alocação dinâmica dos tiles */
typedef struct {
  int largura;
  int altura;
  char **tiles;
  bool porta_norte;
  bool porta_sul;
  bool porta_leste;
  bool porta_oeste;
} Sala;

/* Estrutura do Minimapa 7x7 */
typedef struct {
  TipoSalaMinimapa grid[MINIMAP_SIZE][MINIMAP_SIZE];
} Minimapa;

/* Cria a sala alocando dinamicamente a matriz de tiles */
Sala *criar_sala(int largura, int altura) {
  Sala *s = malloc(sizeof(Sala));
  if (!s)
    return NULL;

  s->largura = largura;
  s->altura = altura;
  s->porta_norte = true;
  s->porta_sul = true;
  s->porta_leste = true;
  s->porta_oeste = true;

  s->tiles = malloc(altura * sizeof(char *));
  if (!s->tiles) {
    free(s);
    return NULL;
  }

  for (int y = 0; y < altura; y++) {
    s->tiles[y] = malloc((largura + 1) * sizeof(char));
    if (!s->tiles[y]) {
      for (int k = 0; k < y; k++) {
        free(s->tiles[k]);
      }
      free(s->tiles);
      free(s);
      return NULL;
    }
    for (int x = 0; x < largura; x++) {
      s->tiles[y][x] = '.';
    }
    s->tiles[y][largura] = '\0';
  }

  return s;
}

/* Libera a memória alocada dinamicamente para a sala */
void liberar_sala(Sala *s) {
  if (!s)
    return;
  if (s->tiles) {
    for (int y = 0; y < s->altura; y++) {
      free(s->tiles[y]);
    }
    free(s->tiles);
  }
  free(s);
}

/* Converte o tipo de sala para o glifo correspondente: [X], [@], [?], [!], [ ]
 */
void obter_slot_minimapa(TipoSalaMinimapa tipo, char *buf) {
  switch (tipo) {
  case SALA_VISITADA:
    strcpy(buf, "[X]");
    break;
  case SALA_ATUAL:
    strcpy(buf, "[@]");
    break;
  case SALA_INEXPLORADA:
    strcpy(buf, "[?]");
    break;
  case SALA_CHEFE:
    strcpy(buf, "[!]");
    break;
  case SALA_VAZIA:
  default:
    strcpy(buf, "[ ]");
    break;
  }
}

/* Renderiza o HUD completo combinando as três colunas linha a linha */
void renderizar_tela(const Jogador *p, const Sala *s, const Minimapa *m) {
  char divisor[112];
  memset(divisor, '=', 111);
  divisor[111] = '\0';
  memcpy(divisor + 50, " DUNGEON C ", 11);

  /* Linha 1: Divisor superior */
  printf("%s\n", divisor);

  /* Linha 2: Título do Minimapa centralizado */
  printf("        MINIMAPA\n");

  /* Total de 15 linhas verticais (altura da sala: borda superior + 13 linhas +
   * borda inferior) */
  for (int l = 0; l < 15; l++) {
    /* --- 1. COLUNA ESQUERDA: Minimapa 7x7 (25 colunas + 5 espaços = 30
     * colunas) --- */
    char col_esq[35];
    if (l == 0) {
      snprintf(col_esq, sizeof(col_esq), "+-----------------------+     ");
    } else if (l >= 1 && l <= MINIMAP_SIZE) {
      int row = l - 1;
      char linha_minimapa[26];
      int pos = 0;
      linha_minimapa[pos++] = '|';
      linha_minimapa[pos++] = ' ';
      for (int col = 0; col < MINIMAP_SIZE; col++) {
        char slot[4];
        obter_slot_minimapa(m->grid[row][col], slot);
        memcpy(linha_minimapa + pos, slot, 3);
        pos += 3;
      }
      linha_minimapa[pos++] = ' ';
      linha_minimapa[pos++] = '|';
      linha_minimapa[pos] = '\0';
      snprintf(col_esq, sizeof(col_esq), "%s     ", linha_minimapa);
    } else if (l == MINIMAP_SIZE + 1) {
      snprintf(col_esq, sizeof(col_esq), "+-----------------------+     ");
    } else {
      /* Linhas 9 a 14: minimapa finalizado, preencher com 30 espaços */
      memset(col_esq, ' ', 30);
      col_esq[30] = '\0';
    }

    /* --- 2. COLUNA CENTRAL: Sala e Portas (50 colunas) --- */
    char col_meio[55];
    if (l == 0) {
      if (s->porta_norte) {
        snprintf(col_meio, sizeof(col_meio),
                 "+----------------------^^^^----------------------+");
      } else {
        snprintf(col_meio, sizeof(col_meio),
                 "+------------------------------------------------+");
      }
    } else if (l == 14) {
      if (s->porta_sul) {
        snprintf(col_meio, sizeof(col_meio),
                 "+----------------------vvvv----------------------+");
      } else {
        snprintf(col_meio, sizeof(col_meio),
                 "+------------------------------------------------+");
      }
    } else {
      int y = l - 1;
      char parede_esq = '|';
      char parede_dir = '|';

      /* Portas laterais ocupam as 3 linhas centrais da sala (linhas 5, 6 e 7)
       */
      if (y >= 5 && y <= 7) {
        if (s->porta_oeste)
          parede_esq = '<';
        if (s->porta_leste)
          parede_dir = '>';
      }
      snprintf(col_meio, sizeof(col_meio), "%c%s%c", parede_esq, s->tiles[y],
               parede_dir);
    }

    /* --- 3. COLUNA DIREITA: Informações do Jogador e Itens --- */
    char col_dir[64] = "";
    if (l == 2) {
      int barra_cheia = (p->hp_max > 0) ? (p->hp * 10 / p->hp_max) : 0;
      if (barra_cheia > 10)
        barra_cheia = 10;
      if (barra_cheia < 0)
        barra_cheia = 0;

      char barra[11];
      memset(barra, '#', barra_cheia);
      memset(barra + barra_cheia, ' ', 10 - barra_cheia);
      barra[10] = '\0';
      snprintf(col_dir, sizeof(col_dir), "      HP: [%s]", barra);
    } else if (l == 3) {
      snprintf(col_dir, sizeof(col_dir), "      ATAQUE: %d", p->ataque);
    } else if (l == 4) {
      snprintf(col_dir, sizeof(col_dir), "      DEFESA: %d", p->defesa);
    } else if (l == 5) {
      snprintf(col_dir, sizeof(col_dir), "      +----------------+");
    } else if (l == 6) {
      snprintf(col_dir, sizeof(col_dir), "      ITENS:");
    } else if (l >= 7 && l <= 9) {
      int idx = l - 7;
      if (idx < p->qtd_itens) {
        snprintf(col_dir, sizeof(col_dir), "      %d. %s", idx + 1,
                 p->itens[idx]);
      }
    }

    /* Imprime a linha completa concatenando as 3 colunas */
    printf("%s%s%s\n", col_esq, col_meio, col_dir);
  }

  /* Linha 18: Linha em branco */
  printf("\n");

  /* Linha 19: Divisor inferior */
  printf("%s\n", divisor);
}

int main(void) {
  Sala *sala = criar_sala(ROOM_WIDTH, ROOM_HEIGHT);
  if (!sala) {
    fprintf(stderr, "Erro ao alocar memoria para a sala.\n");
    return 1;
  }

  Minimapa minimapa;
  memset(&minimapa, 0, sizeof(Minimapa));

  /* Exemplo com salas exploradas na matriz 7x7 */
  minimapa.grid[1][2] = SALA_INEXPLORADA;
  minimapa.grid[2][1] = SALA_INEXPLORADA;
  minimapa.grid[2][2] = SALA_ATUAL;
  minimapa.grid[2][3] = SALA_INEXPLORADA;
  minimapa.grid[3][2] = SALA_INEXPLORADA;
  minimapa.grid[3][3] = SALA_VISITADA;
  minimapa.grid[4][3] = SALA_VISITADA;
  minimapa.grid[4][4] = SALA_CHEFE;

  Jogador jogador = {.hp = 100,
                     .hp_max = 100,
                     .ataque = 12,
                     .defesa = 5,
                     .qtd_itens = 3,
                     .itens = {"Espada longa", "Poção de vida", "Amuleto"}};

  renderizar_tela(&jogador, sala, &minimapa);

  liberar_sala(sala);
  return 0;
}
