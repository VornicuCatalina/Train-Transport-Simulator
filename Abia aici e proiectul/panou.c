//pentru calator

//bibliotecile

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>


//pentru erori

extern int errno;

//portul

int port;

//programul principal

int main(char argc, char *argv[])
{
	//variabile
	
	int sd; //pt socket
	struct sockaddr_in server;
	int nr=0;
	char buf[100]={0},ajutor[50]={0},citire[200]={0};
	char *hm;
	
	//verificare parametrii
	
	if(argc != 3)
	{
		printf("[server] Sintaxa este gresita! Introduceti sub formatul urmator:\n");
		printf("[server] %s <adresa_conectare> 2022\n",argv[0]);
		return -1;
	}
	
	//stabilire port
	
	port=atoi(argv[2]);
	
	//creare socket
	
	if((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
      	{
      		printf("[server] Eroare la creare socket\n");
      		return errno;
      	}
      	
      	//completare structura
      	
      	server.sin_family = AF_INET;			//familie socket
  	server.sin_addr.s_addr = inet_addr(argv[1]);   //IP server
  	server.sin_port = htons (port);		//port conectare
  	  
  	//conectarea la server
  	
  	if(connect (sd, (struct sockaddr *) &server,sizeof (struct sockaddr)) == -1)
  	{
  		printf("[server] Eoare la connect()\n");
  		return errno;
  	}
  	
  	printf("\n[server] S-a stabilit conexiunea la server\n");
  	printf("[server] Va rugam sa introduceti locatia!\n");
  	
  	strcpy(buf,"panou");
  	if(write(sd,buf,sizeof(buf))<=0)
  	{
  		perror("Aici la introducere");
  	}
  	/*if(read(sd,buf,sizeof(buf))<=0)
  	{
  		perror("La citire");
  		}*/
  	while(1)
  	{
  		bzero(buf,100);
  		printf("[client] ");
  		gets(buf);

    		printf("\n");
  		//verificare ca este una din cele doua comenzi
  			  		if (write (sd,buf,sizeof(buf)) <= 0)
    		{
      			perror ("[server]Eroare la write() spre server.\n");
      			return errno;
    		}
  			while(read(sd,citire,sizeof(citire))>0)
  			{
  				if(strstr(citire,"Nu exista"))
  				{
  				printf("[server] Nu exista!\n");
  				 break;
  				}
  				 if(strcmp(citire,"Aici"))
  				{
  				printf(" %s\n",citire);
  				printf("--------------------------------------------------------------------------------------------------------------------------------------------------------\n");
  				}
  				else
  				{
  				printf("========================================================================================================================================================\n\n\n");
  				}
  				bzero(citire,200);
  			}
  			bzero(citire,200);
  		printf("\n");
  		printf("[server] Va rugam sa introduceti locatia:\n");
  	}
  	
  	close(sd);
}

