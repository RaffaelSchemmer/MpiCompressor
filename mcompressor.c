/* Universidade Federal do Rio Grande do Sul UFRGS */
/* Compressor de arquivos paralelo e distribuído utilizando linguagem C e bibliotecas MPI e PThreads */
/* Disciplina : Introdução a Programação Paralela e Distribuída IPDPS 2013/II */
/* Professor : Dr. Nicolas Maillard*/
/* Aluno : Raffael Bottoli Schemmer */
/* Data Termino : 30/11/2013 */

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sched.h>
#include <time.h>

#include "mpi.h"
#include <pthread.h>
FILE *file,*fileo;

// Número de nodos do cluster (1 - N)
#define NODE 3  
// Número de elementos da tabela (1 - 59)
#define NETABLE 50    
// Tamanho da palavra de procura (3 - N)
#define TMELEM 8     
// Número de partes que o arquivo é dividido
#define PFILE 20

int myrank=0,size=0;
MPI_Status status;

int fsize=0,ftemp=0;
	
unsigned long t1=0,t2=0;
	
unsigned int fvet[NODE];

pthread_t *thread,*thread1;
pthread_mutex_t *tmutex,*tmutex1;

int changevalue=0;
int changevalue1=0;
struct Table
{
	int number;
	char *valor;
}Table;
	
struct Timing
{
	unsigned long tamInicialArquivo;
	unsigned long tamFinalArquivo;
	unsigned long tmpInicialLeitura;
	unsigned long tmpFinalLeitura;
	unsigned long tmpInicialTotalProcessamento;
	unsigned long tmpFinalTotalProcessamento;
	unsigned long tmpInicialTransmissaoFile;
	unsigned long tmpFinalTransmissaoFile;
	unsigned long tmpInicialPreProcessamento;
	unsigned long tmpFinalPreProcessamento;
	unsigned long tmpInicialTransmissaoLista;
	unsigned long tmpFinalTransmissaoLista;
	unsigned long tmpInicialProcessamentoTabela;
	unsigned long tmpFinalProcessamentoTabela;
	unsigned long tmpInicialProcessamentoArquivo;
	unsigned long tmpFinalProcessamentoArquivo;
	unsigned long tmpInicialTransmissaoMestre;
	unsigned long tmpFinalTransmissaoMestre;
	unsigned long tmpInicialEscrita;
	unsigned long tmpFinalEscrita;
}Timing;
	
struct Timing* tempos;
	
char *tfile;
char *ffile;
char palavra[TMELEM];
char *tabelatempchar;
int  *tabelatempint;

int  *ttable1int;
char *ttable1char;

struct Table* tfinal;
struct Table* tabela;
struct Table* tabelanodos;
struct Table* t;


static inline uint64_t RDTSC()
{
  unsigned int hi, lo;
  __asm__ volatile("rdtsc" : "=a" (lo), "=d" (hi));
  return ((uint64_t)hi << 32) | lo;
}

// Thread receive que recebe de todo mundo e cria tabelas locais
void* receive()
{
	int cont=0,cont1=0,cont2=0,i=0,j=0,tag=0;
	int cont5=0,cont6=0,cont7=0,cont8=0;
	int tmp=0,cont3=0,cont4=0,cont0=0;

	tabelanodos = malloc(sizeof(struct Table) * ((NETABLE*PFILE)*(NODE)));
		
	cont=0;
	while(cont < ((NETABLE*PFILE)*(NODE)))
	{
		tabelanodos[cont].valor = malloc(sizeof(char)*(TMELEM));
		tabelanodos[cont].valor[0] = '\0';
		tabelanodos[cont].number = 0;
		cont++;
	}
	
	MPI_Status status;
	
	int num = (NODE);
	int vet[num];
	int total = (NODE-1) * PFILE;
	int c1=0;
			
	cont1=0;
	cont2=0;
	cont3=0;
	
	while(cont1 < num) { vet[cont1] = 0; cont1++;}
	
	cont1=0;
	while(1)
	{
		if(cont2 == total)
		{ 
			// fprintf (stderr, "Passei aqui........ %d %d\n",cont2,total);
			
			/*
			cont8=0;
			
				printf("Tamanho %d\n",((NETABLE*PFILE)*(NODE)));
				while(cont8 < ((NETABLE*PFILE)*(NODE)))
				{
					printf("%d %d -- %s\n",myrank,tabelanodos[cont8].number,tabelanodos[cont8].valor);
					cont8++;
				}
			*/
			break; 
		}
		
		MPI_Recv(&cont1,1,MPI_INT,MPI_ANY_SOURCE, tag, MPI_COMM_WORLD, &status);
		if(cont1 == -1) // Tabelas temporárias
		{ 
			
			cont1=0;
			ttable1char = malloc(sizeof(char)*((NETABLE)*TMELEM));
			ttable1int = malloc(sizeof(int)*(NETABLE));
				
			cont3=0;
			ttable1char[0] = '\0';
			while(cont3 < ((NETABLE)))
			{
				ttable1int[cont3] = 0;
				cont3++;
			}
	
			MPI_Recv(ttable1char,(NETABLE*TMELEM),MPI_CHAR, MPI_ANY_SOURCE,MPI_ANY_TAG, MPI_COMM_WORLD,&status);
			MPI_Recv(ttable1int,NETABLE,MPI_INT, MPI_ANY_SOURCE,MPI_ANY_TAG, MPI_COMM_WORLD,&status);
			
			
			/*
			cont8=0;
			cont0=0;
			while(cont8 < NETABLE)
			{
				printf("R - %d - ",myrank);
				cont5=0;
				while(cont5 < TMELEM)
				{
					printf("%c",ttable1char[cont0]);
					cont5++;
					cont0++;
				}
				printf("- %d\n",ttable1int[cont8]);
				cont8++;
			}
			*/
			
			//  0 29 --  30 59 -- 60 89 -- 90 119           ||   120 149 --  150 179 -- 180 209 -- 210 119 
			//  240 269 --  270 299 -- 300 329 -- 330 359   ||   360 389 --  390 419 -- 420 449 -- 450 359
			 			
 			//                        BLOCO                                        DESLOCAMENTO
			
			// 2 partes de 3 (0)
			// 0 2 -- 3 5 || 6 8 -- 9 11
			//  
			
			int c2 = ((status.MPI_SOURCE * (NETABLE*PFILE)) + ((vet[status.MPI_SOURCE] * NETABLE) + (NETABLE - 1)));
			int c1 = ((status.MPI_SOURCE * (NETABLE*PFILE)) + ((vet[status.MPI_SOURCE] * NETABLE)));
			
			//printf("%d %d %d\n",myrank,c1,c2);
			
			// Aloca tabela recebida
			cont3=0;
			cont7=0;
			while(c1 <= c2)
			{
				tabelanodos[c1].number = ttable1int[cont3];
				
				cont8=0;
				while(cont8 < TMELEM)
				{ 
					tabelanodos[c1].valor[cont8] = ttable1char[cont7];
					cont8++;
					cont7++;
				}
				tabelanodos[c1].valor[cont8] = '\0';
				
				cont3++;
				c1++;
			}
			vet[status.MPI_SOURCE]++;
			cont2++;
		}
				
	}
	
	while(1)
	{
		pthread_mutex_lock(tmutex1);
		if(changevalue1 == 0)
		{
			changevalue1=2;
			pthread_mutex_unlock(tmutex1);
			break;
		}
		pthread_mutex_unlock(tmutex1);
	}
}

void* compress()
{
	int cont=0,cont1=0,cont2=0,i=0,j=0,tag=0;
	int cont5=0,cont6=0,cont7=0,cont8=0;
	int tmp=0,cont3=0,cont4=0,cont0=0;
	int cont9=0,cont10=0,cont11=0;
	struct Table* tsimb;
	struct Table* tabelanodos2;			
	
	//printf("Process %d : Size %d \n",myrank,ftemp);
	
	tempos[myrank].tmpInicialPreProcessamento= RDTSC();
	tabela = malloc(sizeof(struct Table) * ((NETABLE*PFILE)));
		
	// Inicia a tabela que irá ler os marcadores do arquivo (aloca memória) e zera contadores
	cont=0;
	while(cont < (NETABLE*PFILE))
	{
		tabela[cont].valor = malloc (sizeof(char)*(TMELEM));
		tabela[cont].valor[0] = '\0';
		tabela[cont].number = 0;
		cont++;
	}
	
	
	cont=0;
	tmp=0;
	cont3=0;
	cont4=1;
	cont7=0;
	while(cont < ftemp)
	{
		// Lê nova palavra
		cont1=0;
		cont2=cont;
		palavra[0] = '\0';
		while(cont1 < TMELEM)
		{
			palavra[cont1] = tfile[cont2];
			cont1++;
			cont2++;
		}
		
		// Dump para ver as palavras coletadas
		//cont1=0;
		//while(cont1 < TMELEM){
		//	printf("%c",palavra[cont1]);
		//	cont1++;
		//}
		//printf("\n");
		
		// Procura na tabela por valor, se achar incrementa
		int flag=0;
		for(cont2=cont3;cont2 < (NETABLE*cont4);cont2++)
		{
			if(strcmp(palavra,tabela[cont2].valor) == 0)
			{
				tabela[cont2].number++;
				flag=1;
				break;
			}
		}
		
		// Se não achar o valor, procura por espaço livre para alocar valor
		if(flag == 0)
		{
			for(cont2=cont3;cont2 < (NETABLE*cont4);cont2++)
			{
				if(tabela[cont2].number == 0)
				{
					tabela[cont2].number = 1;
					strcpy(tabela[cont2].valor,palavra);
					break;
				}
			}
		}
		// Se chegou chegou a hora de transmitir, cria uma lista temporária e transmite
		if(tmp+TMELEM >= ftemp/(PFILE))
		{
			
			
			tmp=NETABLE;
			while(1)
			{
				// Seção crítica
				pthread_mutex_lock(tmutex);
			
				if(changevalue == 0)
				{
					pthread_mutex_unlock(tmutex);
					tabelatempchar = malloc(sizeof(char) * ((NETABLE)*TMELEM));
					tabelatempint = malloc(sizeof(int) * (NETABLE));
					cont6 = cont3;
					cont5=0;
							
					while(cont5 < (NETABLE))
					{
						tabelatempchar[cont5] = '\0';
						tabelatempint[cont5] = 0;
						cont5++;
					}
					
					cont8=0;
					cont2=0;
					cont5=0;
					while(cont5 < NETABLE)
					{
						tabelatempint[cont5] = tabela[cont6].number;
						
						cont8=0;
						while(cont8 < TMELEM)
						{ 
							tabelatempchar[cont2] = tabela[cont6].valor[cont8]; 
							cont8++; 
							cont2++;
						}
						
						
						cont5++;
						cont6++;
					}
					
					changevalue=1;
					break;
				}
				pthread_mutex_unlock(tmutex);
			}
			
			cont7++;
			
			if(cont7 == PFILE) break;
				
			cont3 = cont3 + NETABLE;
			cont4++;
		}
		else
		{
			tmp=tmp+TMELEM;
		}
		
		cont=cont+TMELEM;
		
	}
	
	tempos[myrank].tmpFinalPreProcessamento= RDTSC();
	
	while(1)
	{
		// Informa que o Compressor acabou o arquivo
		pthread_mutex_lock(tmutex);
		if(changevalue == 0)
		{
			changevalue = 3;
			pthread_mutex_unlock(tmutex);
			break;
		}
		pthread_mutex_unlock(tmutex);
	}
	
	// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
	
	tempos[myrank].tmpInicialProcessamentoTabela= RDTSC();
	
				
	// Monta tabela final, cria segmento compactado e manda pra main enviar ao MASTER
	while(1)
	{
		pthread_mutex_lock(tmutex);
		
		if(changevalue == 4)
		{
			pthread_mutex_unlock(tmutex);
			
			
			//  0 29 --  30 59 -- 60 89 -- 90 119           ||   120 149 --  150 179 -- 180 209 -- 210 119 
			//  240 269 --  270 299 -- 300 329 -- 330 359   ||   360 389 --  390 419 -- 420 449 -- 450 359
			
			// Ordena em ordem crescente
			
			t = malloc(sizeof(struct Table) * 1);
			t->valor = malloc (sizeof(char)*(TMELEM));
			
			
			cont=0;
			cont3=0;
			cont1=0;
			while(cont1 < PFILE)
			{
			
				int c2 = ((myrank * (NETABLE*PFILE)) + ((cont * NETABLE) + (NETABLE - 1)));
				int c1 = ((myrank * (NETABLE*PFILE)) + ((cont * NETABLE)));
				
				//printf("%d %d %d\n",myrank,c1,c2);
				
				while(c1 <= c2)
				{
					tabelanodos[c1].number = tabela[cont3].number;
					strcpy(tabelanodos[c1].valor,tabela[cont3].valor);
					
					cont3++;
					c1++;
				}
				cont1++;
				cont++;
			}
			
			// -------------------------------------
			//      Removendo as redundancias     //
			// -------------------------------------
			
			tabelanodos2 = malloc(sizeof(struct Table) * ((NETABLE*PFILE)*(NODE)));
			
			cont5=0;
			while(cont5 < ((NETABLE*PFILE)*(NODE)))
			{
				tabelanodos2[cont5].valor = malloc (sizeof(char)*(TMELEM));
				tabelanodos2[cont5].valor[0] = '\0';
				tabelanodos2[cont5].number = 0;
				cont5++;
			}
			
			cont=0;
			cont3=0;
			
			while(cont < ((NETABLE*PFILE)*(NODE)))
			{
				// Le valor em tabelanodos
				// Pesquisa se valor existe em tabelanodos2
				int flag=0;
				cont2=0;
				while(cont2 < ((NETABLE*PFILE)*(NODE)))
				{
					
					cont8=0,cont4=0;
					while(cont8 < TMELEM){
						if(tabelanodos[cont].valor[cont8] == tabelanodos2[cont2].valor[cont8]) cont4++;
						cont8++;
					}
					if(cont4 == TMELEM)
					{
						//printf("%s -- %s\n",tabelanodos[cont].valor,tabelanodos2[cont2].valor);
						flag=1;
						break;
					}
					
					cont2++;
				}
				
				if(flag == 1) // Valor existe
				{
					tabelanodos2[cont2].number = tabelanodos2[cont2].number + tabelanodos[cont].number;
					//printf("%d -- %s\n",tabelanodos2[cont2].number,tabelanodos2[cont2].valor);
				}
				else // Valor não existe
				{
					if(tabelanodos[cont].number > 0){
					tabelanodos2[cont3].number = tabelanodos[cont].number;
					strcpy(tabelanodos2[cont3].valor,tabelanodos[cont].valor);
					//printf("%d -- %s\n",tabelanodos2[cont3].number,tabelanodos2[cont3].valor);
					cont3++;
					}
				}
				
				cont++;
			}
			
			
			for(cont=0; cont < (((NETABLE*PFILE)*(NODE)));cont++)
			{
				for (cont2 = cont; cont2 < ((NETABLE*PFILE)*(NODE)); cont2++)
				{
					if(tabelanodos2[cont].number < tabelanodos2[cont2].number)
					{
						strcpy(t->valor,tabelanodos2[cont].valor);
						t->number = tabelanodos2[cont].number;
						
						strcpy(tabelanodos2[cont].valor,tabelanodos2[cont2].valor);
						tabelanodos2[cont].number = tabelanodos2[cont2].number;
						
						strcpy(tabelanodos2[cont2].valor,t->valor);
						tabelanodos2[cont2].number = t->number;
						
					}
				}
			}
			
			tabela = malloc(sizeof(struct Table) * ((NETABLE)));
			
			cont8=0;
			while(cont8 < (NETABLE))
			{
				tabela[cont8].valor = malloc (sizeof(char)*(TMELEM));
				tabela[cont8].valor[0] = '\0';
				tabela[cont8].number = 0;
				cont8++;
			}
			
			// Somente os NETABLE primeiros serão considerados
			cont1=0,cont2=0;
			for(cont=0; cont < NETABLE;cont++)
			{
				
				if(tabelanodos2[cont].number > 0)
				{
					strcpy(tabela[cont].valor,tabelanodos2[cont].valor);
					tabela[cont].number = tabelanodos2[cont].number;
					cont1 = cont1 + tabelanodos2[cont].number;
					cont2++;
				}
			}
			
			tempos[myrank].tmpFinalProcessamentoTabela= RDTSC();
			
			tempos[myrank].tmpInicialProcessamentoArquivo= RDTSC();
			
			tsimb = malloc(sizeof(struct Table) * cont2);
			
			int tam=0;
			if(myrank == 0) // Mestre terá os marcadores
			{
				tam = ftemp + ((cont2*2) + 2); // Tamanho original + tabela
				tam = tam - (cont1*TMELEM); // Tamanho do arquivo com tabela - palavras a serem modificadas
				tam = tam + (2*cont1); // Tamanho do arquivo com tabela + palavras modificadas
			}
			else // Escravos terão apenas sua parte do arquivo compactada
			{
				
				tam = ftemp;
				tam = tam - (cont1*TMELEM); // Tamanho do arquivo com tabela - palavras a serem modificadas
				tam = tam + (2*cont1); // Tamanho do arquivo com tabela + palavras modificadas
				
			}
			
			
			fvet[myrank] = tam;
			
			ffile = (char*) malloc (sizeof(char) * tam);
				
			cont=0;
			cont3=48;
			cont4=65;
			cont5=97;
				
			// Cria tabela com marcadores (Para auxiliar processo)
				
			cont8=0;
			while(cont8 < cont2)
			{
				tsimb[cont8].valor = malloc (sizeof(char)*(2));
				tsimb[cont8].valor[0] = '\0';
				cont8++;
			}
				
			// Cria os marcadores
			cont8=0;
			cont=0;
			while(cont < cont2)
			{
				if(cont < 9)
				{
					tsimb[cont].valor[0] = '@';
					tsimb[cont].valor[1] = cont3;
					if(myrank == 0)
					{
						ffile[cont8] = '@';
						cont8++;
						ffile[cont8] = cont3;
						cont8++;
					}
					cont3++;	
				}
				else if(cont >=9 && cont < 25)
				{
					tsimb[cont].valor[0] = '@';
					tsimb[cont].valor[1] = cont4;
					if(myrank == 0)
					{
						ffile[cont8] = '@';
						cont8++;
						ffile[cont8] = cont4;
						cont8++;
					}
					cont4++;
				}
				else if(cont >=25 && cont < 25)
				{
					
					tsimb[cont].valor[0] = '@';
					tsimb[cont].valor[1] = cont5;
					if(myrank == 0)
					{
						ffile[cont8] = '@';
						cont8++;
						ffile[cont8] = cont5;
						cont8++;
					}
					cont5++;
				}
				cont++;
			}
			if(myrank == 0)
			{
				ffile[cont8] = '@';
				cont8++;
				ffile[cont8] = '@';
				cont8++;
			}
			
			/*
			if(myrank == 0) // Mostra tabela de simbolos
			{
				cont=0;
				while(cont < cont2)
				{
					printf("%s\n",tsimb[cont].valor);
					cont++;
				}
			}
			*/
			
			cont=0;
			while(cont+TMELEM <= ftemp)
			{
				cont1=0;
				cont3=cont;
				palavra[0] = '\0';
				while(cont1 < TMELEM)
				{
					palavra[cont1] = tfile[cont3];
					cont1++;
					cont3++;
				}
				
				
				int flag=0;
				for(cont3=0;cont3 < cont2;cont3++)
				{
					if(strcmp(palavra,tabela[cont3].valor) == 0)
					{
						if(tabela[cont3].number > 0)
						{
							flag=1;
							break;
						}
					}
				}
				
				
				if(flag == 1) // Valor encontrado, escreve na tabela o marcador
				{
					ffile[cont8] = tsimb[cont3].valor[0];
					cont8++;
					ffile[cont8] = tsimb[cont3].valor[1];
					cont8++;
					cont=cont+TMELEM;				
				}
				else // Valor não encontrado, escreve na tabela o valor original
				{
					
					ffile[cont8] = tfile[cont];
					cont8++;
					cont++;
					ffile[cont8] = tfile[cont];
					cont8++;
					cont++;
					ffile[cont8] = tfile[cont];
					cont8++;
					cont++;
					ffile[cont8] = tfile[cont];
					cont8++;
					cont++;
				}
				
			}		
			
			fvet[myrank] = cont8;
			
			tempos[myrank].tmpFinalProcessamentoArquivo= RDTSC();
			
			// Neste momento o arquivo esta pronto para ser devolvido ao mestre
			pthread_mutex_lock(tmutex);
			changevalue = 5;
			pthread_mutex_unlock(tmutex);
		}
		pthread_mutex_unlock(tmutex);
	}
}

int main(int argc, char **argv)
{
	int cont=0,cont1=0,cont2=0,i=0,j=0,tag=0;
	int cont5=0,cont6=0,cont7=0,cont8=0;
	int tmp=0,cont3=0,cont4=0,cont0=0;

	changevalue1 = 0;
	int flag=0;
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	tempos = malloc(sizeof(struct Timing) * NODE);
	cont=0;
	while(cont < NODE)
	{
		tempos[cont].tamInicialArquivo=0;
		tempos[cont].tamFinalArquivo=0;
		tempos[cont].tmpInicialLeitura=0;
		tempos[cont].tmpFinalLeitura=0;
		tempos[cont].tmpInicialTotalProcessamento=0;
		tempos[cont].tmpFinalTotalProcessamento=0;
		tempos[cont].tmpInicialTransmissaoFile=0;
		tempos[cont].tmpFinalTransmissaoFile=0;
		tempos[cont].tmpInicialPreProcessamento=0;
		tempos[cont].tmpFinalPreProcessamento=0;
		tempos[cont].tmpInicialTransmissaoLista=0;
		tempos[cont].tmpFinalTransmissaoLista=0;
		tempos[cont].tmpInicialProcessamentoTabela=0;
		tempos[cont].tmpFinalProcessamentoTabela=0;
		tempos[cont].tmpInicialProcessamentoArquivo=0;
		tempos[cont].tmpFinalProcessamentoArquivo=0;
		tempos[cont].tmpInicialTransmissaoMestre=0;
		tempos[cont].tmpFinalTransmissaoMestre=0;
		tempos[cont].tmpInicialEscrita=0;
		tempos[cont].tmpFinalEscrita=0;
		cont++;
	}
	if(myrank == 0) // Master Node
	{
		if(myrank == 0) { tempos[0].tmpInicialLeitura = RDTSC(); }
		
		file = fopen("arquivo.txt","rb");
		fseek(file, 0, SEEK_END);
		fsize = ftell(file);
		
		if(myrank == 0) tempos[0].tamInicialArquivo = fsize; 
		
		fseek(file, 0, SEEK_SET);
		
		ftemp = fsize/NODE;
		
		for(cont=1;cont < NODE;cont++)
		{
			tfile = (char*) malloc (sizeof(char) * ftemp);
			
			cont1 = 0;
			while(cont1 < ftemp)
			{
				char c;
				fscanf(file,"%c",&c);
				tfile[cont1] = c;
				cont1++;
			}
			MPI_Send(&ftemp,1,MPI_INT,cont,tag,MPI_COMM_WORLD);
			MPI_Send(tfile,ftemp,MPI_CHAR,cont,tag,MPI_COMM_WORLD);
		}
		
		ftemp = fsize-(ftemp*(NODE-1));
		
		tfile = (char*) malloc (sizeof(char) * (fsize-(ftemp*(NODE-1))));
		
		cont1 = 0;
		while(cont1 < ftemp)
		{
			char c;
			fscanf(file,"%c",&c);
			tfile[cont1] = c;
			cont1++;
		}
		if(myrank == 0) { tempos[0].tmpFinalLeitura = RDTSC(); }
	}
	else
	{
		MPI_Recv(&ftemp,1,MPI_INT,0,tag,MPI_COMM_WORLD, &status);
		tfile = (char*) malloc (sizeof(char) * ftemp);
		MPI_Recv(tfile,ftemp,MPI_CHAR,0,tag,MPI_COMM_WORLD, &status);
	}	
	if(myrank == 0) { tempos[0].tmpInicialTotalProcessamento = RDTSC(); }
	tmutex = (pthread_mutex_t*) malloc (sizeof(pthread_mutex_t) * 1);
	pthread_mutex_init(tmutex, NULL);
		
	tmutex1 = (pthread_mutex_t*) malloc (sizeof(pthread_mutex_t) * 1);
	pthread_mutex_init(tmutex1, NULL);
	
	thread = (pthread_t*) malloc (sizeof(pthread_t)*1);
	pthread_create(thread, NULL,compress,NULL);
	
	thread1 = (pthread_t*) malloc (sizeof(pthread_t)*1);
	pthread_create(thread1, NULL,receive,NULL);
	
	flag=0;
		
	while(1)
	{
		pthread_mutex_lock(tmutex);
		switch(changevalue)
		{
			case(1): // Deve enviar tabela
			{
				pthread_mutex_unlock(tmutex);
				tempos[myrank].tmpInicialTransmissaoLista= RDTSC();
				int cont=-1,cont1=0;
				for(cont1=0;cont1 < NODE;cont1++) // Envia tabela (-1) para todos os nodos
				{
					if(cont1 != myrank)
					{
						MPI_Send(&cont,1,MPI_INT,cont1,tag,MPI_COMM_WORLD);
						// Manda parte char da tabela
						MPI_Send(tabelatempchar,(NETABLE*TMELEM),MPI_CHAR,cont1,tag,MPI_COMM_WORLD);
						// Manda parte inteira da tabela
						MPI_Send(tabelatempint,NETABLE,MPI_INT,cont1,tag,MPI_COMM_WORLD);
					}
				}
				
				pthread_mutex_lock(tmutex);
				
				changevalue = 0; // Libera var comp para compressor enviar outro pedaço
				tempos[myrank].tmpFinalTransmissaoLista= RDTSC();
				pthread_mutex_unlock(tmutex);
				
				break;

			}
			
			case(3): // Compressor acabou de processar o arquivo
			{
				pthread_mutex_unlock(tmutex);
				pthread_mutex_lock(tmutex1); // Pesquisa para ver se todos os pedaços chegaram
				
				if(changevalue1 == 2)
				{
					pthread_mutex_lock(tmutex);
					changevalue = 4;
					changevalue1 = 0;
					pthread_mutex_unlock(tmutex);
				}
				
				pthread_mutex_unlock(tmutex1);
				break;
			}
			
			case(5): // Mestre recebe dos demais escravos e remonta o arquivo
			{
				pthread_mutex_unlock(tmutex);
				if(myrank == 0)
				{
					// Neste momento o mestre já processou o arquivo
					if(myrank == 0) { tempos[0].tmpInicialEscrita = RDTSC(); }
					fileo = fopen("saida.txt","w"); 
					
					for(cont=0;cont < fvet[0];cont++)
					{
						fprintf(fileo,"%c",ffile[cont]);
					}
					
					tabela = malloc(sizeof(struct Table) * ((NODE-1)));
					
					// Recebe tabelas
					
					
					
					cont=1;
					while(1)
					{
						int cont2=0;
						MPI_Recv(&cont2,1,MPI_INT,MPI_ANY_SOURCE,tag,MPI_COMM_WORLD, &status);
						if(cont2 == -2)
						{
							cont3=0;
							MPI_Recv(&cont3,1,MPI_INT,MPI_ANY_SOURCE, tag, MPI_COMM_WORLD, &status);
							
							tabela[status.MPI_SOURCE].valor = malloc (sizeof(char)*(cont3));
							tabela[status.MPI_SOURCE].valor[0] = '\0';
							tabela[status.MPI_SOURCE].number = cont3;
							MPI_Recv(tabela[status.MPI_SOURCE].valor,cont3,MPI_CHAR, MPI_ANY_SOURCE,MPI_ANY_TAG, MPI_COMM_WORLD,&status);
							
						}
						
						cont++;
						if(cont == NODE) break;
					}
					
					if(myrank == 0) tempos[0].tamFinalArquivo = tempos[0].tamFinalArquivo + fvet[0];
					
						
					cont=1;
					while(cont < (NODE))
					{
						if(myrank == 0) tempos[0].tamFinalArquivo = tempos[0].tamFinalArquivo + tabela[cont].number;
						cont1=0;
						while(cont1 < tabela[cont].number)
						{
							fprintf(fileo,"%c",tabela[cont].valor[cont1]);
							cont1++;
						}
						cont++;
					}
					
					if(myrank == 0) { tempos[0].tmpFinalEscrita = RDTSC(); }
					if(myrank == 0) { tempos[0].tmpFinalTotalProcessamento = RDTSC(); }
					
					cont3=2;
					// Manda 2 para todos os escravos dizendo para finalizar a execução.
					for(cont1=1;cont1 < NODE;cont1++)
					{
						MPI_Send(&cont3,1,MPI_INT,cont1,tag,MPI_COMM_WORLD);
					}
					
					flag = 1;
					
				}
				else
				{
					int cont = -2;
					tempos[myrank].tmpInicialTransmissaoMestre= RDTSC();
					MPI_Send(&cont,1,MPI_INT,0,tag,MPI_COMM_WORLD);
					MPI_Send(&fvet[myrank],1,MPI_INT,0,tag,MPI_COMM_WORLD);
					MPI_Send(ffile,fvet[myrank],MPI_CHAR,0,tag,MPI_COMM_WORLD);
					tempos[myrank].tmpFinalTransmissaoMestre= RDTSC();
					
					cont2=0;
					while(1)
					{
						MPI_Recv(&cont2,1,MPI_INT, 0,tag, MPI_COMM_WORLD,&status);
						if(cont2 == 2) break;
					}
					
					flag = 1;
					
				}
				break;
			}
			default:
				pthread_mutex_unlock(tmutex);
		}
		if(flag == 1) 
		{
			if(myrank == 0)
			{
				printf("FComp : Final : %ld Original : %ld\n",tempos[0].tamFinalArquivo,tempos[0].tamInicialArquivo);
				printf("TleituraDisco : %ld\n",tempos[0].tmpFinalLeitura-tempos[0].tmpInicialLeitura);
				printf("TEscritaDisco : %ld\n",tempos[0].tmpFinalEscrita-tempos[0].tmpInicialEscrita);
				printf("TTotalProcess : %ld\n",tempos[0].tmpFinalTotalProcessamento-tempos[0].tmpInicialTotalProcessamento);
				printf("\n\n");
			}
			
			cont=0;
			while(cont < NODE)
			{
				if(myrank == cont)
				{
					printf("TPreProFile : [%d] %ld\n",cont,tempos[cont].tmpFinalPreProcessamento-tempos[cont].tmpInicialPreProcessamento);
					printf("TTransmTab : [%d] %ld\n",cont,tempos[cont].tmpFinalTransmissaoLista-tempos[cont].tmpInicialTransmissaoLista);
					printf("TProcTab : [%d] %ld\n",cont,tempos[cont].tmpFinalProcessamentoTabela-tempos[cont].tmpInicialProcessamentoTabela);
					printf("TProFile: [%d] %ld\n",cont,tempos[cont].tmpFinalProcessamentoArquivo-tempos[cont].tmpInicialProcessamentoArquivo);
					printf("TTranFileMaster: [%d] %ld\n",cont,tempos[cont].tmpFinalTransmissaoMestre-tempos[cont].tmpInicialTransmissaoMestre);
					printf("\n\n");
					break;
				}
				cont++;
			}
			break;
		}
	}
	MPI_Finalize();
}
