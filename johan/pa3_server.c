#include "pa3_server.h"

int main(int argc, char** argv) {

    int res;

    //strg + c handler
    struct sigaction handler;

    handler.sa_handler = destroy;
    sigemptyset(&handler.sa_mask);
    handler.sa_flags = 0;
    sigaction(SIGINT, &handler, NULL);


    //checking number of arguments
    if (argc != 2) {
        printf("\nwrong number of arguments\n");
        print_usage();
        return -1;
    }

    //looks if port is number
    for (i = 0; i < strlen(argv[1]); i++) {
        if (argv[1][i] < 48 || argv[1][i] > 57) {
            printf("\nthe argument is not a number\n");
            return -1;
        }
    }

    //create udp socket
    udp_socket = socket(PF_INET, SOCK_DGRAM, 0);

    //checking socket
    if(udp_socket < 0){
        printf("\nfailed to init socket\n");
        close(udp_socket);
        return -1;
    }

    //checking inet address
    if(inet_addr("127.0.0.1") < 0){
        printf("\nfailed to convert inet_addr\n");
        close(udp_socket);
        return -1;
    }


    //some server initializing
    server_add.sin_family = AF_INET;
    server_add.sin_port = htons(atoi(argv[1]));
    server_add.sin_addr.s_addr = inet_addr("127.0.0.1");
    memset(server_add.sin_zero, '\0', sizeof server_add.sin_zero);

    //try to bind
    res = bind(udp_socket, (struct sockaddr *) &server_add, sizeof(server_add));

    //looking for errors in binding
    if(res < 0){
        printf("\nfailed to bind socket\n");
        close(udp_socket);
        return -1;
    }


    addr_size = sizeof storage;

    //trying to init gpg stuff
    if(init_gpg() < 0 ){
        close(udp_socket);
        return -1;
    }


    while (no_error) {
    //receiving message
        bytes = recvfrom(
            udp_socket,
            buffer,
            65536,
            0,
            (struct sockaddr *) &storage,
            &addr_size);

        //something went wrong if that happens
        if(bytes < 0){
            printf("\nfailed to receive package\n");
            close(udp_socket);
            gpgme_release(ctx);
            return -1;
        }

        //varyifing message error handling
        if(verify_message() < 0){
            printf("\nfailed to verify message\n");
            close(udp_socket);
            return -1;
        }

        printf("\n");
    }

    gpgme_release(ctx);
    //close udp_socket
    close(udp_socket);
    return 0;
}

int print_usage(){
    printf("\n### please use the following syntax");
    printf("\n### ./pa3_server SERVER_PORT");
    printf("\n### now try again\n\n");

    return 0;
}

int init_gpg(){

    //first get the gpg version
    gpgme_check_version(NULL);
    setlocale(LC_ALL, "");
    gpgme_set_locale(NULL, LC_CTYPE, setlocale(LC_CTYPE, NULL));



    //creates a new gpgme_ctx_t object if check_version was already called
    error = gpgme_new(&ctx);

    //gpg error handling
    if(error){
        printf("\nfailed to create gpg context");
        gpgme_release(ctx);
        return -1;
    }

    return 0;

}

int verify_message(){

    //initialize gpg stuff
    gpgme_data_t data;
    gpgme_data_t buf_data;
    gpgme_verify_result_t result;
    gpgme_key_t key;
    size_t length= 0;

    //get data
    error = gpgme_data_new_from_mem(&data, buffer, bytes, 1);

    //gpg error handling
    if(error){
        printf("\nfailes to read message");
        gpgme_release(ctx);
        gpgme_free(data);
        return -1;
    }

    //creates a new gpgme_data_t object its initially empty
    error = gpgme_data_new(&buf_data);


    //gpg error handling
    if(error){
        printf("\nfailed to initialize data");
        gpgme_release(ctx);
        gpgme_free(data);
        gpgme_free(buf_data);
        return -1;
    }

    //verify message
    error = gpgme_op_verify(ctx, data, NULL, buf_data);

    //gpg error handling
    if(error){
        printf("\nfailed to verify");
        gpgme_release(ctx);
        gpgme_free(data);
        gpgme_free(buf_data);
        return -1;
    }

    //get results of verification
    result = gpgme_op_verify_result(ctx);

    //gpg error handling
    if(!result){
        printf("failed to get result");
        gpgme_release(ctx);
        gpgme_free(data);
        gpgme_free(buf_data);
        return -1;

    }

    if(result->signatures->summary == GPGME_SIGSUM_VALID + GPGME_SIGSUM_GREEN){

        //malloc memory
        received = malloc(sizeof(char)*65536);

        //go to the beginning
        error = gpgme_data_seek(buf_data,0,SEEK_SET);

        //gpg error handling
        if(error){
            printf("\nfailed to point at beginning");
            gpgme_release(ctx);
            gpgme_free(data);
            gpgme_free(buf_data);
            return -1;
        }

        //get plain text and release buf_data
        received = gpgme_data_release_and_get_mem(buf_data, &length);

        printf("\n");

        //print the text
        for (i = 0; i < length; i++) {
            printf("%c", received[i]);
        }

        //free received
        free(received);

        //get key
        error = gpgme_get_key(ctx, result->signatures->fpr, &key, 0);

        //gpg error handling
        if(error){
            printf("\nfailed to get key\n");
            gpgme_release(ctx);
            gpgme_key_release(key);
            gpgme_free(data);
            return 0;
        }

        //print name
        printf("\n%s",key->uids->name);

        //release key
        gpgme_key_release(key);

    }
    else{
        printf("\nsignature is invalid\n");
    }

    //release everything
    gpgme_free(data);

    return 0;
}

//just terminates the while loop
void destroy(){
    no_error = 0;
}
