#include <stdlib.h>    //malloc
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>

#include <gpgme.h>   /* gpgme             */
#include <unistd.h>  /* write             */
#include <errno.h>   /* errno             */
#include <locale.h>  /* locale support    */

char server_adress[65536];
int server_port = 80;
char username[65536];
char message[65536];
int debug = 1;

char private_key[65536];










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

void signText();

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
     
     //Kleiner UDP Client http://www.programminglogic.com/sockets-programming-in-c-using-udp-datagrams/
     
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

     nBytes = strlen(message) + 1;

     char* signature = malloc(sizeof(char)*65536);
     /*Sign message*/
     signText();
     
     /*Send message to server*/
     sendto(clientSocket,message,nBytes,0,(struct sockaddr *)&serverAddr,addr_size);
        
     free(signature);
        
    }
    else{
        printf("usage: ./pa3_client SERVER_ADRESS SERVER_PORT USERNAME \"Message to encypt\" \n");
    }
 
    return 0;
}

void signText(){
    gpgme_ctx_t ctx;
    gpgme_error_t err;
     gpgme_data_t in, out, result;
    /* Set the GPGME signature mode
        GPGME_SIG_MODE_NORMAL : Signature with data
        GPGME_SIG_MODE_CLEAR  : Clear signed text
        GPGME_SIG_MODE_DETACH : Detached signature */
    gpgme_sig_mode_t sigMode = GPGME_SIG_MODE_CLEAR;

    /* Begin setup of GPGME */
    gpgme_check_version (NULL);
    setlocale (LC_ALL, "");
    gpgme_set_locale (NULL, LC_CTYPE, setlocale (LC_CTYPE, NULL));
    
    err = gpgme_engine_check_version (GPGME_PROTOCOL_GPGCONF);
    fail_if_err (err);
    
    // Create the GPGME Context
    err = gpgme_new (&ctx);
    // Error handling
    fail_if_err (err);
    
    unsigned int textLength = strlen(message) + 1;
    
    // Create a data object that contains the text to sign
    fail_if_err (gpgme_data_new_from_mem (&in, message, textLength, 0));

    // Create a data object pointing to the out buffer
    fail_if_err (gpgme_data_new (&out));

    // Create a data object pointing to the result buffer
    fail_if_err (gpgme_data_new (&result));

    // Sign the contents of "in" using the defined mode and place it into "out"
    fail_if_err (gpgme_op_sign (ctx, in, out, sigMode));

    size_t signature_length = 0;
    
    signature = gpgme_release_and_get_mem(out,&signature_length);
    
    
    // Release the "in" data object
    gpgme_data_release (in);
    // Release the context
    gpgme_release (ctx);
}
