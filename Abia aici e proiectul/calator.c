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

//functii introduse

void menu();

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
  	printf("[server] O sa se trimita meniul\n");
  	menu();
  	
  	strcpy(buf,"calator");
  	if(write(sd,buf,sizeof(buf))<=0)
  	{
  		perror("Aici la introducere");
  	}
  	while(read(sd,citire,sizeof(citire))>0)
  			{
  				printf("[server] %s\n",citire);
  				if(strstr(citire,"Comanda"))
  				 break;
  				bzero(citire,200);
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
  				printf("[server] %s\n",citire);
  				if(strstr(citire,"Comanda"))
  				 break;
  				bzero(citire,200);
  			}
  			bzero(citire,200);
  			if(strcmp(buf,"quit")==0)
  			{
  			printf("[server] V-ati deconectat cu succes\n");
  			break;
  			}
  			if(strcmp(buf,"meniu")==0)
  		{
  			menu();
  		}
  		printf("\n");
  	}
  	
  	close(sd);
}

//functiile facute

void menu()
{
	printf("[server] *****MENIU*****\n\n");
	printf("[server] --> meniu (ce observati acum)\n");
	printf("[server] --> sosiri (veti vedea ce trenuri sosesc in intervalul mai scurt sau egal de 1h)\n");
	printf("[server] -----------> Server-ul va va mai intreba despre locatie, iar dvs introduceti o locatie sau introduceti \'-\' in caz ca le vreti pe toate\n");
	printf("[server] --> plecari (veti vedea ce trenuri pleaca in intervalul mai scurt sau egal de 1h)\n");
	printf("[server] -----------> Server-ul va va mai intreba despre locatie, iar dvs introduceti o locatie sau introduceti \'-\' in caz ca le vreti pe toate\n");
	printf("[server] --> quit (daca vreti sa iesiti de pe aplicatie)\n\n");
}

