#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include<stdlib.h>    //malloc
#include<string.h>    //für strings

#include <signal.h> //Damit ich Signale abfangen kann

int server_port;
int debug = 1;
int listen = 1;

int sock_raw; // erstelle unseren socket den Wir brauchen
char buffer[65536];

/**
* Letzer aufruf um alles wichtige zu schließen
*/
void last_wish(int s){
           printf("Manuel beendet\n");
           if(sock_raw > 0) //nur falls ein socket offen ist
           {
               close(sock_raw); //schließe diesen
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
           // Einfacher UDP Server https://www.cs.utah.edu/~swalton/listings/sockets/programs/part1/chap4/udp-server.c    
           
           int sd;
	int server_port=80;
	struct sockaddr_in addr;
        
           port = atoi(argv[1]);
	sd = socket(PF_INET, SOCK_DGRAM, 0);
	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = INADDR_ANY;
	if ( bind(sd, (struct sockaddr*)&addr, sizeof(addr)) != 0 )
		perror("bind");
	while (listen)
	{	int bytes, addr_len=sizeof(addr);

		bytes = recvfrom(sd, buffer, sizeof(buffer), 0, (struct sockaddr*)&addr, &addr_len);
		printf("msg from %s:%d (%d bytes)\n", inet_ntoa(addr.sin_addr),
						ntohs(addr.sin_port), bytes);
		
            
	}
        
        if(debug){
            printf("Starting on Port: %s\n", server_port);
        }
                
    }
    else{
        printf("usage: ./pa3_server SERVER_PORT \n");
    }
  
    return 0;
}
