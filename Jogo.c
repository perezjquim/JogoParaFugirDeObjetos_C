#include <reg51.h>
#include <stdio.h>
#include <stdlib.h>

sbit Input = P3;
sbit Esquerda = P3^2;
sbit Direita = P3^3;
#define DisplayX P2
#define DisplayY P1
#define NR_LINHAS	7		// N?mero total de linhas
#define TEMPO_HIGH	0x3c	 	// Byte mais significativo do timer 0 - 50 ms (12MHz)
#define TEMPO_LOW	0xaf	 	// Byte menos significativo do timer 0 - 50 ms (12MHz)
#define TEMPO_T1 0x70	   	//0x70 Tempo do timer 1 - 112 us (12MHz)
#define NR_IMAGENS 7		// N?mero de imagens
#define VAZIO 0xFF
#define INTERRUPCOES_ESTADO_INICIAL 143
#define GAMEOVER 5
#define VICTORY 6

#define POS_JOGADOR 6
#define POS_INICIAL 4
#define VIDAS_INICIAL 3
#define DIFICULDADE1 20
#define DIFICULDADE2 18
#define DIFICULDADE3 15
#define DIFICULDADE4 10
#define DIFICULDADE5 7
#define LIMITE_X_DIREITA 1
#define LIMITE_X_ESQUERDA 16
#define LINHA_VAZIA 0

char ImagemAtual;			// Vari?vel com o n?mero da imagem
char LinhaAtual;			// Vari?vel para guardar o n?mero de linha
char ImagemX[NR_LINHAS];	// Display com 7 valores

char VidasRestantes;		//vidas restantes do jogador
char TempoObstaculos; 			// Vari?vel para medir 1 s (20x50 ms)
char DificuldadeAtual;
char ObstaculosInicio;
char NivelAtual = 1;

static char Imagens[NR_IMAGENS][NR_LINHAS] = 
{{3, 2, 1, 6, 0, 0, 4},		// N�vel 1
{4, 3, 2, 6, 0, 0, 4},		// N�vel 2
{7, 6, 2, 6, 0, 0, 4},		// N�vel 3
{8, 2, 3, 6, 0, 0, 4},		// N�vel 4
{1, 4, 4, 6, 0, 0, 4},		// N�vel 5

{17,10,4,10,17,0,0},      // Game over
{31,31,31,31,31,31,31}};		// Vit�ria
static char ImagemY[NR_LINHAS] = 
{254, 253, 251, 247, 239, 223, 191};

void redesenharEcra()
{
   int i;				
   for (i = 0; i < POS_JOGADOR; i++)
      ImagemX[i] = Imagens[ImagemAtual][i];
}
void moverObstaculos()
{
	int i;
	for (i = POS_JOGADOR - 1; i > 0 ; i--)
			ImagemX[i] = ImagemX[i-1];
	if(ObstaculosInicio < NR_LINHAS - 1)
	{
		ImagemX[ObstaculosInicio] = LINHA_VAZIA;
		ObstaculosInicio++;
	}
}
void verificarVidasJogador(void)
{
	if((ImagemX[POS_JOGADOR] & ImagemX[POS_JOGADOR - 1]) != LINHA_VAZIA)
		VidasRestantes--;
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
void moverJogadorEsquerda() interrupt 0
{
	if(ImagemX[POS_JOGADOR] != LIMITE_X_ESQUERDA)
		ImagemX[POS_JOGADOR] *= 2; // shift left
}

void moverJogadorDireita() interrupt 2
{
	if(ImagemX[POS_JOGADOR] != LIMITE_X_DIREITA)
		ImagemX[POS_JOGADOR] /= 2; // shift right
}

void verificarObstaculos() interrupt 1
{ 
   TH0 = TEMPO_HIGH;		// Timer 0 = 50 ms
   TL0 = TEMPO_LOW;		// Verifica se j? passou 1 segundo 
   TempoObstaculos--;			
   if (TempoObstaculos == 0)		// Verifica se o tempo terminou
   {
		 	verificarVidasJogador();
      TempoObstaculos = DificuldadeAtual;		// Actualiza a vari?vel tempo
			moverObstaculos();
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
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
void inicializarObstaculos()
{
	 ObstaculosInicio = 0;
	 ImagemAtual = NivelAtual - 1;			// Imagem inicial
   redesenharEcra();			// Chama rotina para mostrar imagem no display
}

void inicializarJogador()
{
	 ImagemX[POS_JOGADOR] = POS_INICIAL;
}


void ligarInterrupcoes(void)
{
	 TMOD = 33;			// Timer 0 de 16 bits - #00100001b
   TH0 = TEMPO_HIGH;			// Timer 0 = 50 ms
   TL0 = TEMPO_LOW; 
   TH1 = TEMPO_T1;			// Timer 1 = 112 us
   TL1 = TEMPO_T1; 
   IP = 0;				// N?o altera as prioridades
   IE = INTERRUPCOES_ESTADO_INICIAL;			// Activa as interrup??es - #10001111b:
   IT0 = 1;			// Ext0 detectada na transi??o descendente
   IT1 = 1;			// Ext1 detectada na transi??o descendente
   TR0 = 1;			// Inicia timer 0
   TR1 = 1;			// Inicia timer 1
   LinhaAtual = 0;	     		// Indica que ? para mostrar a primeira linha
   Input = VAZIO;			// P3 ? uma porta de entrada
}
void gameOver()
{
	ImagemAtual = GAMEOVER;
	redesenharEcra();
	for(;;);
}
void victory()
{
	ImagemAtual = VICTORY;
	redesenharEcra();
	for(;;);
}

void jogar(double dificuldade)
{
	DificuldadeAtual = dificuldade;
	TempoObstaculos = DificuldadeAtual;
	
	inicializarJogador();
	inicializarObstaculos();
	
	VidasRestantes = VIDAS_INICIAL;		//vidas restantes do jogador � inicializada

	//Ciclo do jogo propriamente dito
	while(VidasRestantes && ObstaculosInicio < NR_LINHAS - 1) {}
	
	if(VidasRestantes)
	{
		NivelAtual++;
		return;
	}
	
	gameOver();
}

void main()
{
	ligarInterrupcoes();
	
	jogar(DIFICULDADE1);			//N�vel 1 do jogo
	jogar(DIFICULDADE2);			//N�vel 2 do jogo
	jogar(DIFICULDADE3);			//N�vel 3 do jogo
	jogar(DIFICULDADE4);			//N�vel 4 do jogo
	jogar(DIFICULDADE5);			//N�vel 5 do jogo

	victory();
}
/*******************************************************************************/