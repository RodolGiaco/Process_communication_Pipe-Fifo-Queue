/////////// Pipe_Queue /////////////////////
/*
Cuatro procesos (A,B,C,D) relacionados de la siguiente forma:
A padre de B y padre de C

Los procesos se comunican de la siguiente forma:
A y B se comunican por medio de un pipe
A y C se comunican por medio de un pipe
A y D se comunican por medio de una cola de mensajes

1) El proceso A crea la cola de mensajes, crea al proceso B y al proceso C 
2) El proceso B cuando recibe la señal SIGUSR1 escribe en el pipe1 el mensaje "0123456789" y termina.
3) El proceso C cuando recibe la señal SIGUSR2 escribe en el pipe2 el mensaje "ABCDEFGHIJ" y termina.
4) El proceso A espera a que termine el proceso B, lee el pipe1 y lo leído lo envía a la cola de mensajes 
5) El proceso A espera a que termine el proceso C, lee el pipe2 y lo leído lo envía a la cola de mensajes 
6) El proceso D lee los mensajes de la cola mensajes (2 mensajes) y los muestra en pantalla

El proceso A debe ser el primero en comenzar en una consola
El proceso D comienza luego de A otra consola
Las señales se envían desde otra consola


       --- 
      | B |
       --- 
       |     -------      ---      --------      ---      --------  
       |--->| PIPE1 |--->|   |    | Cola   |    |   |    |        |  
       |     -------     | A |--->|Mensajes|--->| D |--->|pantalla|
       |     -------     |   |--->|        |--->|   |--->|        | 
       |--->| PIPE2 |--->|   |     --------      ---      --------  
       |     -------      ---
       ---              
      | C |
       --- 
*/
#include <errno.h>
#include <fcntl.h>
#include <mqueue.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <wait.h>
#include <arpa/inet.h> 

#define MENSAJE_A "0123456789"
#define MENSAJE_B "ABCDEFGHIJ"

#define MQ_PATH "/Rodol"

int ipc1[2], ipc2[2],pid1,pid2,i;
int leido;
char buff[100];


void pipe1(int a)
{
	  close(ipc1[0]);      
      strncpy(buff, MENSAJE_A, sizeof(MENSAJE_A));
      write(ipc1[1], buff, sizeof(MENSAJE_A));
      exit(0);
}
void pipe2(int b)
{
      close(ipc2[0]);               
      strncpy(buff, MENSAJE_B, sizeof(MENSAJE_B)); 
      write(ipc2[1], buff,    sizeof(MENSAJE_B));
      exit(0); 
}

int main (int argc, const char *argv[])
{
   signal(SIGUSR1, pipe1);
   signal(SIGUSR2, pipe2);
 
   pipe(ipc1);
   pipe(ipc2);
   
   int fd_queue;
   struct sockaddr_in direccion={};
   struct mq_attr attr, attr_rcv;  
   
   /////////////CREO QUEUE///////////////
   unlink(MQ_PATH);
   attr.mq_msgsize = 8192;
   attr.mq_maxmsg = 5;
   fd_queue = mq_open(MQ_PATH,O_WRONLY | O_CREAT, 0777,&attr);
      if(fd_queue < 0)
      {
    	printf("Error en crear la queue\b");
		exit(-1);
      }
   /////////////////////////////////////
    
   pid1 = fork();
   if(pid1 == 0)
   {
	   printf("Hijo 1 PID: %d\n",getpid());
	   while(1){}

   }else{
	   
	   pid2 = fork();
	   if(pid2 == 0)
	   {
		   printf("Hijo 2 PID: %d\n",getpid());
		   while(1){}
		  
	   }else{
		   wait(NULL);
           wait(NULL);
           close(ipc1[1]);
		   close(ipc2[1]);
		   for(i=0; i<2; i++)
		   {
				
				if(i==0)
				{
					leido = read(ipc1[0], buff, sizeof(buff));
				    ///Escribir por consola///
                    write (1,"\nLeido del Pipe 1: ", sizeof("\nLeido del Pipe 1:"));
                    write (1, buff, leido-1);
                    printf("\nPor el proceso padre PID: %d\n", getpid());	
					
					///Escribir en la Queue///
					mq_send(fd_queue, buff, strlen(buff)+1,1);
	                printf("MENSAJE_A leido por el proceso A\n");  
				}
				if(i==1)
				{
					leido = read(ipc2[0], buff, sizeof(buff));
				    ///Escribir por consola///
                    write (1,"\nLeido del pipe 2: ", sizeof("\nLeido del pipe 2:"));
                    write (1, buff, leido-1);
                    printf("\nPor el proceso padre, PID: %d \n", getpid());
					
					///Escribir en la Queue///
					mq_send(fd_queue, MENSAJE_B, strlen(buff)+1,1);
	                printf("MENSAJE_B leido por el proceso A\n");	 
				}
               
            }
		    //Se destruye la tuberia//
		    close(ipc1[0]);
			close(ipc2[0]);
            exit(0);
	   }}

}	   