all: server client

server: server.c index.c index.h
	gcc -Wall -O2 -g -o server server.c index.c

client: client.c
	gcc -Wall -O2 -g -o client client.c
