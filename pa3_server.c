#include <signal.h> //Damit ich Signale abfangen kann
#include <unistd.h> // für strg c anfangen

#include <stdio.h> //For standard things
#include <stdlib.h>    //malloc
#include <string.h>    //für strings
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int server_port = 80;
int debug = 1;
int keep_alive = 1;
int signature_length;
char* signature;
char server_adress[] = "127.0.0.1";
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

#include <gpgme.h>

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

void gpgCheckSign() {
    gpgme_ctx_t ctx;
    gpgme_error_t err;
    gpgme_data_t in, out, result;
    gpgme_verify_result_t verify_result;
    gpgme_signature_t sig;
    int tnsigs, nsigs;
    int BUF_SIZE = 512;
    char buf[BUF_SIZE + 1];
    int ret;
    /* Set the GPGME signature mode
        GPGME_SIG_MODE_NORMAL : Signature with data
        GPGME_SIG_MODE_CLEAR  : Clear signed text
        GPGME_SIG_MODE_DETACH : Detached signature */
    gpgme_sig_mode_t sigMode = GPGME_SIG_MODE_CLEAR;

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

    // Create a data object that contains the text to sign
    err = gpgme_data_new_from_mem (&in, signature, signature_length, 0);
    // Error handling
    fail_if_err (err);

    // Rewind the "out" data object
    ret = gpgme_data_seek (signature, 0, SEEK_SET);

    // Perform a decrypt/verify action
    err = gpgme_op_decrypt_verify (ctx, signature, result);

    // Retrieve the verification result
    verify_result = gpgme_op_verify_result (ctx);

    // Error handling
    if (err != GPG_ERR_NO_ERROR && !verify_result)
        fail_if_err (err);

    // Check if the verify_result object has signatures
    if (verify_result && verify_result->signatures) {
        // Iterate through the signatures in the verify_result object
        for (nsigs=0, sig=verify_result->signatures; sig; sig = sig->next, nsigs++) {
            fprintf(stdout, "Signature made with Key: %s\n", sig->fpr);
            fprintf(stdout, "Created: %lu; Expires %lu\n", sig->timestamp, sig->exp_timestamp);
            char *validity = sig->validity == GPGME_VALIDITY_UNKNOWN? "unknown":
                    sig->validity == GPGME_VALIDITY_UNDEFINED? "undefined":
                    sig->validity == GPGME_VALIDITY_NEVER? "never":
                    sig->validity == GPGME_VALIDITY_MARGINAL? "marginal":
                    sig->validity == GPGME_VALIDITY_FULL? "full":
                    sig->validity == GPGME_VALIDITY_ULTIMATE? "ultimate": "[?]";
            char *sig_status = gpg_err_code (sig->status) == GPG_ERR_NO_ERROR? "GOOD":
                    gpg_err_code (sig->status) == GPG_ERR_BAD_SIGNATURE? "BAD_SIG":
                    gpg_err_code (sig->status) == GPG_ERR_NO_PUBKEY? "NO_PUBKEY":
                    gpg_err_code (sig->status) == GPG_ERR_NO_DATA? "NO_SIGNATURE":
                    gpg_err_code (sig->status) == GPG_ERR_SIG_EXPIRED? "GOOD_EXPSIG":
                    gpg_err_code (sig->status) == GPG_ERR_KEY_EXPIRED? "GOOD_EXPKEY": "INVALID";
            fprintf(stdout, "Validity: %s; Signature Status: %s", validity, sig_status);
            fwrite("\n", 1, 1, stdout);
            tnsigs++;
        }
    }

    if (err != GPG_ERR_NO_ERROR && tnsigs < 1)
        fail_if_err(err);

    // Release the "in" data object
    gpgme_data_release (in);
    // Release the "out" data object
    gpgme_data_release (out);

    // Release the context
    gpgme_release (ctx);
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
	  
	  printf("Server Gestartet \n");  
	  printf("Adresse: %s \n",server_adress);
	  printf("Port: %i \n",server_port);
	  /*Initialize size variable to be used later on*/
	  addr_size = sizeof serverStorage;

	  while(keep_alive){
	    /* Try to receive any incoming UDP datagram. Address and port of 
	      requesting client will be stored on serverStorage variable */
	    signature_length = recvfrom(udpSocket,signature,1024,0,(struct sockaddr *)&serverStorage, &addr_size);
	    
	    if(signature_length>0){	  
		    int i;
		    for(i=0; i<signature_length;i++){
			printf("%c",signature[i]);
		    }
		    gpgCheckSign();
	    }
	  }
       
                
    }
    else{
        printf("usage: ./pa3_server SERVER_PORT \n");
    }
  
    return 0;
}
