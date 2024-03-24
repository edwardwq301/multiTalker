

all: client.cpp util.cpp server.cpp 
	g++ server.cpp util.cpp -g -o server
	g++ client.cpp util.cpp -g -o client

clean: 
	rm client server