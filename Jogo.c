// Projeto 3 de Arquitetura de Computadores
// Jogo para fugir de objetos (C)
// Manuel Joaquim Andrade Sousa Perez, 2029015
// Cl�udio Ascenso Sardinha, 2030215

/*	Livrarias	*/
#include <reg51.h>
#include <stdio.h>
#include <stdlib.h>
/**************/

/*	Portas	*/
sbit Input = P3;
#define DisplayX P2
#define DisplayY P1
/************/

/* Constantes auxiliares (microcontrolador) */
#define TEMPO_T0_HIGH	0x3C	 								// Byte mais significativo do timer 0 - 50 ms (12MHz)
#define TEMPO_T0_LOW	0xAF	 									// Byte menos significativo do timer 0 - 50 ms (12MHz)
#define TEMPO_T1 0x70	   									// 0x70 Tempo do timer 1 - 112 us (12MHz)
#define VAZIO 0xFF												// Porta vazia (tudo desligado)
#define INTERRUPCOES_SETUP 143		// Estado das interrup��es ao ser corrido o programa
/********************************************/

/*	Constantes auxiliares (jogo)	*/
#define IMAGEM_GAMEOVER 5						// ID da imagem do gameover
#define IMAGEM_VICTORY 6							// ID da imagem da vit�ria
#define NR_IMAGENS 7					// N�mero de imagens
#define NR_LINHAS	7						// N�mero total de linhas
#define POS_JOGADOR 6					// Coordenada Y do jogador (�ltima linha)
#define POS_JOGADOR_INICIAL 4					// Posi��o inicial do jogador (no meio da linha)
#define VIDAS_INICIAL 4				// N� de vidas para o jogador (3 vidas)
#define DIFICULDADE1 5				// Velocidade a que descem os obst�culos (dificuldade)
#define DIFICULDADE2 4				// Velocidade a que descem os obst�culos (dificuldade)
#define DIFICULDADE3 3 			// Velocidade a que descem os obst�culos (dificuldade)
#define DIFICULDADE4 2 			// Velocidade a que descem os obst�culos (dificuldade)
#define DIFICULDADE5 1  			// Velocidade a que descem os obst�culos (dificuldade)
#define LIMITE_X_DIREITA 1		// Limite direito do display em rela��o ao X
#define LIMITE_X_ESQUERDA 16	// Limite esquerdo do display em rela��o ao X
#define LINHA_VAZIA 0					// Representa uma linha vazia (imagem com os LEDs todos desligados)
/**********************************/

/*	Vari�veis do jogo	*/
char LinhaAtual;						// Vari�vel para guardar o n�mero de linha
char ImagemDisplay[NR_LINHAS];		// Display com 7 valores

char VidasRestantes;				// Vidas restantes do jogador
char TempoObstaculos; 			// Tempo (restante) antes de descerem (novamente) os obst�culos
char DificuldadeAtual;			// Dificuldade atual do jogo (tempo estipulado para a frequ�ncia com que os obst�culos v�o descendo)
char ObstaculosInicio;			// Limite superior dos obst�culos
char NivelAtual = 1;				// N�vel atual do jogo (que representa a imagem do vetor a ser desenhada)
/**********************/

/*	Conjunto de imagens do jogo	*/
static char Imagens[NR_IMAGENS][NR_LINHAS - 1] = 
{{3, 2, 1, 6, 0, 0},		// N�vel 1
{4, 3, 2, 6, 0, 0},		// N�vel 2
{7, 6, 2, 6, 0, 0},		// N�vel 3
{8, 2, 3, 6, 0, 0},		// N�vel 4
{1, 4, 4, 6, 0, 0},		// N�vel 5
{17,10,4,10,17,0},      // Game over
{31,31,31,31,31,31}};	// Vit�ria
/********************************/

/*	Vetor para ativar as linhas do display	*/
static char ImagemLinha[NR_LINHAS] = 
{254, 253, 251, 247, 239, 223, 191};
/********************************************/

/* Fun��o para desenhar uma nova imagem no ecr�	*/
void desenharNovaImagem(char NovaImagem)
{
   int i;				
   for (i = 0; i < POS_JOGADOR; i++)
      ImagemDisplay[i] = Imagens[NovaImagem][i];		//Atualiza a linha com a nova imagem
}
/************************************************/

/* Fun��o para mover todos os obst�culos (uma "casa" para baixo)	*/
void moverObstaculos()
{
	int i;
	for (i = POS_JOGADOR - 1; i > ObstaculosInicio ; i--)			//Percorre todas as linhas
			ImagemDisplay[i] = ImagemDisplay[i-1]; 							//Puxa a imagem da linha (acima) para baixo
	if(ObstaculosInicio < NR_LINHAS - 1)				//Se os obst�culos ainda n�o chegaram at� ao fim
	{
		ImagemDisplay[ObstaculosInicio] = LINHA_VAZIA;	//Apaga a "primeira" linha dos obst�culos
		ObstaculosInicio++;												//Incrementa a posi��o onde "come�am" os obst�culos
	}
}
/******************************************************************/

/*	Fun��o para verificar se h� colis�o entre o jogador e um obst�culo	*/
void verificarColisoes(void)
{
	if(ImagemDisplay[POS_JOGADOR] & ImagemDisplay[POS_JOGADOR - 1]) //Se houver "interse��o" (colis�o)
		VidasRestantes--;																									 //Decrementa a quantidade de vidas restantes
}
/************************************************************************/

/* Fun��o para mover o jogador para a esquerda (quando � premido o devido bot�o)	*/
void moverJogadorEsquerda() interrupt 0
{
	if(ImagemDisplay[POS_JOGADOR] != LIMITE_X_ESQUERDA)	//Se n�o tiver encostado na borda esquerda
		ImagemDisplay[POS_JOGADOR] *= 2; 									//O jogador anda para a esquerda (shift left)
}
/********************************************************************6**************/

/* Fun��o para mover o jogador para a direita (quando � premido o devido bot�o)	*/
void moverJogadorDireita() interrupt 2
{
	if(ImagemDisplay[POS_JOGADOR] != LIMITE_X_DIREITA)	//Se n�o tiver encostado na borda direita
		ImagemDisplay[POS_JOGADOR] /= 2;									//O jogador anda para a direita (shift right)
}
/********************************************************************************/

/*	Fun��o de controlo para os obst�culos	(que � feita a cada segundo)	*/
/*	(verificar se h� colis�o e mov�-los para baixo)											*/
void verificarObstaculos() interrupt 1
{ 
	 // Reinicializa o timer
   TH0 = TEMPO_T0_HIGH;			
   TL0 = TEMPO_T0_LOW;
	
   TempoObstaculos--;						// Decrementa a contagem (de 1 segundo)
	
	 // Caso a contagem tenha chegado ao fim,
	 // Passa �s verifica��es e a��es dos obst�culos
   if (TempoObstaculos == 0)		
   {
		 	verificarColisoes();							// Verifica se h� colis�o entre o jogador e um obst�culo
			TempoObstaculos = DificuldadeAtual;		// Reinicializa a contagem
			moverObstaculos();										// Move os obst�culos para baixo
   }
}
/************************************************************************/

/*	Fun��o para varrer o display	*/
void varrerDisplay(void) interrupt 3
{
   DisplayY = VAZIO;								// Desliga todas as linhas de sa?da
   DisplayX = ImagemDisplay[LinhaAtual];	// Mostra a linha
   DisplayY = ImagemLinha[LinhaAtual];	// Activa a linha
   LinhaAtual++;										// Passa para a pr�xima linha
	
   if (LinhaAtual == NR_LINHAS) 		// Caso tenha chegado � ultima linha,
		LinhaAtual = 0;									// Passa novamente para a primeira linha
}
/**********************************/

/*	Fun��o para desenhar os obst�culos pela primeira vez	*/
/*	(consoante a imagem (dos obst�culos) do devido n�vel)	*/
void inicializarObstaculos()
{
	 ObstaculosInicio = 0;						// A "primeira" linha dos obst�culos come�a no topo do display
     desenharNovaImagem(NivelAtual - 1);						// Desenha a imagem do n�vel atual
}
/**********************************************************/

/*	Fun��o para desenhar pela primeira vez o jogador (na sua posi��o inicial)	*/
void inicializarJogador()
{
	 ImagemDisplay[POS_JOGADOR] = POS_JOGADOR_INICIAL;	//O jogador fica na sua posi��o inicial
}
/******************************************************************************/

/*	Fun��o para ligar as interrup��es (com o estado adequado)	*/
void ligarInterrupcoes(void)
{
   TMOD = 33;													// Timer 0 de 16 bits - #00100001b
   TH0 = TEMPO_T0_HIGH;									// Timer 0 = 50 ms
   TL0 = TEMPO_T0_LOW; 
   TH1 = TEMPO_T1;										// Timer 1 = 112 us
   TL1 = TEMPO_T1; 
   IP = 0;														// N�o altera as prioridades
   IE = INTERRUPCOES_SETUP;	// Activa as interrup��es - #10001111b:
   IT0 = 1;														// Ext0 detectada na transi��o descendente
   IT1 = 1;														// Ext1 detectada na transi��o descendente
   TR0 = 1;														// Inicia timer 0
   TR1 = 1;														// Inicia timer 1
   Input = VAZIO;											// Porta de entrada � limpa
}
/**************************************************************/

/*	Fun��o para desligar as interrup��es */
void desligarInterrupcoes(void)
{
	TR0 = 0;									// Desliga o timer do verificarObst�culos (que traz os obst�culos para baixo)
}
/*****************************************/

/* Fun��o para mostrar a imagem de GameOver (quando o jogador perde todas as vidas num n�vel)	*/
void gameOver()
{
	desenharNovaImagem(IMAGEM_GAMEOVER);			//Desenha essa imagem
	desligarInterrupcoes();
	for(;;);									//Fica em loop
}
/**********************************************************************************************/

/* Fun��o para mostrar a imagem da vit�ria (quando o jogador passa com sucesso por todos os n�veis)	*/
void victory()
{
	desenharNovaImagem(IMAGEM_VICTORY);			//Desenha essa imagem
	desligarInterrupcoes();
	for(;;);									//Fica em loop
}
/****************************************************************************************************/

/* Fun��o para jogar um dado n�vel do jogo */
void jogar(double dificuldade)
{
	//Busca no argumento 'dificuldade' o tempo que demora os obst�culos a descer
	DificuldadeAtual = dificuldade;			
	TempoObstaculos = DificuldadeAtual;
	
	inicializarJogador();							//O jogador � inicializado no display
	inicializarObstaculos();					//Os obst�culos do n�vel s�o inicializados no display

	//Ciclo do jogo propriamente dito
	while(VidasRestantes && ObstaculosInicio < NR_LINHAS - 1) {}
	
	//Caso tenha acabado o n�vel com sucesso,
	//Passa para o pr�ximo n�vel
	if(VidasRestantes)
	{
		NivelAtual++;		
		return;
	}
	
	//Caso contr�rio,
	//Mostra a imagem de GameOver
	gameOver();
}
/*******************************************/

/********************************************/
/*						Fun��o principal							*/
/********************************************/
void main()
{
	LinhaAtual = 0;	     							// Indica que � para mostrar a primeira linha
	ligarInterrupcoes();			//Liga as interrup��es
	VidasRestantes = VIDAS_INICIAL;		//vidas restantes do jogador � inicializada
	
	jogar(DIFICULDADE1);			//N�vel 1 do jogo
	jogar(DIFICULDADE2);			//N�vel 2 do jogo
	jogar(DIFICULDADE3);			//N�vel 3 do jogo
	jogar(DIFICULDADE4);			//N�vel 4 do jogo
	jogar(DIFICULDADE5);			//N�vel 5 do jogo

	//Caso tenha passado por todos os n�veis sem perder as vidas todas
	//Mostra a imagem da vit�ria
	victory();
}
/********************************************/
/********************************************/