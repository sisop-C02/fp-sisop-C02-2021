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


// Query starts
struct TableHeader {
    char *name;
    int type;
};

struct Column {
    void *value;
};

struct Row {
    struct Column **columns;
};

struct Table {
    struct TableHeader **header;
    struct Row **row;
    int rowCount;
    int columnCount;
};

int* getTableKeys(char **columns) {

}

struct Column *createColumn(void *value) {
    struct Column *tb = malloc(sizeof(struct Column*));
    tb->value = value;
    return tb;
}

struct Row *parseRow(struct Table *ta, struct Row *r, char *row) {
    int len = strlen(row);
    int lookForDelimiter = 1;
    int lookForQuote = 0;
    int type = ta->header[0]->type;
    int columnAt = 0;
    char *temp = malloc(sizeof(char)*strlen(row));
    for(int i=0; i<len; i++) {
        if(row[i] == '"' && lookForQuote) {
            lookForQuote = 0;
            continue;
        } else if(row[i] == '"' && !lookForQuote) {
            lookForQuote = 1;
            continue;
        }
        
        if(row[i] == ';' && lookForDelimiter && lookForQuote == 0) {
            struct Column *column = malloc(sizeof(struct Column*));
            
            r->columns = malloc(sizeof(struct Column*)*strlen(row));
            printf("%s\n", temp);
            if(type == 0) {
                printf("Masuk int\n");
                int intVal = atoi(temp);
                int *ptr = &intVal;
                column->value = ptr;
                r->columns[columnAt++] = column;
            } else {
                printf("Masuk str\n");
                column->value = temp;
                r->columns[columnAt++] = column;
            }
            type = ta->header[columnAt]->type;
            temp[0] = '\0'; // cut substring
        } else {
            // push to buffer
            strncat(temp, &row[i], 1);
        }
    }
}

struct Row *getRowByIndex(struct Table *table, int index) {
    return table->row[index];
}

int loadFromFile(struct Table *ta, char *file) {
    int i = 0;
    char isHeader = 1;
    char *delim = "'\n'";
    char *split, *endToken;
    split = strtok_r(file, delim, &endToken);
    while (split != NULL) {
        if(isHeader == 1) {
            int columnCount = 0;

            // parse header, input e.g. idi;ages;names;
            char *headerColumn;
            char *headerDelim = ";";
            char *endToken2;

            char *newSplit = strdup(split);
            
            headerColumn = strtok_r(newSplit, headerDelim, &endToken2);
            ta->header = malloc(sizeof(struct TableHeader*)*strlen(headerColumn));
            while(headerColumn != NULL) {
                struct TableHeader *tb = malloc(sizeof(struct TableHeader*));
                char *buffer = malloc(sizeof(char)*strlen(headerColumn));

                strcpy(buffer, headerColumn);
                int type = 0;
                if(buffer[strlen(buffer)-1] == 's') {
                    type = 1;
                }

                // set name and type 
                tb->type = type;
                buffer[strlen(buffer)-1] = '\0';
                tb->name = buffer;
                ta->header[columnCount++] = tb;

                headerColumn = strtok_r(NULL, headerDelim, &endToken2);
            }
            ta->columnCount = columnCount;
            isHeader = 0;
        } else {
            struct Row *r = malloc(sizeof(struct Row*));
            r->columns = malloc(sizeof(struct Column*)*ta->columnCount*10);
            parseRow(ta, r, split);
            ta->row = malloc(sizeof(struct Row*)*100);
            // printf("%d\n", *((int*) r->columns[0]->value));
            ta->row[ta->rowCount] = r;
            ta->rowCount++;
        }
        split = strtok_r(NULL, delim, &endToken);
    }
    for(int i=0;i<ta->columnCount;i++) {
        printf("Header %d %s %d\n", i, ta->header[i]->name, ta->header[i]->type);
    }
    void *ptr = ta->row[0]->columns[0];
    printf("Column count %d\n", ta->columnCount);
    printf("Row count %d\n", ta->rowCount);
    printf("%s\n", file);
}

struct DatabaseServer {

};
// Query ends




// structures
typedef struct client {
	char username[50];
	char password[50];
} client;

typedef struct db_permit {
	char db_name[50];
	char db_user[50];
} db_permit;

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
char listofperms[100][50];
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
			if(client_message[0] == 'l' || client_message[0] == 's') {
				strcpy(credentials, client_message);
				incorrect = false;
			}
			memset(client_message,0,2000);
		}

		if (credentials[0] == 's') {
			status = true;
		} else if (credentials[0] == 'l'){
			status = login(credentials, sock);
		}
		if (!status) {
			continue;
		} 
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
    		char *msg_gagal = "regis_failed";
    		send(sock , msg_gagal , strlen(msg_gagal), 0);
    		return false;
    	}
    }

    fprintf(fptr, "%s\n", cleancreds);
    fclose(fptr);
    char *msg_success = "regis_success";
    send(sock , msg_success , strlen(msg_success), 0);
    return true;
}

void permit(char* permis, int sock) {
	// menghapus huruf p
	char cleanper[50]; memset(cleanper,0,sizeof(cleanper));
	for (int i = 1; i < strlen(permis); ++i)
		cleanper[i-1] = permis[i];

	int counter = 0;
	FILE *fptr;
    fptr = fopen("permit.txt", "a+");
    while(fscanf(fptr, "%s\n", listofperms[counter]) != EOF)
    	counter++;

    for (int i = 0; i < counter; ++i) {
    	if (!strcmp(listofperms[i], cleanper)) {
    		fclose(fptr);
    		char *msg_gagal = "permission_failed";
    		send(sock , msg_gagal , strlen(msg_gagal), 0);
    		return;
    	}
    }

    fprintf(fptr, "%s\n", cleanper);
    fclose(fptr);
    char *msg_success = "permission_success";
    send(sock , msg_success , strlen(msg_success), 0);
}

void process_query(int sock, char* user_query) {
	//Query
	// struct Table myTable = {};
 //    char buffer[100];
 //    strcpy(buffer, "idi;ages;names;\n1;3213;\n1;3213;");
 //    loadFromFile(&myTable, buffer);
	if (user_query[0]=='r')
	{
		regis(user_query, sock);
	}
	else if (user_query[0]=='p')
 	{
		permit(user_query, sock);
	}
	return;
}