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

int gpgme_global_error = 0;

gpgme_ctx_t ctx;
gpgme_error_t err;
gpgme_data_t in, result;
gpgme_data_t plain;
gpgme_verify_result_t verify_result;
gpgme_signature_t sig;
int tnsigs, nsigs;

void gpgInit(){
	/* Begin setup of GPGME */
	gpgme_check_version (NULL);
	setlocale (LC_ALL, "");
	gpgme_set_locale (NULL, LC_CTYPE, setlocale (LC_CTYPE, NULL));
	/* End setup of GPGME */

	err = gpgme_engine_check_version (GPGME_PROTOCOL_GPGCONF);
	if(err){
		printf("GPGME: Engine Check failed\n");
		keep_alive = 0;
		return;
	}

	// Create the GPGME Context
	err = gpgme_new (&ctx);
	if(err){
		printf("GPGME: Context creation failed\n");
		keep_alive = 0;
		return;
	}
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
	if(err){
		printf("GPGME: Read Buffer into In failed\n");
		keep_alive = 0;
		return;
	}

	err = gpgme_data_new (&plain);
	if(err){
		printf("GPGME: Data new plain failed\n");
		keep_alive = 0;
		return;
	}
	
	// Perform a verify action
	err = gpgme_op_verify (ctx, in,NULL,plain);
	if(err){
		printf("GPGME: Verify failed\n");
		gpgme_data_release (plain);
		keep_alive = 0;
		return;
	}
	
	// Retrieve the verification result
	verify_result = gpgme_op_verify_result (ctx);
	if(err){
		printf("GPGME: Verify result failed\n");
		gpgme_data_release (plain);
		keep_alive = 0;
		return;
	}

	if (verify_result && verify_result->signatures && verify_result->signatures->status){
		if(gpg_err_code(verify_result->signatures->status)==GPG_ERR_NO_ERROR){
			gpgme_key_t key;
			err = gpgme_get_key (ctx, verify_result->signatures->fpr, &key, 0);
			if(err){
				printf("GPGME: get Key failed\n");
				gpgme_data_release (plain);
				keep_alive = 0;
				return;
			}

			printf("%s: ",key->uids->name);

			size_t plainTextLength;
			
			gpg_data_seek(plain,0,SEEK_SET);
			char* plainText = gpgme_data_release_and_get_mem(plain,&plainTextLength);

			int i;
			for(i=0; i<plainTextLength; i++){
				printf("%c",plainText[i]);	
			}


		}
		else{
		printf("Die Signatur ist INVALID\n");
		}
	}
	else{
		printf("GPGME: Verify result failed\n");
		gpgme_data_release (plain);
		keep_alive = 0;
		return;
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
            keep_alive = 0;
        }        
        server_port = atoi(argv[1]);
	    
	//Einfacher UDP Server http://www.programminglogic.com/sockets-programming-in-c-using-udp-datagrams/
	
	
	struct sockaddr_in serverAddr;
	struct sockaddr serverStorage;
	socklen_t addr_size;

	/*Create UDP socket*/
	udpSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(udpSocket<0){
	    printf("Unable to setup Socket\n");	
	    keep_alive = 0;
	}

	  /*Configure settings in address struct*/
	  serverAddr.sin_family = AF_INET;
	  serverAddr.sin_port = htons(server_port);
	  serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	  /*Bind socket with address struct*/
	  int binding = bind(udpSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));
	    if(binding<0){
		printf("Binding doesnt worked\n");
		keep_alive = 0;
	    }
	  
	  
	  /*Initialize size variable to be used later on*/
	  addr_size = sizeof serverStorage;
	    
	  buffer = malloc(sizeof(char)*65536);
	    
	  if(keep_alive){
		printf("Server starting: %s:%i \n\n",server_adress,server_port);  	  
	  }
	  while(keep_alive){
		  
	    /* Try to receive any incoming UDP datagram. Address and port of 
	      requesting client will be stored on serverStorage variable */
	    data_length = recvfrom(udpSocket,buffer,65536,0,(struct sockaddr *)&serverStorage, &addr_size);
	    
	    if(data_length>=0){	  
		    gpgCheckSign();
	    }
	    else{
		if(keep_alive!=0){ //falls kein selbstgewollter Abbruch
			printf("Recieve From Error\n");
		}
		break;
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
