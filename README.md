# Dungeon C — Exploração de Masmorras

Projeto final da disciplina de **Laboratório de Programação** (Ciência da Computação — 2026.2).  
**Equipe:** Helton, Lucas Gabriel, Francisco Rerisson e Camilly.

---

## Visão Geral

**Dungeon C** é um roguelike por turnos em terminal Linux escrito em **C99 puro** com interface em **`ncurses`**. O jogador explora andares gerados proceduralmente, enfrenta inimigos com comportamentos distintos e coleta itens para fortalecer seus atributos.

---

## Guia Rápido: Como o Jogo Funciona

### 1. Fluxo da Partida
1. **Menu Inicial (`showMainMenu`):** permite iniciar um novo jogo (`Start New Game`), continuar (`Continue`, quando existir save) ou sair (`Quit`).
2. **Geração do Andar (`setupFloor`):** cria uma grade `7x7` e gera de **8 a 15 salas** conectadas usando busca em profundidade (DFS) com pilha (`coordStack`), garantindo a sala inicial, ao menos uma sala de itens e uma sala de Boss.
3. **Loop de Turnos (`gameLoop`):** cada movimento ou disparo do jogador executa um turno das entidades vivas na sala (`passTime`), redesenha as janelas e exibe os logs de combate.
4. **Game Over (`showGameOver`):** ao zerar o HP do jogador, a partida é encerrada e é possível voltar ao menu ou sair.

### 2. Interface no Terminal (mínimo `77x14`)
- **Minimapa (esquerda):** janela deslizante `3x3` com névoa de guerra (`[@]` atual, `[S]` início, `[B]` boss, `[I]` itens, `[X]` normal visitada, `[?]` adjacente descoberta, `###` borda do mapa).
- **Sala (centro):** área `31x11` em escala `1:1` carregada dos templates ASCII em `data/rooms/`, com portas nas bordas conectadas às salas vizinhas.
- **HUD (direita):** barra de vida (`HP / Max HP`), `ATAQUE` e `DEFESA` atuais do jogador.
- **Logs (rodapé):** mensagens de combate e eventos exibidas na penúltima linha do terminal.

### 3. Entidades, Combate e IA
- **`@` Jogador:** inicia com `10 HP`, `4 ATK` (corpo a corpo), `2 RANGED` (arco com alcance `10`, sem cooldown) e `0 DEF`.
  - **Corpo a corpo:** mover-se sobre um inimigo desfere um ataque; se ele morrer, o espaço fica livre no próximo passo.
  - **À distância:** as setas direcionais disparam uma flecha em linha reta (para em paredes `#` ou inimigos).
  - **Prioridade de ataque:** ao entrar na linha de ataque de um inimigo nesse turno, o jogador tem prioridade e não sofre o golpe imediato.
- **`E` Goblin Melee (`4 HP`, `3 ATK`, `0 DEF`):** persegue o jogador pelo menor caminho usando busca em largura (BFS) e ataca quando adjacente.
- **`A` Goblin Arqueiro (`8 HP`, `2 RANGED`, `0 DEF`):** permanece parado e atira nas 4 direções cardeais (alcance `7`, cooldown de `1` turno).
- **`B` Boss (`20 HP`, `5 ATK`, `3 RANGED`, `1 DEF`):** opera em 2 fases:
  - *HP > 50%:* persegue o jogador (BFS) e ataca corpo a corpo.
  - *HP <= 50%:* atira em 8 direções (cardeais e diagonais, cooldown `1`) e tenta se afastar durante a recarga (`evadePlayer`).
- **`I` Itens:** coletados ao caminhar sobre eles, concedendo cura, aumento de HP máximo, ataque ou defesa.

---

## Controles

| Ação | Teclas |
| :--- | :--- |
| **Mover / Atacar corpo a corpo** | `W`, `A`, `S`, `D` ou `k`, `h`, `j`, `l` |
| **Disparar arco (4 direções)** | Setas direcionais (`↑`, `↓`, `←`, `→`) |
| **Navegar nos menus** | `W`/`S`, `k`/`j` ou `↑`/`↓` + `Enter` |
| **Sair da partida / menu** | `q` |

---

## Estrutura do Projeto

```text
.
├── makefile             # Compilação, execução e testes (-Wall -Wextra -pedantic -std=c99)
├── include/rogue.h      # Structs, enums, constantes, globais e protótipos
├── data/rooms/          # Templates ASCII (start/, boss/, items/, normal/)
├── src/
│   ├── main.c           # Ponto de entrada e gerenciamento das janelas ncurses
│   ├── engine.c         # Setup do ncurses, menus, gameLoop e limpeza de memória
│   ├── draw.c           # Renderização da sala, minimapa, HUD, projéteis e logs
│   ├── map.c            # Geração procedural do andar (DFS) e névoa de guerra
│   ├── room.c           # Leitura de templates ASCII e controle de portas
│   ├── entity.c         # Entidades, itens, movimento, combate e IA (BFS/ranged/boss)
│   ├── time.c           # Loop de turno dos inimigos (passTime)
│   ├── log.c            # Exibição de mensagens no rodapé
│   └── structs.c        # Pilha (coordStack) e filas (coordQueue, logQueue)
└── tests/
    └── test_floor.c     # Suíte com 11 testes automatizados (mapa, combate, IA e morte)
```

---

## Compilação, Execução e Testes

**Dependências:** compilador C99 (`gcc`) e biblioteca `ncurses` com suporte a caracteres largos (`libncursesw5-dev` no Debian/Ubuntu, `ncurses-devel` no Fedora ou `ncurses` no Arch Linux).

```bash
make compile   # Compila o binário ./game sem warnings
make run       # Executa ./game
make           # Compila, executa e limpa os binários
make test      # Compila e executa os 11 testes automatizados
make clean     # Remove binários gerados (game e test_runner)
```
