
#include "headers/server.h"

int main (int argc, char** argv) {

	int nomport = 1488;

	if(argc > 1) {
		nomport = atoi(argv[1]);
	}
	else {
		puts("Server is starting with default port 1488. If u want another pls restart it with port number in arguments.");
	}

	server _server = server(nomport);
	_server.start();

	return 0;
}
