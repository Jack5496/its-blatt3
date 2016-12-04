#include <stdlib.h>    //malloc
#include <string.h>    //für strings
#include <signal.h> //Damit ich Signale abfangen kann
#include <stdio.h>

#include<arpa/inet.h>
#include<sys/socket.h>

int server_port = 80;
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

void die(char *s)
{
    perror(s);
    exit(1);
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
	
	    
	    
	    
	    
	    struct sockaddr_in si_me, si_other;
     
    int s, slen = sizeof(si_other) , recv_len;
	    
    //create a UDP socket
    if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        die("socket");
    }
     
    // zero out the structure
    memset((char *) &si_me, 0, sizeof(si_me));
     
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(server_port);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);
     
    //bind socket to port
    if( bind(s , (struct sockaddr*)&si_me, sizeof(si_me) ) == -1)
    {
        die("bind");
    }
     
    //keep listening for data
    while(listen)
    {
        printf("Waiting for data...");
        fflush(stdout);
         
        //try to receive some data, this is a blocking call
        if ((recv_len = recvfrom(s, buffer, 65536, 0, (struct sockaddr *) &si_other, &slen)) == -1)
        {
            die("recvfrom()");
        }
         
        //print details of the client/peer and the data received
        printf("Received packet from %s:%d\n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
        printf("Data: %s\n" , buffer);
        
    }
 
    close(s);
        
       
                
    }
    else{
        printf("usage: ./pa3_server SERVER_PORT \n");
    }
  
    return 0;
}
