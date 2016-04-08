/*
 * server.h
 *
 *  Created on: 26 февр. 2016 г.
 *      Author: SonyaHon
 */

#ifndef CLASSES_SERVER_H_
#define CLASSES_SERVER_H_
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <pthread.h>
#include <iostream>
#include <vector>
#include <map>
#include <queue>

class server {
private:
	struct sockaddr_in addr;
	struct sockaddr_in client_addr;
public:
	server(int portNumber);
	void start();
	void close();
	static int parser(std::string str);
	std::string parserName(std::string);
};

#endif /* CLASSES_SERVER_H_ */
