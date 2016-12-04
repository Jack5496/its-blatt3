binaries = pa3_server pa3_client

all: pa3_server pa3_client

pa3_server: pa3_server.c
	gcc -o pa3_server pa3_server.c -Wall -Werror
	
pa3_client: pa3_client.c
	$(CC) -o pa3_client pa3_client.c -Wall -Werror -pgpme-dev

clean: 
	rm -f $(binaries) *.o
