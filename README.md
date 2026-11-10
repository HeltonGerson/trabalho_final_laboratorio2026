# Dungeon C — Exploração de Masmorras

> Projeto Final da disciplina de **Laboratório de Programação** (Ciência da Computação — 2026.2).

---

## 📌 Visão Geral

**Dungeon C** é um jogo roguelike de exploração de masmorras em terminal Linux baseado em caracteres ASCII, inspirado em clássicos como *Rogue*, *The Binding of Isaac* e *Stone Story RPG*.

O projeto utiliza **C99 puro** para toda a arquitetura e lógica de jogo (geração procedural, combate, inventário, entidades e estados), e a biblioteca **`ncurses`** para renderização em múltiplas janelas no terminal e captura responsiva de entradas de teclado.

---

## 👥 Equipe de Desenvolvimento

- **Helton**
- **Lucas Gabriel**
- **Francisco Rerisson**
- **Camilly**

---

## 🎮 Mecânicas e Funcionalidades

- **Geração Procedural de Masmorras:**
  - Algoritmo baseado em DFS aleatória com pilha de coordenadas (`coordStack`) e backtracking para gerar a malha de salas do andar.
  - Carregamento de templates de sala pré-construídos a partir de arquivos ASCII em `data/rooms/`.
  - Conexão e abertura de portas cardeais automáticas apenas entre salas vizinhas válidas.
- **Renderização Multi-Janelas com `ncurses`:**
  - **Minimapa (7x7):** Exibição do andar com névoa de guerra (*fog of war*), identificando a sala atual (`[@]`), salas visitadas/descobertas (`[X]`) e salas vizinhas inexploradas (`[?]`).
  - **Viewport da Sala Ativa:** Renderização detalhada da sala (10x32 caracteres), com paredes, cantos decorados em caracteres gráficos (ACS), chão e portas.
  - **HUD Lateral:** Painel de status do jogador (pontos de vida, dano, defesa, armas e chaves).
- **Movimentação e Navegação:**
  - Suporte completo a direcionais: `WASD`, setas direcionais ou teclas clássicas `HJKL`.
  - Transição contínua entre salas através das portas cardeais.
- **Sistema de Combate e Entidades:**
  - Inimigo Melee (Goblin corpo a corpo com movimentação em menor caminho).
  - Inimigo Ranged (Goblin Arqueiro com ataque à distância e em diagonais).
  - Itens de equipamento (espada, arco, armaduras) e consumíveis de cura/encantamento.
- **Persistência de Dados:**
  - *Save & Load Game*: Salvamento do estado completo do andar e do personagem em arquivo (`saves/savegame.dat`).
  - *Ranking*: Registro de pontuações e recordes de andares alcançados (`saves/ranking.txt`).

---

## 🏗️ Estrutura do Projeto

```text
.
├── makefile                 # Regras de compilação, testes e execução
├── README.md                # Documentação principal do projeto
├── compile_flags.txt        # Configuração para LSPs (clangd/ccls)
├── data/
│   └── rooms/               # Templates ASCII de salas (normal, boss, items)
├── docs/
│   ├── ideas.txt            # Planejamento do MVP e anotações de design
│   └── mockups/             # Rascunhos conceituais de telas
├── include/
│   └── rogue.h              # Declarações de structs, constantes e protótipos
├── src/
│   ├── main.c               # Ponto de entrada, loop de jogo e transições
│   ├── engine.c             # Inicialização e encerramento do ncurses e janelas
│   ├── draw.c               # Funções de renderização gráfica (sala, minimapa, HUD)
│   ├── map.c                # Geração procedural do andar (DFS) e fog of war
│   ├── room.c               # Leitura de templates e configuração de portas
│   └── structs.c            # Implementação de estruturas auxiliares (coordStack)
├── tests/
│   └── test_floor.c         # Suíte de testes automatizados de integração do mapa
└── saves/                   # Arquivos de persistência (savegames e rankings)
```

---

## ⚙️ Pré-requisitos e Dependências

- Sistema Operacional Linux (ou ambiente POSIX compatível).
- Compilador C: `gcc` ou `clang` com suporte ao padrão C99.
- Biblioteca `libncurses` (desenvolvimento):
  - Em distribuições baseadas em Debian/Ubuntu:
    ```bash
    sudo apt-get install libncurses5-dev libncursesw5-dev build-essential
    ```
  - Em distribuições baseadas em Fedora/RHEL:
    ```bash
    sudo dnf install ncurses-devel
    ```
  - Em distribuições Arch Linux:
    ```bash
    sudo pacman -S ncurses base-devel
    ```

---

## 🚀 Como Compilar e Executar

O projeto conta com um `makefile` configurado com flags rigorosas de compilação (`-Wall -Wextra -pedantic -std=c99 -g`):

1. **Compilar e rodar diretamente:**
   ```bash
   make
   ```
2. **Apenas compilar:**
   ```bash
   make compile
   ```
3. **Executar o binário gerado:**
   ```bash
   ./game
   # ou via make:
   make run
   ```
4. **Executar a suíte de testes:**
   ```bash
   make test
   ```
5. **Limpar arquivos compilados:**
   ```bash
   make clean
   ```

---

## 🕹️ Controles

| Ação | Teclas |
| :--- | :--- |
| **Mover para Cima** | `W`, `k` ou `Seta para Cima` |
| **Mover para Baixo** | `S`, `j` ou `Seta para Baixo` |
| **Mover para Esquerda** | `A`, `h` ou `Seta para Esquerda` |
| **Mover para Direita** | `D`, `l` ou `Seta para Direita` |
| **Sair do Jogo** | `q` ou `Q` |

---

## 📜 Requisitos Técnicos Acadêmicos Atendidos

1. **Padrão Rigoroso de Compilação:** Compilação limpa sem nenhum aviso (*warnings*) sob `-Wall -Wextra -pedantic -std=c99`.
2. **Modularização:** Divisão semântica de responsabilidades em múltiplos arquivos `.c` e cabeçalhos `.h`.
3. **Estruturas de Dados:** Uso extensivo de matrizes bidimensionais e `struct` para representação do mundo, salas, entidades e tiles.
4. **Alocação Dinâmica:** Gerenciamento explícito de memória (`malloc` / `free`) com verificação rigorosa de ponteiros nulos e funções de desalocação dedicadas.
5. **Prevenção e Robustez:** Validação de limites de matrizes (`map[y][x]`) para prevenir falhas de segmentação e *buffer overflows*.
6. **Testes Automatizados:** Módulo de testes unitários e de integração (`tests/test_floor.c`).
