#Makefile#

CC=gcc
all: client

client: client.c
	$(CC) client.c -lcrypto -lm -o client
client.o: client.c
	$(CC) client.c -I math.h -c -o client.o

clean:
	rm -f client	
