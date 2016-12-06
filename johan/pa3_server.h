//include stuff
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <gpgme.h>
#include <errno.h>
#include <locale.h>
#include <arpa/inet.h>

#ifndef PA3_SERVER_H
#define PA3_SERVER_H

//function declaring
int print_usage();
int verify_message();
int init_gpg();
void destroy();

//some server stuff
int udp_socket;
int bytes;
char buffer[65536];
struct sockaddr_in server_add;
struct sockaddr_storage storage;
socklen_t addr_size;
int i;
int no_error = 1;

//gpg stuff
char* received;
gpgme_ctx_t ctx;
gpgme_error_t error;

#endif
        
