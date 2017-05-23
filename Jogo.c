#include <reg51.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

sbit Input = P3;
sbit Esquerda = P3^2;
sbit Direita = P3^3;
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

char pxJogador;
char ImagemAtual;			// Vari?vel com o n?mero da imagem
char LinhaAtual;			// Vari?vel para guardar o n?mero de linha
char TempoAntesDeRedesenhar; 			// Vari?vel para medir 1 s (20x50 ms)
char ImagemX[NR_LINHAS];	// Display com 7 valores
char ObstaculosInicio;
char nivel = 1;

int vidasRestantes;		//vidas restantes do jogador
int tempoRestante;						//tempo restante para o n�vel acabar

static char Imagens[NR_IMAGENS][NR_LINHAS] = 
{{3, 2, 1, 6, 0, 0, 4},		// N�vel 1
{4, 3, 2, 6, 0, 0, 4},		// N�vel 2
{7, 6, 2, 6, 0, 0, 4},		// N�vel 3
{8, 2, 3, 6, 0, 0, 4},		// N�vel 4
{1, 4, 4, 6, 0, 0, 4},		// N�vel 5

{0, 0, 0, 0, 0, 0, 4},		// Imagem 2
{0, 0, 0, 0, 0, 0, 4},		// Imagem 3
{0, 0, 0, 0, 0, 0, 4},		// Imagem 1
{0, 0, 0, 0, 0, 0, 4},		// Imagem 2
{0, 0, 0, 0, 0, 0, 4}};		// Imagem 3
static char ImagemY[NR_LINHAS] = 
{254, 253, 251, 247, 239, 223, 191};

void redesenharEcra()
{
   int i;				
   for (i = 0; i < NR_LINHAS - 1; i++)
      ImagemX[i] = Imagens[ImagemAtual][i];
}
void moverObstaculos()
{
	int i;
	char obstaculo;
	for (i = NR_LINHAS - 2; i > 0 ; i--)
			ImagemX[i] = ImagemX[i-1];
	if(ObstaculosInicio < Y_MAX)
	{
		ImagemX[ObstaculosInicio] = 0;
		ObstaculosInicio++;
	}
}
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
void moverJogadorEsquerda() interrupt 0
{
	if(ImagemX[NR_LINHAS - 1] != 16)
	{
		ImagemX[NR_LINHAS - 1] << 1; // shift left
		pxJogador--;
	}
}

void moverJogadorDireita() interrupt 2
{
	if(ImagemX[NR_LINHAS - 1] != 1)
	{
		ImagemX[NR_LINHAS - 1] >> 1; // shift right
		pxJogador++;
	}
}

void verificarObstaculos() interrupt 1
{
   TH0 = TEMPO_HIGH;		// Timer 0 = 50 ms
   TL0 = TEMPO_LOW;		// Verifica se j? passou 1 segundo 
   TempoAntesDeRedesenhar--;			
   if (TempoAntesDeRedesenhar == 0)		// Verifica se o tempo terminou
   {
      TempoAntesDeRedesenhar = TEMPO_1SEGUNDO;		// Actualiza a vari?vel tempo
			moverObstaculos();
   }
	 tempoRestante--;
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
void inicializarDisplay()
{
	 ObstaculosInicio = 0;
	 ImagemAtual = nivel - 1;			// Imagem inicial
   redesenharEcra();			// Chama rotina para mostrar imagem no display
}

void inicializarJogador()
{
	ImagemX[NR_LINHAS - 1] = 4;
	pxJogador = 2;
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

void verificarVidasJogador(void)
{
	if(estaLigado(px,Y_MIN+1))
		vidasRestantes--;
}

void jogar(double dificuldade)
{
	inicializarJogador();
	inicializarDisplay();
	ligarInterrupcoes();
	
	vidasRestantes = VIDAS_INICIAL;		//vidas restantes do jogador � inicializada
	tempoRestante = TEMPO_NIVEL;

	//Ciclo do jogo propriamente dito
	while(vidasRestantes && tempoRestante)	//Enquanto que houver vidas restantes
	{
		//verificarVidasJogador();		//verifica se o jogador bate num obst�culo (decrementando uma vida nesse caso)
	}
	
	if(vidasRestantes)
	{
		nivel++;
		return;
	}
	
	while(1){}
	
}

void main(void)
{
	jogar(DIFICULDADE1);			//N�vel 1 do jogo
	jogar(DIFICULDADE2);			//N�vel 2 do jogo
	jogar(DIFICULDADE3);			//N�vel 3 do jogo
	jogar(DIFICULDADE4);			//N�vel 4 do jogo
	jogar(DIFICULDADE5);			//N�vel 5 do jogo

  for(;;);
}
/*******************************************************************************/