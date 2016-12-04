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
     
     
     
     
     
     
     
     
     
     
     
     
     
     
     char *p;
   char buf[SIZE];
   size_t read_bytes;
   int tmp;
   gpgme_ctx_t ceofcontext;
   gpgme_error_t err;
   gpgme_data_t data;

   gpgme_engine_info_t enginfo;

   /* The function `gpgme_check_version' must be called before any other
    * function in the library, because it initializes the thread support
    * subsystem in GPGME. (from the info page) */
   setlocale (LC_ALL, "");
   p = (char *) gpgme_check_version(NULL);
   printf("version=%s\n",p);
   /* set locale, because tests do also */
   gpgme_set_locale(NULL, LC_CTYPE, setlocale (LC_CTYPE, NULL));

   /* check for OpenPGP support */
   err = gpgme_engine_check_version(GPGME_PROTOCOL_OpenPGP);
   if(err != GPG_ERR_NO_ERROR) return 1;

   p = (char *) gpgme_get_protocol_name(GPGME_PROTOCOL_OpenPGP);
   printf("Protocol name: %s\n",p);

   /* get engine information */
   err = gpgme_get_engine_info(&enginfo);
   if(err != GPG_ERR_NO_ERROR) return 2;
   printf("file=%s, home=%s\n",enginfo->file_name,enginfo->home_dir);

   /* create our own context */
   err = gpgme_new(&ceofcontext);
   if(err != GPG_ERR_NO_ERROR) return 3;

   /* set protocol to use in our context */
   err = gpgme_set_protocol(ceofcontext,GPGME_PROTOCOL_OpenPGP);
   if(err != GPG_ERR_NO_ERROR) return 4;

   /* set engine info in our context; I changed it for ceof like this:

   err = gpgme_ctx_set_engine_info (ceofcontext, GPGME_PROTOCOL_OpenPGP,
               "/usr/bin/gpg","/home/user/nico/.ceof/gpg/");

      but I'll use standard values for this example: */

   err = gpgme_ctx_set_engine_info (ceofcontext, GPGME_PROTOCOL_OpenPGP,
               enginfo->file_name,enginfo->home_dir);
   if(err != GPG_ERR_NO_ERROR) return 5;

   /* do ascii armor data, so output is readable in console */
   gpgme_set_armor(ceofcontext, 1);

   /* create buffer for data exchange with gpgme*/
   err = gpgme_data_new(&data);
   if(err != GPG_ERR_NO_ERROR) return 6;

   /* set encoding for the buffer... */
   err = gpgme_data_set_encoding(data,GPGME_DATA_ENCODING_ARMOR);
   if(err != GPG_ERR_NO_ERROR) return 7;

   /* verify encoding: not really needed */
   tmp = gpgme_data_get_encoding(data);
   if(tmp == GPGME_DATA_ENCODING_ARMOR) {
      printf("encode ok\n");
   } else {
      printf("encode broken\n");
   }

   /* with NULL it exports all public keys */
   err = gpgme_op_export(ceofcontext,NULL,0,data);
   if(err != GPG_ERR_NO_ERROR) return 8;

   read_bytes = gpgme_data_seek (data, 0, SEEK_END);
   printf("end is=%d\n",read_bytes);
   if(read_bytes == -1) {
      p = (char *) gpgme_strerror(errno);
      printf("data-seek-err: %s\n",p);
      return 9;
   }
   read_bytes = gpgme_data_seek (data, 0, SEEK_SET);
   printf("start is=%d (should be 0)\n",read_bytes);

   /* write keys to stderr */
   while ((read_bytes = gpgme_data_read (data, buf, SIZE)) > 0) {
      write(2,buf,read_bytes);
   }
   /* append \n, so that there is really a line feed */
   write(2,"\n",1);

   /* free data */
   gpgme_data_release(data);

   /* free context */
   gpgme_release(ceofcontext);
     
     
     
     
     
     
     
     
        
    }
    else{
        printf("usage: ./pa3_client SERVER_ADRESS SERVER_PORT USERNAME \"Message to encypt\" \n");
    }
 
    return 0;
}

