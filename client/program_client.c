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
void start_query(char*, bool);

// Global variable
int sock;
char credentials[101];
char newString[50][100] = {0};

int main(int argc , char *argv[])
{
	// Connection attempt
	if( connection_start() )
		return 1;
	// Availability check
	bool isEmpty = check_empty();

	bool isRootUser = false;
	// Jika akses root
	if (geteuid() == 0) {
		printf("Welcome, root user.\nType EXIT to exit this program.\n\n");
		isRootUser = true;
		send(sock , "sudo" , strlen("sudo") , 0);
	} else if (argc > 1 && !isRootUser) {
		// Combining credentials
		strcpy(credentials,"l");
		strcat(credentials, argv[2]);
		strcat(credentials, ":");
		strcat(credentials, argv[4]);
		
		// Mengirim info user ke server
		puts("Sending credentials to server...");
		send(sock , credentials , strlen(credentials) , 0);
		// Menerima balasan server
		char creds_check[1000] = {0};
		recv(sock , creds_check , sizeof(creds_check) , 0);
		// Jika login gagal
		if (!strcmp(creds_check, "login_failed")) {
			puts("Invalid credentials\nExiting...");
			exit_program();
			return 0;
		} else {
			printf("Welcome, %s.\nType EXIT to exit this program.\n\n", argv[2]);
		}
	} else {
		puts("No credentials entered.\n");
		exit_program();
		return 0;
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
		start_query(user_msg, isRootUser);
		free(user_msg);
	}

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
	printf("App closed\n");
}

void split_string(char* str1) {
	memset(newString, 0, sizeof(newString));
	int i, j = 0, ctr = 0;
	for(i=0;i<=(strlen(str1));i++) {
        if(str1[i]==' '||str1[i]=='\0') {
            newString[ctr][j]='\0';
            ctr++; 
            j=0;
        } else {
            newString[ctr][j]=str1[i];
            j++;
        }
    }

}

void start_query(char* user_query, bool isRootUser) {
	user_query[strlen(user_query)-1] = '\0';
	if (isRootUser) {
		if (strstr(user_query, "CREATE USER")!=NULL)
		{
			if (strstr(user_query, "IDENTIFIED BY")==NULL) {
				printf("Invalid syntax for create user.\n");
				return;
			}
			// User register process
			split_string(user_query);
			memset(credentials, 0, sizeof(credentials));
			strcpy(credentials, "r"); strcat(credentials, newString[2]);
			strcat(credentials, ":"); strcat(credentials, newString[5]);
			send(sock , credentials , strlen(credentials) , 0);
			// Menerima balasan server
			char msg[1000] = {0};
			recv(sock , msg , sizeof(msg) , 0);
			printf("%s\n\n", msg);
			return;
		}
		else if (strstr(user_query, "GRANT PERMISSION")!=NULL) 
		{
			if (strstr(user_query, "INTO")==NULL) {
				printf("Invalid syntax for granting permission.\n");
				return;
			}
			// User permission process
			split_string(user_query);
			char permit[100];
			memset(permit, 0, sizeof(permit));
			strcpy(permit, "p"); strcat(permit, newString[2]);
			strcat(permit, ":"); strcat(permit, newString[4]);
			send(sock , permit , strlen(permit) , 0);
			// Menerima balasan server
			char msg[1000] = {0};
			recv(sock , msg , sizeof(msg) , 0);
			printf("%s\n\n", msg);
			return;
		}
	}

	send(sock , user_query , strlen(user_query) , 0);
	// Menerima balasan server
	char msg[1000] = {0};
	recv(sock , msg , sizeof(msg) , 0);
	printf("%s\n", msg);
}