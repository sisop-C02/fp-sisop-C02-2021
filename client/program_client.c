#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>

bool check_empty();
int connection_start();
void exit_program();
void start_query(char*);

// Global variable
int sock;
char credentials[101];

int main(int argc , char *argv[])
{
	// Combining credentials
	if (argc > 1) {
		strcpy(credentials,"l");
		strcat(credentials, argv[2]);
		strcat(credentials, ":");
		strcat(credentials, argv[4]);
	}
	
	// Connection attempt
	if( connection_start() )
		return 1;
	// Availability check
	bool isEmpty = check_empty();
	
	// Mengirim info user ke server
	puts("Sending credentials to server...");
	send(sock , credentials , strlen(credentials) , 0);
	// Menerima balasan server
	char creds_check[1000] = {0};
	recv(sock , creds_check , sizeof(creds_check) , 0);
	
	// Jika login gagal
	if (!strcmp(creds_check, "login_fail")) {
		puts("Invalid credentials\nExiting...");
		exit_program();
		return 0;
	} else {
		printf("Welcome, %s.\nType EXIT to exit this program.\n\n", argv[2]);
	}

	while(true) {
	    char *user_msg = malloc(2000);
	    fgets(user_msg, 2000, stdin);
	    if ((strlen(user_msg) > 0) && (user_msg[strlen (user_msg) - 1] == '\n'))
	        user_msg[strlen (user_msg) - 1] = '\0';
		if (!strcmp(user_msg,"EXIT")) {
			exit_program();
			break;
		}
		start_query(user_msg);
		free(user_msg);
	}


	close(sock);
	return 0;
}

bool check_empty() {
	// Check availability
	char empty[1000] = {0}, usrcreds[101] = {0};
	recv(sock , empty , sizeof(empty) , 0);
	while(strcmp(empty, "free")) {
		memset(empty, 0, sizeof(empty));
		recv(sock , empty , sizeof(empty) , 0);
	}
	memset(empty, 0, sizeof(empty));
	return true;
}

int connection_start() {
	//Create socket
	struct sockaddr_in server;
	sock = socket(AF_INET , SOCK_STREAM , 0);
	if (sock == -1)
		printf("Could not create socket.");

	server.sin_addr.s_addr = inet_addr("127.0.0.1");
	server.sin_family = AF_INET;
	server.sin_port = htons( 8888 );
	//Connect to remote server
	if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0) {
		perror("Connection failed.");
		return 1;
	}
	puts("[connected]");
	puts("You're connected. Please wait...\n");
	return 0;
}

void exit_program() {
	char msg[10] = "exit";
	send(sock , msg , strlen(msg) , 0);
	close(sock);
}

void start_query(char* user_query) {
	send(sock , user_query , strlen(user_query) , 0);
	// Menerima balasan server
	char msg[1000] = {0};
	recv(sock , msg , sizeof(msg) , 0);
	printf("%s\n", msg);
}