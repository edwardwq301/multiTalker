

all: client.cpp util.cpp server.cpp 
	g++ server.cpp util.cpp -o server
	g++ client.cpp util.cpp -o client

clean: 
	rm client server