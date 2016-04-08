#include "../headers/server.h"
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
#include <arpa/inet.h>
#include <fstream>

bool shallAccept;
int sockMain;

void sendall(int s, std::string str, int len);
void *listening(void *sock);
void *writing(void *);
void *serverAdmins(void *);
std::string ntoa(struct sockaddr_in a);

std::string adminsCommands[5];

std::queue<std::string> messages; //Queue of all messages, which should be sent
std::map<int, std::string> sockets; // All connected users are here
std::map<std::string, std::string> addresses; // Users addresses
std::vector<pthread_t> threads;


server::server(int portNumber) {
	shallAccept = true;
	sockMain = socket(AF_INET, SOCK_STREAM, 0);
	if(sockMain < 0) {
		perror("socket error: ");
		exit(0);
	}

	int foo = 1;
	setsockopt(sockMain, SOL_SOCKET, SO_REUSEADDR, (void*)&foo, (socklen_t)sizeof(foo));

	addr.sin_family = AF_INET;
	addr.sin_port = htons(portNumber);
	addr.sin_addr.s_addr = INADDR_ANY;

	adminsCommands[0] = "help";
	adminsCommands[1] = "kick";
	adminsCommands[2] = "list";
	adminsCommands[3] = "ban";
	adminsCommands[4] = "exit";

	if(bind(sockMain, (struct sockaddr*) &addr, sizeof(addr))) {
		perror("Error on binding.");
		exit(1);
	}
}

void server::start() {


	listen(sockMain, 5);
	std::cout << "Server is started." << std::endl;

	int clientSock;
	unsigned int clientLenth = sizeof(client_addr);

	pthread_t tempP;

	pthread_create(&tempP, NULL, writing, NULL);
	//threads.reserve(threads.size() + 1);
	threads.push_back(tempP);

	pthread_create(&tempP, NULL, serverAdmins, NULL);
	//threads.reserve(threads.size() + 1);
	threads.push_back(tempP);

	while(shallAccept) {

	std::ifstream bans("ban_list.txt");
	if(bans == NULL) {
		system("echo >> ban_list.txt");
		bans.close();
		std::ifstream bans("ban_list.txt");
	}

	if(shallAccept)
		clientSock = accept(sockMain, (struct sockaddr*) &client_addr, &clientLenth);
	if(clientSock < 0 && shallAccept == false) {
		puts("Server is shuting down never mind");
		break;
	}
	else if(clientSock < 0) {
		perror("Client is a NIGGER: ");
		exit(3);
	}

	std::string ip;
	bool banned  = false;
	while(bans >> ip) {

		if(ip == ntoa(client_addr)) {
			puts("This client is in ban list, kicking him.");
			banned = true;
			::close(clientSock);
		}
	}

	bans.close();

	if(!banned) {
		puts("Client connected.");
		char buffer[256];
		bzero(buffer, 256);
		read(clientSock, buffer, sizeof(buffer));
		std::cout << "Client " << buffer << " with socket " << clientSock << " is connected" << "\n";
		sockets[clientSock] = buffer;

		addresses[sockets[clientSock]] = ntoa(client_addr);

		std::string socks;
		socks += "&";

		for(auto it = sockets.begin(); it != sockets.end(); ++it) {
			socks += (*it).second;
			socks += "\n*";
		}
		for(auto it = sockets.begin(); it != sockets.end(); ++it) {
			sendall((*it).first, socks, socks.size());
		}
		pthread_t tempP;
		pthread_create(&tempP, NULL, listening, (void *)clientSock);
		//threads.reserve(threads.size() + 1);
		threads.push_back(tempP);
		}
	}

}

int server::parser(std::string str) {
	int i = 0;
	std::string name = "";

	for(; i < str.size(); ++i) {
		if(str[i] == '/') break;
	}
	if(str[i] == '/') {
		++i;
		for(; i < str.size(); ++i) {
			name += str[i];
			if(str[i + 1] == ' ') break;
		}
	}
	else {
		return -1;
	}

	int result = - 2;
	for(auto it = sockets.begin(); it != sockets.end(); ++it) {
		if((*it).second == name) {
			result = (*it).first;
		}
	}
	return result;
}

std::string server::parserName(std::string str) {
	std::string name;
	for(int i = 0; i < str.size(); ++i) {
		name += str[i];
		if(str[i + 1] == ':') break;
	}
	return name;
}

std::string deleteWhisper(std::string str, std::string name) {

	int i = 0;
	for(; i < str.size(); ++i) {
		if(str[i] == '/') break;
	}
	int z = i;
	for(; z < str.size(); ++z) {
		if(str[z] == ' ') break;
	}
	str.replace(str.begin() + i, str.begin() + z, "(Whispers)");
	return str;
}

void sendall(int s, std::string str, int len) {
	char buff[len + 1];

	for(int z = 0; z <= len; ++z) {
		buff[z] = str[z];
	}

	std::cout << buff << " " << len << std::endl;
	int i = 0;
	for(; i < 256; ++i) {
		char subBuff[1];
		subBuff[0] = buff[i];
		int n = write(s, subBuff, 1);
		if(buff[i] == '\0') break;
		if(n <= 0) {
			puts("Smthign went wrong: ");
			break;
		}
		bzero(subBuff, 1);
	}
}

void *listening(void *sock) {
	char buffer[1];
	while(true) {
		std::string str;
		str.clear();
		while(true) {
			int n = read((long)sock, buffer, 1);
			if(buffer[0] == '\0') break;
			if(n <= 0) break;
			str += buffer[0];
 		}
		if(!str.empty()) {
			//str.erase(str.end() - 1);
			messages.push(sockets.find((long)sock)->second + ": " + str);
		}
		else if(str.empty()) {
			break;
		}
		str.clear();
		bzero(buffer, 1);
	}
	sockets.erase(sockets.find((long)sock));
	close((long)sock);
	pthread_exit(NULL);
}

void *writing(void *) {
	while(true) {
		if(!messages.empty()) {
			std::string str = messages.front();
			int newSock = server::parser(str);
			switch(newSock) {
			case -1:

					for(auto it = sockets.begin(); it != sockets.end(); ++it) {
						sendall((*it).first, str, str.size());
					}
				break;
			case -2:
				puts("No client with this nickname.");
				break;
			default:
				str = deleteWhisper(str, "");
				sendall(newSock, str, str.size());
				break;
			}
			messages.pop();
		}
		else {
			continue;
		}
	}
	pthread_exit(NULL);
}

void *serverAdmins(void *) {

	std::string command;

	while(true) {
		std::cin >> command;

		if(command == adminsCommands[0]) { // Help
		std::cout << adminsCommands[0] << " Help command, shows all avalible commands and their short description." << std::endl;
		std::cout << adminsCommands[1] << " + nickname Kick client from server. He will be able to connect again." << std::endl;
		std::cout << adminsCommands[2] << " Lists all peers Nicknames and their ip-addresses." << std::endl;
		std::cout << adminsCommands[4] << " Shuts down server." << std::endl;
		std::cout << adminsCommands[3] << " + nickname Bans  client from server, he wont be able to connect again." << std::endl;
		}
		else if(command == adminsCommands[1]) { //Kick
			std::cout << "Pld enter name of dat nigger." << std::endl;
			std::cin >> command;
			int tempSock;
			for(auto it = sockets.begin(); it != sockets.end(); ++it) {
				if((*it).second == command) {
					tempSock = (*it).first;
					break;
				}
			}
			sendall(tempSock, "U were kicked!", 15);
			close(tempSock);
			sockets.erase('tempSock');
		}
		else if(command == adminsCommands[2]) { //List
			for(auto it = sockets.begin(); it != sockets.end(); ++it) {
				std::cout << (*it).second << " " << addresses[(*it).second] << std::endl;
			}
		}
		else if(command == adminsCommands[3]) { // Ban
			std::cout << "Pls enter name of dat Nigger." << std::endl;
			std::cin >> command;
			int tempSock;
			for(auto it = sockets.begin(); it != sockets.end(); ++it) {
				if((*it).second == command) {
					tempSock = (*it).first;
					break;
				}
			}

			std::ofstream banList("ban_list.txt");
			banList << addresses[command] << "\n";
			close(tempSock);
			sockets.erase('tempSock');
			banList.close();

		}
		else if(command == adminsCommands[4]) { // exit
			std::cout << "W8 a bit..." << sockMain << std::endl;

			shallAccept = false;

			for(auto it = sockets.begin(); it != sockets.end(); ++it) {
				int tempSock = (*it).first;
				close(tempSock);
				puts("Done_socket closed.");
				sockets.erase('tempSock');
				if(sockets.empty()) break;
			}

			std::cout << sockets.size() << std::endl;

			shutdown(sockMain, 2);
			puts("Shutdown done");
			close(sockMain);

			for(size_t i = 0; i < threads.size(); ++i) {
				if(i != 1)
					pthread_cancel(threads[i]);
				puts("done pthread");
			}


			std::cout << "Done. " << std::endl;
			exit(0);

		}
	}

	pthread_exit(NULL);
}

std::string ntoa(struct sockaddr_in a) {

	char * str = inet_ntoa(a.sin_addr);
	std::string strq;
	for(size_t i = 0; i < strlen(str); ++i) {
		strq += str[i];
	}

	return strq;
}
