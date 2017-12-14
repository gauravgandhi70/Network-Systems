#Makefile#

CC=gcc
all: webproxy

webproxy: webproxy.c
	$(CC) webproxy.c -lcrypto -lm -o webproxy

clean:
	rm -f webproxy
