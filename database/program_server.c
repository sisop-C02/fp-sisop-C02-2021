#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>	
#include <stdlib.h>	
#include <sys/socket.h>
#include <arpa/inet.h>	
#include <unistd.h>	
#include <pthread.h> 
#include <stdbool.h>

// structures
typedef struct client {
	char username[50];
	char password[50];
} client;


//the thread function
void *connection_handler(void *);

//user id functions
bool login(char*, int); 
void get_client(char*);
bool regis(char*, int);

//query functions
void process_query(int, char*);

//global variables
bool isOccupied;
char listofcreds[100][50];
char server_path[250] = "/home/jaglfr/Documents/SourceCodes/SisOp/fp/fp-sisop-C02-2021/database";
char client_path[250] = "/home/jaglfr/Documents/SourceCodes/SisOp/fp/fp-sisop-C02-2021/client";
client curr_client;


int main(int argc , char *argv[])
{
	int socket_desc , client_sock , c , *new_sock;
	struct sockaddr_in server , client;
	
	//Create socket
	socket_desc = socket(AF_INET , SOCK_STREAM , 0);
	if (socket_desc == -1)
	{
		printf("Could not create socket");
	}
	puts("Socket created");
	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons( 8888 );
	//Bind
	if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0) {
		perror("bind failed. Error");
		return 1;
	}
	puts("Bind done");
	//Listen
	listen(socket_desc , 10);

	puts("Waiting for incoming connections...\n");
	c = sizeof(struct sockaddr_in);
	
	while( (client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) )
	{
		puts("Connection accepted");
		
		pthread_t sniffer_thread;
		new_sock = malloc(1);
		*new_sock = client_sock;
		
		if( pthread_create( &sniffer_thread , NULL ,  connection_handler , (void*) new_sock) < 0)
		{
			perror("could not create thread");
			return 1;
		}

		pthread_join( sniffer_thread , NULL);
	}
	
	if (client_sock < 0)
	{
		perror("accept failed");
		return 1;
	}
	
	return 0;
}

void *connection_handler(void *socket_desc)
{
	//Get the socket descriptor
	int sock = *(int*)socket_desc;
	int read_size;
	char client_message[2000]; memset(client_message,0,sizeof(client_message));
	char credentials[50]; memset(credentials,0,50);
	
	// Cek multiple clients
	while(isOccupied);
	isOccupied = true;

	// Kirim ke client
	send(sock , "free" , strlen("free") , 0);
	bool client_exit = false;
	while(true) {
		bool status;
		// Client exits with "EXIT"
		if (client_exit){
			client_exit = false;
			close(sock); free(socket_desc);
			isOccupied = false;
			return 0;
		}
		// Looping input data
		memset(client_message,0,2000);
		bool incorrect = true;
		while( incorrect && (read_size = recv(sock , client_message , 2000 , 0)) > 0 )
		{
			if(client_message[0] == 'l' || client_message[0] == 'r') {
				strcpy(credentials, client_message);
				incorrect = false;
			} else if (!strcmp(client_message, "exit")){
				puts("[client exit]\n");
				close(sock); free(socket_desc);
				isOccupied = false;
				return 0;
			}
			memset(client_message,0,2000);
		}

		if (credentials[0] == 'r') {
			status = regis(credentials, sock);
		} else {
			status = login(credentials, sock);
		}
		if (!status) {
			continue;
		} 
		else {
			if (credentials[0] == 'r')
				continue; 
			else {
				puts("[client logged in]");
				// Setelah login
				while(true) 
				{
					char login_message[2000]; 
					memset(login_message,0,sizeof(login_message));
					recv(sock , login_message , 2000 , 0);
					// Jika logout kembali ke home menu
					if (!strcmp(login_message, "exit")) {
						puts("[client logged out]\n");
						client_exit = true;
						break;
					}
					process_query(sock, login_message);
				}
				continue;
			}
		}

	}

	// Client terputus
	if(read_size == 0) {
		puts("Client disconnected\n");
		fflush(stdout);
	} else if(read_size == -1) {
		perror("recv failed");
	}
	close(sock);
	free(socket_desc);
	isOccupied = false;

	return 0;
}

bool login(char credentials[], int sock) {
	// menghapus huruf l
	char cleancreds[50]; memset(cleancreds,0,sizeof(cleancreds));
	for (int i = 1; i < strlen(credentials); ++i)
		cleancreds[i-1] = credentials[i];

	int counter = 0;
	FILE *fptr;
    fptr = fopen("akun.txt", "a+");
    while(fscanf(fptr, "%s\n", listofcreds[counter]) != EOF)
    	counter++;
    fclose(fptr);

    for (int i = 0; i < counter; ++i) {
    	if (!strcmp(listofcreds[i], cleancreds)) {
    		char *msg_success = "login_success";
    		get_client(cleancreds);
    		send(sock , msg_success , strlen(msg_success), 0);
    		return true;
    	}
    }
    char *msg_gagal = "login_failed";
    send(sock , msg_gagal , strlen(msg_gagal), 0);
    return false;
}

void get_client(char credentials[]) {
	// memset
	memset(curr_client.username,0,sizeof(curr_client.username));
	memset(curr_client.password,0,sizeof(curr_client.password));
	// index pembagi
	int colon_idx = 0;
	for (int i = 0; i < strlen(credentials); ++i) {
		if (credentials[i] == ':')
			colon_idx = i;
	}
	// get username
	for (int i = 0; i < colon_idx; ++i)
		curr_client.username[i] = credentials[i];
	// get password
	for (int i = colon_idx + 1; i < strlen(credentials); ++i)
		curr_client.password[i-colon_idx-1] = credentials[i];
}

bool regis(char credentials[], int sock) {
	// Menghapus indikator 'r' dan parse username
	char usr_name[50]; memset(usr_name,0,sizeof(usr_name));
	int colon_idx = 0;
	char cleancreds[50]; memset(cleancreds,0,sizeof(cleancreds));
	for (int i = 1; i < strlen(credentials); ++i) {
		cleancreds[i-1] = credentials[i];
		if (credentials[i] == ':')
			colon_idx = i-1;
	}
	for (int i = 0; i < colon_idx; ++i)
		usr_name[i] = cleancreds[i];

	int counter = 0;
	FILE *fptr;
    fptr = fopen("akun.txt", "a+");
    while(fscanf(fptr, "%s\n", listofcreds[counter]) != EOF)
    	counter++;
    
    // Cek apakah sudah ada username
    for (int i = 0; i < counter; ++i) {
    	if (strstr(listofcreds[i], usr_name) != NULL) {
    		fclose(fptr);
    		char *msg_gagal = "gagal";
    		send(sock , msg_gagal , strlen(msg_gagal), 0);
    		return false;
    	}
    }

    fprintf(fptr, "%s\n", cleancreds);
    fclose(fptr);
    char *msg_success = "sukses";
    send(sock , msg_success , strlen(msg_success), 0);
    return true;
}

void process_query(int sock, char* user_query) {
	printf("\nUser query: %s\n", user_query);
	send(sock, "query_accepted", strlen("query_accepted"), 0);
	return;
}