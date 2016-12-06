#include <signal.h> //Damit ich Signale abfangen kann
#include <unistd.h> // für strg c anfangen

#include <stdio.h> //For standard things
#include <stdlib.h>    //malloc
#include <string.h>    //für strings
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <gpgme.h>   /* gpgme             */
#include <errno.h>   /* errno             */
#include <locale.h>  /* locale support    */

int server_port = 5050; //default port
int debug = 1;
int keep_alive = 1;
int data_length;
char* buffer;
char server_adress[] = "127.0.0.1";
int udpSocket;

gpgme_ctx_t ctx;
gpgme_error_t err;
gpgme_data_t in, result;
gpgme_data_t plain;
gpgme_verify_result_t verify_result;
gpgme_signature_t sig;
int tnsigs, nsigs;


#define fail_if_err(err)                                    \
    do {                                                    \
        if (err) {                                          \
            fprintf (stderr, "%s:%d: %s: %s\n",             \
                __FILE__, __LINE__, gpgme_strsource (err),  \
                gpgme_strerror (err));                      \
            exit (1);                                       \
        }                                                   \
    }                                                       \
    while (0)


void gpgInit(){
	/* Begin setup of GPGME */
	gpgme_check_version (NULL);
	setlocale (LC_ALL, "");
	gpgme_set_locale (NULL, LC_CTYPE, setlocale (LC_CTYPE, NULL));
	/* End setup of GPGME */

	err = gpgme_engine_check_version (GPGME_PROTOCOL_GPGCONF);
	fail_if_err (err);

	// Create the GPGME Context
	err = gpgme_new (&ctx);
	// Error handling
	fail_if_err (err);

	// Create a data object pointing to the result buffer
	
	
	// Error handling
	fail_if_err (err);	
}

void gpgRelease(){
	// Release the "in" data object
	gpgme_data_release (in);

	// Release the context
	gpgme_release (ctx);
}

/**
* Letzer aufruf um alles wichtige zu schließen
*/
void last_wish(int i){
	printf("\nManuelles Beenden\n");
	keep_alive=0;
}

void gpgCheckSign() {
	// Create a data object that contains the text to sign
	err = gpgme_data_new_from_mem (&in, buffer, data_length, 1);
	// Error handling
	fail_if_err (err);

	err = gpgme_data_new (&plain);
	
	// Perform a verify action
	err = gpgme_op_verify (ctx, in,NULL,plain);
	fail_if_err (err);
	
	// Retrieve the verification result
	verify_result = gpgme_op_verify_result (ctx);

	// Error handling
	if (err != GPG_ERR_NO_ERROR && !verify_result)
        	fail_if_err (err);
		
	if(gpg_err_code(verify_result->signatures->status)==GPG_ERR_NO_ERROR){
		gpgme_key_t key;
		err = gpgme_get_key (ctx, verify_result->signatures->fpr, &key, 0);
		
		
		printf("%s: ",key->uids->name);
		
		size_t plainTextLength;
		
		char* plainText = gpgme_data_release_and_get_mem(plain,&plainTextLength);
		//gpg_data_seek(plain,0,SEEK_SET);
		
		int i;
		for(i=0; i<plainTextLength; i++){
			printf("%c",plainText[i]);	
		}
		
		
	}
	else{
		printf("Die Signatur ist INVALID\n");
	}
}


int main(int argc, char **argv){
   //Handlet aktivierung für STRG+C
   //http://stackoverflow.com/questions/1641182/how-can-i-catch-a-ctrl-c-event-c
   struct sigaction sigIntHandler;
int i;

   sigIntHandler.sa_handler = last_wish;
   sigemptyset(&sigIntHandler.sa_mask);
   sigIntHandler.sa_flags = 0; //setze sa flags 0

   sigaction(SIGINT, &sigIntHandler, NULL);
   // Ende für STRG+C
    
    
    int needed_arguments = 1; //programm self
    needed_arguments++; //Server Port
	
    gpgInit();
 
    if(argc==needed_arguments){       
	int err_port = 0;
        i=0;
        while(argv[1][i] != '\0'){
             if (argv[1][i] < 47 || argv[1][i] > 57){
                err_port = 1;
                break;
            }
            i++;
        }
        if(err_port==1){
            printf("Error: No valid Port\n");
            exit(1);
        }        
        server_port = atoi(argv[1]);
	    
	//Einfacher UDP Server http://www.programminglogic.com/sockets-programming-in-c-using-udp-datagrams/
	
	
	struct sockaddr_in serverAddr;
	struct sockaddr serverStorage;
	socklen_t addr_size;

	/*Create UDP socket*/
	udpSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	  /*Configure settings in address struct*/
	  serverAddr.sin_family = AF_INET;
	  serverAddr.sin_port = htons(server_port);
	  serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	  /*Bind socket with address struct*/
	  int binding = bind(udpSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));
	    if(binding<0){
		printf("Binding doesnt worked\n");
		exit(1);    
	    }
	  
	  printf("Server starting: %s:%i \n\n",server_adress,server_port);  
	  /*Initialize size variable to be used later on*/
	  addr_size = sizeof serverStorage;
	    
	  buffer = malloc(sizeof(char)*65536);
	  while(keep_alive){
	    /* Try to receive any incoming UDP datagram. Address and port of 
	      requesting client will be stored on serverStorage variable */
	    data_length = recvfrom(udpSocket,buffer,65536,0,(struct sockaddr *)&serverStorage, &addr_size);
	    
	    if(data_length>=0){	  
		    gpgCheckSign();
	    }
	    else{
		printf("Rec. Error");
		exit(1);
	    }
	  }
	    
	  if(udpSocket > 0){
			close(udpSocket);
	  }
	  free(buffer);
	  gpgRelease();
       
                
    }
    else{
        printf("usage: ./pa3_server PORT \n");
    }
  
    return 0;
}
