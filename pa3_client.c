#include <stdlib.h>    //malloc
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>

char server_adress[65536];
int server_port = 80;
char username[65536];
char message[65536];
int debug = 1;

char private_key[65536];
 
int main(int argc, char **argv){
    int needed_arguments = 1; //programm self
    needed_arguments++; //Server Adress
    needed_arguments++; //Server Port
    needed_arguments++; //UserName
    needed_arguments++; //From Here up the Message
 
    if(argc==needed_arguments){
        strncpy(server_adress, argv[1], sizeof server_adress);
        server_port = atoi(argv[2]);
        strncpy(username, argv[3], sizeof username);
        strncpy(message, argv[4], sizeof message);
     
        if(debug){
            printf("Server Adress: %s\n", server_adress);
            printf("Server Port: %i\n", server_port);
            printf("Username: %s\n", username);
            printf("Message: %s\n", message);
        }
     
     
     
     int clientSocket, nBytes;
     
     struct sockaddr_in serverAddr;
     socklen_t addr_size;

     /*Create UDP socket*/
     clientSocket = socket(PF_INET, SOCK_DGRAM, 0);

     /*Configure settings in address struct*/
     serverAddr.sin_family = AF_INET;
     serverAddr.sin_port = htons(server_port);
     serverAddr.sin_addr.s_addr = inet_addr(server_adress);
     memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);  

     /*Initialize size variable to be used later on*/
     addr_size = sizeof serverAddr;

     while(1){
       nBytes = strlen(message) + 1;

       /*Send message to server*/
       sendto(clientSocket,message,nBytes,0,(struct sockaddr *)&serverAddr,addr_size);

     }
     
     
     
     
     
        
    }
    else{
        printf("usage: ./pa3_client SERVER_ADRESS SERVER_PORT USERNAME \"Message to encypt\" \n");
    }
 
    return 0;
}

