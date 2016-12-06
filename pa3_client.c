#include <stdlib.h> /* malloc */
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>

#include <gpgme.h>   /* gpgme             */
#include <unistd.h>  /* write             */
#include <errno.h>   /* errno             */
#include <locale.h>  /* locale support    */

char server_adress[65536]; /* Platz für Server Adresse */
int server_port = 80;  /* Server Port */
char username[65536];  /* Username */
char message[65536];  /* Unsere Plain Message */
char* signature;  /* Unsere spätere Signatur */
char* signaturePointer; /* Pointer zum anfang unserer Signatur */
size_t signature_length;  /* Länger dieser Signatur */

void signText(char* signaturePointer){
     /* GPG wird hier nicht extra initialisiert, da wir eh nur einen Aufruf starten */

    gpgme_ctx_t ctx;  /* GPG Context */
    gpgme_error_t err;  /* GPG Errors */
    gpgme_data_t in, out;  /* Data Input in, Data Output out */
    gpgme_sig_mode_t sigMode = GPGME_SIG_MODE_CLEAR;  /* Setze den Modus auf Clear --> keine Kompression */

    /* Begin setup of GPGME */
    gpgme_check_version (NULL);  /* Entnommen aus der Doku */
    setlocale (LC_ALL, "");
    gpgme_set_locale (NULL, LC_CTYPE, setlocale (LC_CTYPE, NULL));

    err = gpgme_engine_check_version (GPGME_PROTOCOL_GPGCONF);
    if(err){
        printf("GPGME: Engine check failed\n");
        return;
    }

    /* Erstelle GPG Context */
    err = gpgme_new (&ctx);
    if(err){
        printf("GPGME: Context init failed\n");
        return;
    }

    /* Hole Länge unserer Nachricht */
    unsigned int textLength = strlen(message)+1;

    /* Erstelle Data Objekt um unsere Nachricht zu halten */
    err = gpgme_data_new_from_mem (&in, message, textLength, 0);
    if(err){
        printf("GPGME: Data assign failed\n");
        gpgme_data_release (out);
        /* Release CTX */
        gpgme_release (ctx);
        return;
    }
     
    gpgme_key_t key;  /* Erstelle neuen halter unseres Keys */

    /* Öffne KeyList um richtigen Key zu finden */
    err = gpgme_op_keylist_start(ctx, username, 0);
    if(err){
        printf("GPGME: Start Keylist failed\n");
        /* Release Input */
        gpgme_data_release (in);
        /* Release CTX */
        gpgme_release (ctx);
        return;
    }

    /* Nehme den ersten passenden Key zum Namen */
    err = gpgme_op_keylist_next(ctx, &key);
    if(err){
        printf("GPGME: Next key finding Failed, Ambigeos name ?\n");
        /* Release Input */
        gpgme_data_release (in);
        /* Release CTX */
        gpgme_release (ctx);
        gpgme_key_release (key); /* Release den Key */
        return;
    }

    /* Beende Keylist */
    err = gpgme_op_keylist_end(ctx);
    if(err){
        printf("GPGME: Keylist closing failed\n");
        /* Release Input */
        gpgme_data_release (in);
        /* Release CTX */
        gpgme_release (ctx);
        gpgme_key_release (key); /* Release den Key */
        return;
    }

    /* Füge key zum Context hinzu */
    err = gpgme_signers_add(ctx, key);
    if(err){
        printf("GPGME: Key adding failed\n");
        /* Release Input */
        gpgme_data_release (in);
        /* Release CTX */
        gpgme_release (ctx);
        gpgme_key_release (key); /* Release den Key */
        return;
    }

    /* Erstelle Output Objekt */
    err = gpgme_data_new (&out);
    if(err){
        printf("GPGME: Data assign failed\n");
        /* Release Input */
        gpgme_data_release (in);
        /* Release CTX */
        gpgme_release (ctx);
        return;
    }

    /* Signiere den Inhalt mit dem Modus und packe dies in out */
    err = gpgme_op_sign (ctx, in, out, sigMode);
    if(err){
        printf("GPGME: Signation failed\n");
        /* Release Input */
        gpgme_data_release (in);
        /* Release Output */
        gpgme_data_release (out);
        /* Release CTX */
        gpgme_release (ctx);
        gpgme_key_release (key); /* Release den Key */
        return;
    }

    gpgme_key_release (key); /* Release den Key */

    /* Sinatur länge halter */
    signature_length = 0;

    gpgme_data_seek(out,0,SEEK_SET); /* Setze Pointer auf den Anfang */

    /* Hole die Signatur aus out mit der länge und release out */
    signaturePointer = gpgme_data_release_and_get_mem(out,&signature_length);
     
     /* Release Input */
    gpgme_data_release (in);
    /* Release CTX */
    gpgme_release (ctx);
}

int main(int argc, char **argv){
    struct sockaddr_in serverAddr;
    int clientSocket;
    socklen_t addr_size;
    int i;
    
    int needed_arguments = 1; //programm selber
    needed_arguments++; //Server Adress
    needed_arguments++; //Server Port
    needed_arguments++; //UserName
    needed_arguments++; //From Here up the Message
 
    if(argc==needed_arguments){
        int err_adr = inet_aton(argv[1],&serverAddr.sin_addr);
        if(err_adr==0) {
            printf("Error: No valid Server IP\n");
            exit(1);
        }
        
        int err_port = 0;
        i=0;
        while(argv[2][i] != '\0'){
             if (argv[2][i] < 47 || argv[2][i] > 57){
                err_port = 1;
                break;
            }
            i++;
        }
        if(err_port==1){
            printf("Error: No valid Port\n");
            exit(1);
        }        
        server_port = atoi(argv[2]);
        
        strncpy(username, argv[3], sizeof username);
        strncpy(message, argv[4], sizeof message);
     
     //Kleiner UDP Client http://www.programminglogic.com/sockets-programming-in-c-using-udp-datagrams/
     
     

     /*Create UDP socket*/
     clientSocket = socket(AF_INET, SOCK_DGRAM, 0);

     /*Configure settings in address struct*/
     serverAddr.sin_family = AF_INET;
     serverAddr.sin_port = htons(server_port);

     /*Initialize size variable to be used later on*/
     addr_size = sizeof serverAddr;

     signature = malloc(sizeof(char)*65536);
     signaturePointer = signature;
     /*Sign message*/
     signText(signaturePointer);
     for(i=0;i<signature_length;i++){
         printf("%c",signature[i]);
     }
     /*Send message to server*/
     sendto(clientSocket,signaturePointer,signature_length,0,(struct sockaddr *)&serverAddr,addr_size);
        
     free(signature);
         
     printf("Signatrue Length: %i\n",(int)signature_length);
        
    }
    else{
        printf("usage: ./pa3_client SERVER_ADRESS PORT USERNAME \"Message\" \n");
    }
 
    return 0;
}

