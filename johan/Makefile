all : pa3_client.c pa3_server.c
	gcc pa3_client.c -o pa3_client -lgpgme -Wall
	gcc pa3_server.c -o pa3_server -lgpgme -Wall
s_test :
	./pa3_server 4567
c_test :
	./pa3_client 127.0.0.1 4567 Johann "Hello dude"
clean :
	rm pa3_client pa3_server
