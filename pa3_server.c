#include <signal.h> //Damit ich Signale abfangen kann
#include <unistd.h> // für strg c anfangen

#include<stdio.h> //For standard things
#include<stdlib.h>    //malloc
#include<string.h>    //für strings
#include<sys/socket.h>
#include<netinet/in.h>

int server_port = 80;
int debug = 1;
int keep_alive = 1;

char buffer[65536];
int udpSocket;

/**
* Letzer aufruf um alles wichtige zu schließen
*/
void last_wish(int i){
           printf("Manuel beendet\n");
           if(udpSocket > 0) //nur falls ein socket offen ist
           {
	       close(udpSocket);
               printf("Socket geschlossen\n");
           }
           exit(1); //schließe
}


int main(int argc, char **argv){
   //Handlet aktivierung für STRG+C
   //http://stackoverflow.com/questions/1641182/how-can-i-catch-a-ctrl-c-event-c
   struct sigaction sigIntHandler;

   sigIntHandler.sa_handler = last_wish;
   sigemptyset(&sigIntHandler.sa_mask);
   sigIntHandler.sa_flags = 0; //setze sa flags 0

   sigaction(SIGINT, &sigIntHandler, NULL);
   // Ende für STRG+C
    
    
    int needed_arguments = 1; //programm self
    needed_arguments++; //Server Port
 
    if(argc==needed_arguments){       
        server_port = atoi(argv[1]);
	
	  int nBytes;
	  struct sockaddr_in serverAddr, clientAddr;
	  struct sockaddr_storage serverStorage;
	  socklen_t addr_size, client_addr_size;
	  int i;

	  /*Create UDP socket*/
	  udpSocket = socket(PF_INET, SOCK_DGRAM, 0);

	  /*Configure settings in address struct*/
	  serverAddr.sin_family = AF_INET;
	  serverAddr.sin_port = htons(server_port);
	  memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);  

	  /*Bind socket with address struct*/
	  bind(udpSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));

	  /*Initialize size variable to be used later on*/
	  addr_size = sizeof serverStorage;

	  while(keep_alive){
	    /* Try to receive any incoming UDP datagram. Address and port of 
	      requesting client will be stored on serverStorage variable */
	    nBytes = recvfrom(udpSocket,buffer,1024,0,(struct sockaddr *)&serverStorage, &addr_size);
	  }
       
                
    }
    else{
        printf("usage: ./pa3_server SERVER_PORT \n");
    }
  
    return 0;
}
