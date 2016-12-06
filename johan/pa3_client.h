//include stuff
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <gpgme.h>
#include <arpa/inet.h>
#include <errno.h>
#include <locale.h>
#include <unistd.h>

#ifndef PA3_CLIENT_H
#define PA3_CLIENT_H

//usage printing function
int print_usage();

//udp server stuff
int client_socket;
size_t bytes = 0;
char buffer[65536];
struct sockaddr_in server_add;
socklen_t addr_size;
int i;

//gpg stuff
char* to_send;
char* to_send_pointer;
gpgme_ctx_t ctx;
gpgme_error_t error;
gpgme_key_t key;
gpgme_data_t data;
gpgme_data_t signed_data;

#endif
        
