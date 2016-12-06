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
int signature_length;
char* signature;
char server_adress[] = "127.0.0.1";
int udpSocket;

gpgme_ctx_t ctx;
gpgme_error_t err;
gpgme_data_t in, result;
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
	err = gpgme_data_new (&result);
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
	printf("Manuel beendet\n");
	if(udpSocket > 0) //nur falls ein socket offen ist
	{
		close(udpSocket);
		free(signature);
		gpgRelease();
		printf("Socket geschlossen\n");
	}
	exit(1); //schließe
}

void printUser(){
	gpgme_key_t key;
	
	printf("Start Finding all Users: \n");
	
	err = gpgme_op_keylist_start (ctx, NULL, 0);
	while (!err)
	      {
		err = gpgme_op_keylist_next (ctx, &key);
		if (err)
		  break;
		printf ("Fingerprint: %s\n", key->subkeys->fpr);
		if (key->uids && key->uids->name)
		  printf ("Name: %s\n", key->uids->name);
		if (key->uids && key->uids->email)
		  printf ("Email: <%s>\n", key->uids->email);
		
		putchar ('\n');
		gpgme_key_release (key);
	      }
		
	printf("Found all Users: \n");	
}

void gpgCheckSign() {
	// Create a data object that contains the text to sign
	err = gpgme_data_new_from_mem (&in, signature, signature_length, 0);
	// Error handling
	fail_if_err (err);



	// Rewind the "out" data object
	gpgme_data_seek (in, 0, SEEK_SET);

	// Perform a decrypt/verify action
	err = gpgme_op_decrypt_verify (ctx, in, result);

	// Retrieve the verification result
	verify_result = gpgme_op_verify_result (ctx);

	// Error handling
	if (err != GPG_ERR_NO_ERROR && !verify_result)
        	fail_if_err (err);
	
    
	// Check if the verify_result object has signatures
	if (verify_result && verify_result->signatures) {
		sig=verify_result->signatures;
		
		int valid = sig->summary==GPGME_SIGSUM_VALID;
		int green = sig->summary==GPGME_SIGSUM_GREEN;
		if(valid || green){
			if(valid){printf("Die Signatur ist VALID\n");	}
			if(green){printf("Die Signatur ist GREEN\n");	}
		}
	}

	if (err != GPG_ERR_NO_ERROR && tnsigs < 1)
	fail_if_err(err);
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
	
    gpgInit();
 
    if(argc==needed_arguments){       
        server_port = atoi(argv[1]);
	    
	//Einfacher UDP Server http://www.programminglogic.com/sockets-programming-in-c-using-udp-datagrams/
	
	
	struct sockaddr_in serverAddr;
	struct sockaddr_storage serverStorage;
	socklen_t addr_size;

	/*Create UDP socket*/
	udpSocket = socket(PF_INET, SOCK_DGRAM, 0);

	  /*Configure settings in address struct*/
	  serverAddr.sin_family = AF_INET;
	  serverAddr.sin_port = htons(server_port);
	  serverAddr.sin_addr.s_addr = inet_addr(server_adress);
	  memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);  

	  /*Bind socket with address struct*/
	  bind(udpSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));
	  
	  printf("Server starting: %s:%i \n\n",server_adress,server_port);  
	  /*Initialize size variable to be used later on*/
	  addr_size = sizeof serverStorage;
	    
	  signature = malloc(sizeof(char)*65536);
	  while(keep_alive){
	    /* Try to receive any incoming UDP datagram. Address and port of 
	      requesting client will be stored on serverStorage variable */
	    signature_length = recvfrom(udpSocket,signature,65536,0,(struct sockaddr *)&serverStorage, &addr_size);
	    
	    if(signature_length>0){	  
		    gpgCheckSign();
	    }
	  }
	  free(signature);
	  gpgRelease();
       
                
    }
    else{
        printf("usage: ./pa3_server PORT \n");
    }
  
    return 0;
}
