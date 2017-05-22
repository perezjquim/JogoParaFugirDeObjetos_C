#include <reg51.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

sbit Input = P3;
#define DisplayX P2
#define DisplayY P1
#define NR_LINHAS	7		// N?mero total de linhas
#define TEMPO_HIGH	0x3c	 	// Byte mais significativo do timer 0 - 50 ms (12MHz)
#define TEMPO_LOW	0xaf	 	// Byte menos significativo do timer 0 - 50 ms (12MHz)
#define TEMPO_1SEGUNDO 20		// Tempo do timer 0 para medir 1 s (20x50 ms)
#define TEMPO_T1 0x70	   	//0x70 Tempo do timer 1 - 112 us (12MHz)
#define NR_IMAGENS 10		// N?mero de imagens
#define VAZIO 0xFF
#define INTERRUPCOES_ESTADO_INICIAL 143

#define X_MIN 0
#define X_MAX 4
#define Y_MIN 0
#define Y_MAX 6
#define X_INICIAL 2
#define Y_INICIAL 0
#define VIDAS_INICIAL 2
#define TEMPO_NIVEL 9
#define DIFICULDADE1 1500
#define DIFICULDADE2 1200
#define DIFICULDADE3 900
#define DIFICULDADE4 600
#define DIFICULDADE5 50


char ImagemAtual;			// Vari?vel com o n?mero da imagem
char LinhaAtual;			// Vari?vel para guardar o n?mero de linha
char TempoAntesDeRedesenhar; 			// Vari?vel para medir 1 s (20x50 ms)
char ImagemX[NR_LINHAS];	// Display com 7 valores

int vidasRestantes;		//vidas restantes do jogador
int tempoRestante;						//tempo restante para o nível acabar

static char Imagens[NR_IMAGENS][NR_LINHAS] = 
{{0, 1, 2, 3, 4, 5, 6},		// Imagem 0
{7, 8, 9, 10, 11, 12, 13},		// Imagem 1
{14, 15, 16, 17, 18, 19, 20},		// Imagem 2
{30, 1, 1, 30, 1, 1, 30},		// Imagem 3
{17, 17, 17, 15, 1, 1, 1},		// Imagem 4
{31, 16, 16, 14, 1, 1, 30},		// Imagem 5
{16, 16, 16, 30, 17, 17, 14},		//Imagem 6
{30, 1, 1, 2, 4, 8, 16},			// Imagem 7
{14, 17, 17, 14, 17, 17, 14},		// Imagem 8
{14, 17, 17, 15, 1, 1, 1}};		// Imagem 9
static char ImagemY[NR_LINHAS] = 
{254, 253, 251, 247, 239, 223, 191};


void redesenharEcra()
{
   int i;				
   for (i = 0; i < NR_LINHAS; i++)
      ImagemX[i] = Imagens[ImagemAtual][i];
}


void Trata_Timer0() interrupt 1
{
   TH0 = TEMPO_HIGH;		// Timer 0 = 50 ms
   TL0 = TEMPO_LOW;		// Verifica se j? passou 1 segundo 
   TempoAntesDeRedesenhar--;			
   if (TempoAntesDeRedesenhar == 0)		// Verifica se o tempo terminou
   {
      TempoAntesDeRedesenhar = TEMPO_1SEGUNDO;		// Actualiza a vari?vel tempo
			if (ImagemAtual == NR_IMAGENS - 1) 	
				 ImagemAtual = 0;
			else
				 ImagemAtual++;
			redesenharEcra();	// Mostra o vector
   }
}

void varrerDisplay(void) interrupt 3
{
   DisplayY = VAZIO;			// Desliga todas as linhas de sa?da
   DisplayX = ImagemX[LinhaAtual];	// Mostra a linha
   DisplayY = ImagemY[LinhaAtual];	// Activa a linha
   LinhaAtual++;			// Pr?xima linha
   if (LinhaAtual == NR_LINHAS) 	// Verifica se ? a ?ltima linha
		LinhaAtual = 0;		// Para a linha inicial
}
void inicializarDisplay(void)
{
	 ImagemAtual = 0;			// Imagem inicial
   redesenharEcra();			// Chama rotina para mostrar imagem no display
}

void ligarInterrupcoes(void)
{
	TMOD = 33;			// Timer 0 de 16 bits - #00100001b
   TH0 = TEMPO_HIGH;			// Timer 0 = 50 ms
   TL0 = TEMPO_LOW; 
   TH1 = TEMPO_T1;			// Timer 1 = 112 us
   TL1 = TEMPO_T1; 
   TempoAntesDeRedesenhar = TEMPO_1SEGUNDO;   		// Inicializa vari?vel para medir 1 s
   IP = 0;				// N?o altera as prioridades
   IE = INTERRUPCOES_ESTADO_INICIAL;			// Activa as interrup??es - #10001111b:
   IT0 = 1;			// Ext0 detectada na transi??o descendente
   IT1 = 1;			// Ext1 detectada na transi??o descendente
   TR0 = 1;			// Inicia timer 0
   TR1 = 1;			// Inicia timer 1
   LinhaAtual = 0;	     		// Indica que ? para mostrar a primeira linha
   Input = VAZIO;			// P3 ? uma porta de entrada
}

void jogar(double dificuldade)
{
	vidasRestantes = VIDAS_INICIAL;		//vidas restantes do jogador é inicializada
	tempoRestante = TEMPO_NIVEL;

	//inicializarJogador();	//o jogador é desenhado no DisplayY

	//Ciclo do jogo propriamente dito
	while(vidasRestantes && tempoRestante)	//Enquanto que houver vidas restantes
	{
		//verificarControlos();				//verifica se é clicado para ir para a esquerda ou para a direita (fazendo o respetivo movimento)
		//moverObstaculos();					//desenha novos objetos e arrasta os atuais para baixo
		//verificarVidasJogador();		//verifica se o jogador bate num obstáculo (decrementando uma vida nesse caso)
		//delay(dificuldade);
		//tempoRestante--;
	}
	
	if(vidasRestantes)
		return;
	
	//showGameOver();
	
}

void main(void)
{
	inicializarDisplay();
	ligarInterrupcoes();
	
	jogar(DIFICULDADE1);			//Nível 1 do jogo
	jogar(DIFICULDADE2);			//Nível 2 do jogo
	jogar(DIFICULDADE3);			//Nível 3 do jogo
	jogar(DIFICULDADE4);			//Nível 4 do jogo
	jogar(DIFICULDADE5);			//Nível 5 do jogo

  for(;;);
}
/*******************************************************************************/