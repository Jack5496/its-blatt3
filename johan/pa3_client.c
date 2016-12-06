#include "pa3_client.h"

int main(int argc, char** argv){

    //looking if enough arguments
    if(argc != 5){
        printf("\nwrong number of arguments\n");
	    print_usage();
	    return -1;
    }

    //looks if port is a number
    for(i=0; i < strlen(argv[2]); i++){
        if(argv[2][i] < 48 || argv[2][i] >57){
	        printf("\nthe port is not a number\n");
	        return -1;
	    }
    }

    //init socket
    client_socket = socket(PF_INET, SOCK_DGRAM, 0);

    //socket error handling
    if(client_socket < 0){
        printf("\nfailed to open socket");
        close(client_socket);
        return -1;
    }

    //inet address handling
    if(inet_addr(argv[1]) < 0){
        printf("\nfailed to use ip adress\n");
        close(client_socket);
        return -1;
    }

    server_add.sin_family =  AF_INET;
    server_add.sin_port = htons(atoi(argv[2]));
    server_add.sin_addr.s_addr = inet_addr(argv[1]);
    memset(server_add.sin_zero, '\0', sizeof(server_add.sin_zero));

    addr_size = sizeof(server_add);

    //init gpg
    gpgme_check_version(NULL);
    setlocale(LC_ALL, "");
    gpgme_set_locale(NULL, LC_CTYPE, setlocale(LC_CTYPE, NULL));

    //creates a new gpgme_ctx_t object
    error = gpgme_new(&ctx);

    //gpg error handling
    if(error){
        printf("\nfailed to create gpg context\n");
        gpgme_release(ctx);
        close(client_socket);
        return -1;
    }

    //searches for key
    error = gpgme_op_keylist_start(ctx, argv[3], 0);

    //gpg error handling
    if(error){
        printf("\nfailed to read key for user %s\n",argv[3]);
        gpgme_release(ctx);
        close(client_socket);
        return -1;
    }

    //gets the key (only way in gpgme  to get a key)
    error = gpgme_op_keylist_next(ctx, &key);

    //gpg error handling
    if(error){
        printf("\nfailed to get key\n");
        gpgme_release(ctx);
        gpgme_key_release(key);
        close(client_socket);
        return -1;
    }

    //gets the key (only way in gpgme  to get a key)
    error = gpgme_op_keylist_end(ctx);

    //gpg error handling
    if(error){
        printf("\nfailed to close list\n");
        gpgme_release(ctx);
        gpgme_key_release(key);
        close(client_socket);
        return -1;
    }

    //gets data
    error = gpgme_data_new_from_mem(&data, argv[4], strlen(argv[4]), 1);

    //gpg error handling
    if(error){
        printf("\nfailes to read message\n");
        gpgme_release(ctx);
        gpgme_key_release(key);
        gpgme_free(data);
        close(client_socket);
        return -1;
    }

    //creates a new gpgme_data_t object its initially empty
    error = gpgme_data_new(&signed_data);

    //gpg error handling
    if(error){
        printf("\nfailed to initialize data\n");
        gpgme_release(ctx);
        gpgme_key_release(key);
        gpgme_free(data);
        close(client_socket);
        return -1;
    }

    //add signature
    error = gpgme_signers_add(ctx, key);

    //gpg error handling
    if(error){
        printf("\nfailed to add key\n");
        gpgme_release(ctx);
        gpgme_key_release(key);
        gpgme_free(data);
        gpgme_free(signed_data);
        close(client_socket);
        return -1;
    }

    //sign message
    error = gpgme_op_sign(ctx, data, signed_data, GPGME_SIG_MODE_CLEAR);

    //gpg error handling
    if(error){
        printf("\nfailed to sign\n");
        gpgme_release(ctx);
        gpgme_key_release(key);
        gpgme_free(data);
        gpgme_free(signed_data);
        close(client_socket);
        return -1;
    }

    //allocate memory
    to_send = malloc(sizeof(char)*65536);

    //gpg error handling
    to_send = gpgme_data_release_and_get_mem(signed_data, &bytes);

    //send message to server
    sendto( client_socket,
            to_send,
            bytes,
            0,
            (struct sockaddr*) &server_add,
            addr_size);

    //free memory
    free(to_send);

    //release gpg stuff
    gpgme_release(ctx);
    gpgme_key_release(key);
    gpgme_free(data);

    //close socket
    close(client_socket);
    
    return 0;
}

//prints usage
int print_usage(){
    printf("\n### please use the following syntax");
    printf("\n### ./pa3_client SERVER_ADRESS SERVER_PORT USERNAME \"MESSAGE\"");
    printf("\n### now try again\n\n");

    return 0;
}
