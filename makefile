CC = g++
CFLAGS = -Wall

all: server client

server: server.cpp util.h
	$(CC) $< $(CFLAGS) -o $@ 

client: client.cpp util.h
	$(CC) $< $(CFLAGS) -o $@

.PHONY: clean
clean:
	rm -f server client
