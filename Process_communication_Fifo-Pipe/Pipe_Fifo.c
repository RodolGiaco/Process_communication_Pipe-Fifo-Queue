
/////////// Pipe_FIFO /////////////////////
/*
Cuatro procesos (A,B,C,D) relacionados de la siguiente forma:
A padre de B y padre de C

Los procesos se comunican de la siguiente forma:
A y B se comunican por medio de un pipe
A y C se comunican por medio de un pipe
A y D se comunican por medio de una FIFO

1) El proceso A crea la FIFO, crea al proceso B y al proceso C 
2) El proceso B cuando recibe la señal SIGUSR1 escribe en el pipe1 el mensaje "0123456789" y termina.
3) El proceso C cuando recibe la señal SIGUSR2 escribe en el pipe2 el mensaje "ABCDEFGHIJ" y termina.
4) El proceso A espera a que termine el proceso B, lee el pipe1 y lo leído lo envía a la FIFO
5) El proceso A espera a que termine el proceso C, lee el pipe2 y lo leído lo envía a la FIFO
6) El proceso D lee los mensajes de la FIFO (2 mensajes) y los muestra en pantalla

El proceso A debe ser el primero en comenzar en una consola
El proceso D comienza luego de A otra consola
Las señales se envían desde otra consola


       --- 
      | B |
       --- 
       |     -------      ---      --------      ---      --------  
       |--->| PIPE1 |--->|   |    |        |    |   |    |        |  
       |     -------     | A |--->|  FIFO  |--->| D |--->|pantalla|
       |     -------     |   |--->|        |--->|   |--->|        | 
       |--->| PIPE2 |--->|   |     --------      ---      --------  
       |     -------      ---
       ---              
      | C |
       --- 
*/


#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include<sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <mqueue.h>


#define FRASE_A "0123456789"
#define FRASE_B "ABCDEFGHIJ"
#define FIFO_PATH "/tmp/MI_FIFO"
#define BUFF_SIZE 80

int ipc1[2], ipc2[2];
int leido,aux1=0,aux2=0;
char buff[BUFF_SIZE] = {0};

int fd,err;

void pipe1()
{
	strncpy(buff, FRASE_A, sizeof(FRASE_A)); 
	write(ipc1[1], buff, sizeof(FRASE_A));
	exit(0);
	
}
void pipe2()
{
	strncpy(buff, FRASE_B, sizeof(FRASE_B)); 
	write(ipc2[1], buff,    sizeof(FRASE_B));
	exit(0);
	
}

int main (int argc , char const * argv[]){
	
	
	signal(SIGUSR1,pipe1);
	signal(SIGUSR2,pipe2);
	
	/////////////////CREAMOS LA FIFO///////////////
	unlink(FIFO_PATH);
	err=mkfifo(FIFO_PATH, 0777);
	if(err == (-1))
	{
		printf("Se produjo un error\n");
	}else{printf("Se creo la FIFO\n");}
	
	////////////////////////////////////////////////
	
	
	pipe(ipc1);
	pipe(ipc2);
	switch (fork()){ 
		
	case 0:
		close(ipc1[0]);  
		printf("Proceso B PID: %d mi padre: %d\n",getpid(),getppid());
		while(1){}
		break;
		
	default:
		switch (fork()){ 
			
		case 0:
			close(ipc2[0]);   
			printf("Proceso C PID: %d mi padre: %d\n",getpid(),getppid());
			while(1){}
			break;
		default:
			close(ipc1[1]);
			close(ipc2[1]);
			wait(NULL);
			wait(NULL);
			int i;
			
			
			leido = read(ipc1[0], buff, sizeof(buff));
			//Escribir por consola//
			write (1,"\nLeido del Pipe 1: ", sizeof("\nLeido del Pipe 1:"));
			write (1, buff, leido-1);
			printf("\nPor el proceso padre PID: %d\n", getpid());
			
			//Escribimos en la FIFO
			fd = open(FIFO_PATH, O_WRONLY, 0);
			write(fd, buff, sizeof(buff));
			
			
			leido = read(ipc2[0], buff, sizeof(buff));
			//Escribir por consola//
			write (1,"\nLeido del pipe 2: ", sizeof("\nLeido del pipe 2:"));
			write (1, buff, leido-1);
			printf("\nPor el proceso padre, PID: %d \n", getpid());
			
			//Escribimos en la FIFO
			fd = open(FIFO_PATH, O_WRONLY, 0);
			write(fd, buff, sizeof(buff));
			
			
			
			close(ipc1[0]);
			close(ipc2[0]);
			close(fd);
			exit(0);
			break;
		}
	}   
}
	
