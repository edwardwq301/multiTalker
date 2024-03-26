main: server client
server: server.cpp util.cpp
	g++ $^ -o server 

client: client.cpp util.cpp
	g++ $^ -o client

.PHONY: clean

clean: 
	rm client server