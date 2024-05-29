#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "screen.h" 
#include "keyboard.h"
#include "timer.h"

#define SNAKE_MAX_LENGTH 100 // Define o comprimento máximo da cobra
#define FOOD_SYMBOL '$' // Define o símbolo do alimento
#define SNAKE_SYMBOL '*' // Define o símbolo da cobra
#define HORIZONTAL_BORDER_SYMBOL '-' // Define o símbolo da borda horizontal
#define VERTICAL_BORDER_SYMBOL '|' // Define o símbolo da borda vertical
#define SAVE_FILE "snake_save.txt" // Nome do arquivo de salvamento

// Constantes para a área de jogo
#define MINX 0
#define MAXX 79
#define MINY 0
#define MAXY 23

// Estrutura para representar as coordenadas de uma posição no plano
typedef struct {
    int x;
    int y;
} Posicao;

// Enumeração para representar as direções possíveis da cobra
typedef enum {
    CIMA,
    BAIXO,
    ESQUERDA,
    DIREITA
} Direcao;

// Estrutura para representar a cobra
typedef struct {
    Posicao *corpo;
    int comprimento;
    Direcao direcao;
} Cobra;

// Variáveis globais
Cobra cobra;
Posicao posicaoComida;
int pontuacao = 0;
int intervaloTimer = 150; // Inicialmente 150ms

// Função para inicializar a cobra no início do jogo
void inicializarCobra() {
    // Aloca memória para o corpo da cobra
    cobra.corpo = (Posicao *)malloc(SNAKE_MAX_LENGTH * sizeof(Posicao));
    if (cobra.corpo == NULL) {
        printf("Erro ao alocar memória para a cobra.\n");
        exit(1);
    }

    // Inicializa a posição e o comprimento da cobra
    cobra.corpo[0].x = MAXX / 2;
    cobra.corpo[0].y = MAXY / 2;
    cobra.comprimento = 1;

    // Inicializa a direção da cobra
    cobra.direcao = DIREITA;
}

// Função para liberar a memória alocada para a cobra
void destruirCobra() {
    free(cobra.corpo);
}

// Função para mover a cobra na direção especificada
void moverCobra() {
    // Move o corpo da cobra
    for (int i = cobra.comprimento - 1; i > 0; i--) {
        cobra.corpo[i] = cobra.corpo[i - 1];
    }

    // Move a cabeça da cobra na direção atual
    switch (cobra.direcao) {
        case CIMA:
            cobra.corpo[0].y--;
            break;
        case BAIXO:
            cobra.corpo[0].y++;
            break;
        case ESQUERDA:
            cobra.corpo[0].x--;
            break;
        case DIREITA:
            cobra.corpo[0].x++;
            break;
    }
}

// Função para verificar se a cobra colidiu com a parede ou consigo mesma
int verificarColisao() {
    // Verifica colisão com a parede
    if (cobra.corpo[0].x <= MINX || cobra.corpo[0].x >= MAXX || cobra.corpo[0].y <= MINY || cobra.corpo[0].y >= MAXY) {
        return 1;
    }

    // Verifica colisão consigo mesma
    for (int i = 1; i < cobra.comprimento; i++) {
        if (cobra.corpo[0].x == cobra.corpo[i].x && cobra.corpo[0].y == cobra.corpo[i].y) {
            return 1;
        }
    }

    return 0;
}

// Função para gerar uma nova posição aleatória para o alimento
void gerarComida() {
    do {
        posicaoComida.x = (rand() % (MAXX - MINX - 1)) + MINX + 1; // Posição aleatória dentro da área de jogo
        posicaoComida.y = (rand() % (MAXY - MINY - 1)) + MINY + 1;
    } while (screenGetCharAt(posicaoComida.x, posicaoComida.y) != ' '); // Garante que o alimento não seja gerado em cima da cobra
}

// Função para verificar se a cobra comeu o alimento
int verificarComida() {
    if (cobra.corpo[0].x == posicaoComida.x && cobra.corpo[0].y == posicaoComida.y) {
        pontuacao++; // Aumenta a pontuação
        cobra.comprimento++; // Aumenta o comprimento da cobra
        gerarComida(); // Gera um novo alimento
        // Diminui o intervalo do timer para aumentar a velocidade
        if (intervaloTimer > 50) { // Define um limite mínimo para o intervalo
            intervaloTimer -= 10;
        }
        // Reinitialize the timer with the new interval
        timerDestroy();
        timerInit(intervaloTimer);
        return 1;
    }
    return 0;
}

// Função para desenhar a cobra na tela
void desenharCobra() {
    screenSetTextColor(COLOR_GREEN);
    for (int i = 0; i < cobra.comprimento; i++) {
        screenGotoxy(cobra.corpo[i].x, cobra.corpo[i].y);
        printf("%c", SNAKE_SYMBOL);
    }
    screenResetTextColor();
}

// Função para desenhar o alimento na tela
void desenharComida() {
    screenSetTextColor(COLOR_YELLOW);
    screenGotoxy(posicaoComida.x, posicaoComida.y);
    printf("%c", FOOD_SYMBOL);
    screenResetTextColor();
}

// Função para desenhar a borda da tela
void desenharBorda() {
    for (int x = MINX; x <= MAXX; x++) {
        screenGotoxy(x, MINY);
        printf("%c", HORIZONTAL_BORDER_SYMBOL);
        screenGotoxy(x, MAXY);
        printf("%c", HORIZONTAL_BORDER_SYMBOL);
    }
    for (int y = MINY + 1; y < MAXY; y++) {
        screenGotoxy(MINX, y);
        printf("%c", VERTICAL_BORDER_SYMBOL);
        screenGotoxy(MAXX, y);
        printf("%c", VERTICAL_BORDER_SYMBOL);
    }
}

// Função para desenhar a pontuação na tela
void desenharPontuacao() {
    screenGotoxy(MAXX - 20, MAXY + 1);
    printf("Dinheiro acumulado: %d", pontuacao);
}

// Função para salvar o estado do jogo em um arquivo
void salvarJogo() {
    FILE *file = fopen(SAVE_FILE, "w");
    if (file == NULL) {
        printf("Erro ao abrir o arquivo de salvamento.\n");
        return;
    }

    // Salva a posição da cobra, do alimento e a pontuação
    fprintf(file, "%d %d %d %d %d %d %d %d\n", cobra.comprimento, cobra.direcao, cobra.corpo[0].x, cobra.corpo[0].y, posicaoComida.x, posicaoComida.y, pontuacao, intervaloTimer);

    fclose(file);
}

// Função para carregar o estado do jogo de um arquivo
void carregarJogo() {
    FILE *file = fopen(SAVE_FILE, "r");
    if (file == NULL) {
        printf("Nenhum arquivo de salvamento encontrado.\n");
        return;
    }

    // Carrega o estado do jogo do arquivo
    fscanf(file, "%d %d %d %d %d %d %d %d", &cobra.comprimento, &cobra.direcao, &cobra.corpo[0].x, &cobra.corpo[0].y, &posicaoComida.x, &posicaoComida.y, &pontuacao, &intervaloTimer);

    fclose(file);
}

int main() {
    // Inicialização
    srand(time(NULL)); // Inicializa a semente para geração de números aleatórios
    screenInit(1);
    keyboardInit();
    timerInit(intervaloTimer);
    // Verifica se o jogo deve ser carregado a partir de um arquivo
    printf("Deseja carregar o jogo anterior? (s/n): ");
    char escolha;
    scanf(" %c", &escolha);
    if (escolha == 's' || escolha == 'S') {
        carregarJogo(); // Carrega o jogo anterior
    } else {
        inicializarCobra(); // Inicializa a cobra
        gerarComida(); // Gera o primeiro alimento
    }

    // Loop principal do jogo
    while (1) {
        // Verifica se uma tecla foi pressionada
        if (keyhit()) {
            int tecla = readch();
            switch (tecla) {
                case 'w':
                case 'W':
                    if (cobra.direcao != BAIXO) cobra.direcao = CIMA;
                    break;
                case 's':
                case 'S':
                    if (cobra.direcao != CIMA) cobra.direcao = BAIXO;
                    break;
                case 'a':
                case 'A':
                    if (cobra.direcao != DIREITA) cobra.direcao = ESQUERDA;
                    break;
                case 'd':
                case 'D':
                    if (cobra.direcao != ESQUERDA) cobra.direcao = DIREITA;
                    break;
                case 'q':
                case 'Q':
                    // Finaliza o jogo se a tecla 'q' for pressionada
                    salvarJogo(); // Salva o estado do jogo antes de sair
                    goto fim_jogo;
            }
        }

        // Apaga a cauda da cobra (última posição)
        screenGotoxy(cobra.corpo[cobra.comprimento - 1].x, cobra.corpo[cobra.comprimento - 1].y);
        printf(" ");

        // Atualiza a posição da cobra e verifica colisões
        moverCobra();
        if (verificarColisao()) break; // Se houver colisão, sai do loop

        // Verifica se a cobra comeu o alimento
        if (verificarComida()) desenharComida();

        // Desenha os elementos do jogo
        desenharBorda();
        desenharCobra();
        desenharComida();
        desenharPontuacao();
        screenUpdate();

        // Aguarda até que o timer expire
        while (!timerTimeOver()) {
            // Verifica se uma tecla foi pressionada
            if (keyhit()) {
                int tecla = readch();
                switch (tecla) {
                    case 'w':
                    case 'W':
                        if (cobra.direcao != BAIXO) cobra.direcao = CIMA;
                        break;
                    case 's':
                    case 'S':
                        if (cobra.direcao != CIMA) cobra.direcao = BAIXO;
                        break;
                    case 'a':
                    case 'A':
                        if (cobra.direcao != DIREITA) cobra.direcao = ESQUERDA;
                        break;
                    case 'd':
                    case 'D':
                        if (cobra.direcao != ESQUERDA) cobra.direcao = DIREITA;
                        break;
                    case 'q':
                    case 'Q':
                        // Finaliza o jogo se a tecla 'q' for pressionada
                        salvarJogo(); // Salva o estado do jogo antes de sair
                        goto fim_jogo;
                }
            }
        }
    }

    fim_jogo:
    // Finaliza o jogo
    destruirCobra(); // Libera a memória alocada para a cobra
    keyboardDestroy();
    screenDestroy();
    timerDestroy();

    return 0;
}
