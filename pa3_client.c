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

#define SIZE 1024

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

     /*Send message to server*/
     sendto(clientSocket,message,nBytes,0,(struct sockaddr *)&serverAddr,addr_size);
     
     
     
     
     
     
     
     
     
     
  gpgme_error_t error;
  gpgme_engine_info_t info;
  gpgme_ctx_t context;
  gpgme_key_t recipients[2] = {NULL, NULL};
  gpgme_data_t clear_text, encrypted_text;
  gpgme_encrypt_result_t  result;
  gpgme_user_id_t user;
  char *buffer;
  ssize_t nbytes;

  /* Initializes gpgme */
  gpgme_check_version (NULL);
  
  /* Initialize the locale environment.  */
  setlocale (LC_ALL, "");
  gpgme_set_locale (NULL, LC_CTYPE, setlocale (LC_CTYPE, NULL));
#ifdef LC_MESSAGES
  gpgme_set_locale (NULL, LC_MESSAGES, setlocale (LC_MESSAGES, NULL));
#endif

  error = gpgme_new(&context);
  fail_if_err(error);
  /* Setting the output type must be done at the beginning */
  gpgme_set_armor(context, 1);

  /* Check OpenPGP */
  error = gpgme_engine_check_version(GPGME_PROTOCOL_OpenPGP);
  fail_if_err(error);
  error = gpgme_get_engine_info (&info);
  fail_if_err(error);
  while (info && info->protocol != gpgme_get_protocol (context)) {
    info = info->next;
  }  
  /* TODO: we should test there *is* a suitable protocol */
  fprintf (stderr, "Engine OpenPGP %s is installed at %s\n", info->version,
	   info->file_name); /* And not "path" as the documentation says */

  /* Initializes the context */
  error = gpgme_ctx_set_engine_info (context, GPGME_PROTOCOL_OpenPGP, NULL,
				     KEYRING_DIR);
  fail_if_err(error);

  error = gpgme_op_keylist_start(context, "John Smith", 1);
  fail_if_err(error);
  error = gpgme_op_keylist_next(context, &recipients[0]);
  fail_if_err(error);
  error = gpgme_op_keylist_end(context);
  fail_if_err(error);

  user = recipients[0]->uids;
  printf("Encrypting for %s <%s>\n", user->name, user->email);

  /* Prepare the data buffers */
  error = gpgme_data_new_from_mem(&clear_text, SENTENCE, strlen(SENTENCE), 1);
  fail_if_err(error);
  error = gpgme_data_new(&encrypted_text);
  fail_if_err(error); 

  /* Encrypt */
  error = gpgme_op_encrypt(context, recipients, 
			   GPGME_ENCRYPT_ALWAYS_TRUST, clear_text, encrypted_text);
  fail_if_err(error);
  result = gpgme_op_encrypt_result(context);
  if (result->invalid_recipients)
    {
      fprintf (stderr, "Invalid recipient found: %s\n",
	       result->invalid_recipients->fpr);
      exit (1);
    }

  nbytes = gpgme_data_seek (encrypted_text, 0, SEEK_SET);
  if (nbytes == -1) {
    fprintf (stderr, "%s:%d: Error in data seek: ",			
	     __FILE__, __LINE__);
    perror("");
    exit (1);					
    }  
  buffer = malloc(MAXLEN);
  nbytes = gpgme_data_read(encrypted_text, buffer, MAXLEN);
  if (nbytes == -1) {
    fprintf (stderr, "%s:%d: %s\n",			
	     __FILE__, __LINE__, "Error in data read");
    exit (1);					
  }
  buffer[nbytes] = '\0';
  printf("Encrypted text (%i bytes):\n%s\n", (int)nbytes, buffer);
  /* OK */
     
     
     
     
     
     
        
    }
    else{
        printf("usage: ./pa3_client SERVER_ADRESS SERVER_PORT USERNAME \"Message to encypt\" \n");
    }
 
    return 0;
}

