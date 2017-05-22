#include <reg51.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define true 1
#define false 0
	
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

#define NR_OBSTACULOS	10 // Indice da ?ltima imagem
#define NR_LINHAS 7

#define TEMPO_HIGH	0x3C	 	// Byte mais significativo do timer 0 - 50 ms (12MHz)
#define TEMPO_LOW	0xAF	 	// Byte menos significativo do timer 0 - 50 ms (12MHz)
#define TEMPO_1SEGUNDO 20		// Tempo do timer 0 para medir 1 s (20x50 ms)
#define TEMPO_T1 0x70	   	//0x70 Tempo do timer 1 - 112 us (12MHz)
#define INTERRUPCOES_ESTADO_INICIAL 143
#define VAZIO 0xFF


char ImagemX[NR_LINHAS];
char ImagemY[NR_LINHAS];

void varrerDisplay(void) interrupt 1
{
   DisplayY = VAZIO;			// Desliga todas as linhas de sa?da
   DisplayX = ImagemX[LinhaAtual];	// Mostra a linha
   DisplayY = ImagemY[LinhaAtual];	// Activa a linha
   LinhaAtual++;			// Pr?xima linha
   if (LinhaAtual == NR_LINHAS) 	// Verifica se ? a ?ltima linha
   	 LinhaAtual = 0;		// Para a linha inicial
}


void ligarInterrupcoes(void)
{
   TMOD = 33;
   TH0 = TEMPO_HIGH;			
   TL0 = TEMPO_LOW; 
   TH1 = TEMPO_T1;			
   TL1 = TEMPO_T1; 
   VTempo = TEMPO_1SEGUNDO;   		
   IP = 0;				
   IE = INTERRUPCOES_ESTADO_INICIAL;
   IT0 = 1;	
   IT1 = 1;
   TR0 = 1;
   TR1 = 1;
   LinhaAtual = 0;	     		
   Input = VAZIO;			
}

void main(void)
{
	ligarInterrupcoes();
	
	//jogar(DIFICULDADE1);			//N�vel 1 do jogo
	//jogar(DIFICULDADE2);			//N�vel 2 do jogo
	//jogar(DIFICULDADE3);			//N�vel 3 do jogo
	//jogar(DIFICULDADE4);			//N�vel 4 do jogo
	//jogar(DIFICULDADE5);			//N�vel 5 do jogo
	
	return;
}

