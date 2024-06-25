CC = g++
CFLAGS = -Wall

all: server client

server: server.cpp
	$(CC) $< $(CFLAGS) -o $@ 

client: client.cpp
	$(CC) $< $(CFLAGS) -o $@

.PHONY: clean
clean:
	rm -f server client
