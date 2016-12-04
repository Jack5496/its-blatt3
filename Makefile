binaries = pa3_server pa3_client
flags=$(shell gpgme-config --libs --cflags)

all: pa3_server pa3_client

pa3_server: pa3_server.c
	gcc -o pa3_server pa3_server.c -Wall -Werror
	
pa3_client: pa3_client.c
	$(CC) -o pa3_client ${flags} pa3_client.c -Wall -Werror

clean: 
	rm -f $(binaries) *.o
