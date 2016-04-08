#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <pthread.h>
#include <netdb.h>
#include <iostream>
#include <vector>
#include <unistd.h>

void *listening(void *);
void *writing(void *);

void sendall(int s, char buff[], int len);

int sockMain;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
WINDOW *chat;
WINDOW *users;
WINDOW *input;

int car;

int main(int argc, char **argv) {

	car = 15;

	system("clear");

	if(argc < 4) {
		perror("No port.");
		exit(0);
	}

	std::string myName = argv[3];
	int numport = atoi(argv[1]);
	struct hostent *server;

	pthread_t threads[3];

	server = gethostbyname(argv[2]);
	if(server == NULL) {
		perror("No server.");
		exit(1);
	}

	sockMain = socket(AF_INET, SOCK_STREAM, 0);
	if(sockMain < 0) {
		perror("Error in socket: ");
		exit(2);
	}

	struct sockaddr_in addr;

	bzero((char *) &addr, sizeof(addr));

	addr.sin_family = AF_INET;
	bcopy((char *)server->h_addr, (char *)&addr.sin_addr.s_addr, server->h_length);
	addr.sin_port = htons(numport);


	initscr();
	curs_set(0);
	cbreak();


	chat = newwin(18, 59, 1, 1);
	box(chat, '*', '*');
	mvwaddstr(chat, 2, 1, "Chat:\n");

	users = newwin(18, 18, 1, 60);
	box(users, '*', '*');
	mvwaddstr(users, 1, 1, "Users:\n");

	input = newwin(4, 77, 19, 1);
	box(input, '*', '*');
	wmove(input, 1, 1);

	wclear(chat);
	wclear(input);
	wclear(users);
	wrefresh(chat);
	wrefresh(users);
	wrefresh(input);
	wmove(input, 1, 1);

	if(connect(sockMain, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		perror("Connection error: ");
		exit(3);
	}

	mvwaddstr(chat, 1, 1, "Chat:\n");
	mvwaddstr(chat, 2, 1, "Connected!\n");
	wrefresh(chat);
	write(sockMain, myName.c_str(), sizeof(myName));
	pthread_create(&threads[0], NULL, listening, NULL);
	pthread_create(&threads[1], NULL, writing, NULL);


	pthread_join(threads[0], NULL);
	pthread_join(threads[1], NULL);

	endwin();
	pthread_exit(NULL);
}

void *writing(void *) {
	char buffer[256];
	while(true) {
		
		bzero(buffer, 256);
		wrefresh(input);
		wgetstr(input, buffer);
		if(!buffer[0]) {
			wmove(input, 1, 1);
			continue;
		}
		pthread_mutex_lock(&mutex);
		wclear(input);
		box(input, '*', '*');
		wmove(input, 1, 1);
		wrefresh(input);
		sendall(sockMain, buffer, sizeof(buffer));
		pthread_mutex_unlock(&mutex);
	}
	pthread_exit(NULL);
}

void *listening(void *) {
	char buffer[1];
	box(chat, '*', '*');
	wrefresh(chat);
	while(true) {
		bzero(buffer, 1);
		std::string str;
		str.clear();
		while(true) {
			int n = read(sockMain, buffer, 1);
			if(buffer[0] == '\0') break;
			if(n <= 0) break;
			str += buffer[0];
		}

		if(!str.empty()) {
			if(str[0] == '&') {
				wclear(users);
				mvwaddstr(users, 1, 1, "Users:\n");
				str.erase(str.begin());
				mvwaddstr(users, 2, 1, str.c_str());
				box(input, '*', '*');
				wrefresh(users);
				wrefresh(chat);
			}
			else {
			if(str.size() > 50) {
				str.insert(49, "\n*");
			}
			mvwaddstr(chat, car, 1, str.c_str());
			wrefresh(chat);
			if(str.size() > 50 && car + 1 != 16) {
				car++;
			}
			if(car + 1 == 16) {
				scrollok(chat, true);
				wsetscrreg(chat, 2, 16);
				idlok(chat, true);
				wscrl(chat, 1);
				box(chat, '*', '*');
				box(users, '*', '*');
				wrefresh(users);
				wrefresh(chat);
			}
			else if(car + 1 == 16 && str.size() > 50) {
				scrollok(chat, true);
				wsetscrreg(chat, 2, 16);
				idlok(chat, true);
				wscrl(chat, 3);
				box(chat, '*', '*');
				wrefresh(chat);

			}
			else
			car++;
			}
		}
		else if(str.empty()) {
			break;
		}
		str.clear();
		bzero(buffer, 1);
	}
	close(sockMain);
	pthread_exit(NULL);
}

void sendall(int s, char buff[], int len)
{
	int i = 0;
	for(; i < 256; ++i) {
		char subBuff[1];
		subBuff[0] = buff[i];
		int n = write(s, subBuff, 1);
		if(n <= 0) {
			puts("Smthign went wrong: ");
			break;
		}
		if(buff[i] == '\0') break;
		bzero(subBuff, 1);
	}
}
