#include<stdio.h>
#include<string.h>	//strlen
#include<stdlib.h>	//strlen
#include<sys/socket.h>
#include<arpa/inet.h>	//inet_addr
#include<unistd.h>	//write
#include<pthread.h> //for threading , link with lpthread
#include <sys/stat.h>
#include <fcntl.h>

pthread_mutex_t lock_data = PTHREAD_MUTEX_INITIALIZER;

int adminIsConnected;
int clientsConnectedNr;
int newData;

char data[100][2048];

void *connection_handler(void *);
void *wait_for_web_connections(void *);
void *web_connection_handler(void *);
void *wait_for_admin_connection(void *);
void *admin_connection_handler(void *);
void createHTTPResponse(char*);

int main(int argc , char *argv[]) {

    adminIsConnected = 0;
    clientsConnectedNr = 0;
    newData = 0;

    pthread_t webConnectionsHandler;
    pthread_create(&webConnectionsHandler, NULL, wait_for_web_connections, NULL);

    pthread_t adminConnectionsHandler;
    pthread_create(&adminConnectionsHandler, NULL, wait_for_admin_connection, NULL);

    int socket_desc, client_sock, c, *new_sock;
    struct sockaddr_in server, client, webconnection;

    //Create socket
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc == -1) {
        printf("Could not create socket");
    }
    puts("[+] Socket created");

    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(8888);

    //Bind
    if (bind(socket_desc, (struct sockaddr *) &server, sizeof(server)) < 0) {
        //print the error message
        perror("bind failed. Error");
        return 1;
    }
    puts("[+] Bind done");

    //Listen
    listen(socket_desc, 3);

    //Accept and incoming connection
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);
    while ((client_sock = accept(socket_desc, (struct sockaddr *) &client, (socklen_t *) &c))) {
        puts("[+] Connection accepted");

        pthread_t sniffer_thread;
        new_sock = malloc(1);
        *new_sock = client_sock;

        if (pthread_create(&sniffer_thread, NULL, connection_handler, (void *) new_sock) < 0) {
            perror("Could not create thread");
            return 1;
        }
    }

    if (client_sock < 0) {
        perror("Accept failed");
        return 1;
    }

    return 0;
}

void sendFile(char *path, int sockfd)
{
	int fd;
	struct stat stbuf;

	fd = open(path, O_RDONLY);

	if (0 == fd) {
		printf("Cannot open file at path: %s", path);
		return;
	}
	fstat(fd, &stbuf);

	sendfile(sockfd, fd, 0, stbuf.st_size);
	close(sockfd);
	close(fd);
}

void createHTTPResponse(char* response)
{
    strcpy(response, "HTTP/1.1 200 OK\ncontent-type: text/html\nServer: WindTurbine\n\n");

    strcat(response, "<html>");
    strcat(response, "<head>");
    strcat(response, "<body>");
    
    for (int i = 0; i < clientsConnectedNr; i++)
    {
        strcat(response, "<h3>");
        pthread_mutex_lock(&lock_data);
        strcat(response, data[i]);
        pthread_mutex_unlock(&lock_data);
        strcat(response, "</h3>");
    }
    
    strcat(response, "</body>");
    strcat(response, "</head>");
    strcat(response, "</html>");

    strcat(response, "\r\n\r\n");

    return response;
}

void *wait_for_admin_connection(void *argv)
{
    struct sockaddr_in server, client;
    int client_sock, c;
    int* new_sock;
    //Create socket
    int socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc == -1) {
        printf("Could not admin create socket");
    }
    puts("[+] Admin socket created");

    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(9090);

    //Bind
    if (bind(socket_desc, (struct sockaddr *) &server, sizeof(server)) < 0) {
        //print the error message
        perror("Admin bind failed. Error");
        return 1;
    }
    puts("[+] Admin bind done");

    //Listen
    listen(socket_desc, 3);

    //Accept and incoming connection
    puts("Waiting for incoming admin connections...");
    c = sizeof(struct sockaddr_in);
    if ((client_sock = accept(socket_desc, (struct sockaddr *) &client, (socklen_t *) &c)) && adminIsConnected == 0) {
        adminIsConnected = 1;
        puts("[+] Admin connection accepted");

        pthread_t sniffer_thread;
        new_sock = malloc(1);
        *new_sock = client_sock;

        if (pthread_create(&sniffer_thread, NULL, admin_connection_handler, (void *) new_sock) < 0) {
            perror("Could not create admin thread");
            return 1;
        }
    }

    if (client_sock < 0) {
        perror("Admin accept failed");
        return 1;
    }
}

void *wait_for_web_connections(void *argv)
{
    struct sockaddr_in server, client;
    int client_sock, c;
    int* new_sock;
    //Create socket
    int socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc == -1) {
        printf("Could not web create socket");
    }
    puts("[+] Web socket created");

    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(8080);

    //Bind
    if (bind(socket_desc, (struct sockaddr *) &server, sizeof(server)) < 0) {
        //print the error message
        perror("web bind failed. Error");
        return 1;
    }
    puts("[+] Web bind done");

    //Listen
    listen(socket_desc, 3);

    //Accept and incoming connection
    puts("Waiting for incoming WEB connections...");
    c = sizeof(struct sockaddr_in);
    while ((client_sock = accept(socket_desc, (struct sockaddr *) &client, (socklen_t *) &c))) {
        puts("[+] Web connection accepted");

        pthread_t sniffer_thread;
        new_sock = malloc(1);
        *new_sock = client_sock;

        if (pthread_create(&sniffer_thread, NULL, web_connection_handler, (void *) new_sock) < 0) {
            perror("Could not create web thread");
            return 1;
        }
    }

    if (client_sock < 0) {
        perror("Web accept failed");
        return 1;
    }
}

void *admin_connection_handler(void *argv)
{
    int sock = *(int*)argv;
    //printf("Admin thread\n");
    while (1)
    {
        char fromAdmin[2048];
        char adminMessage[2048] = "To admin";

        recv(sock, fromAdmin, sizeof(fromAdmin), 0);

        if (newData != 0)
        {
            char headerMessage[2048] = "FILE";
            send(sock, headerMessage, sizeof(headerMessage), 0);
            sendFile("config.txt", sock);
            bzero(&headerMessage, sizeof(headerMessage));
        
            // Delete file contents
            FILE *fp;
            fp = fopen("config.txt", "w");
            fclose(fp);

            newData = 0;
        }
        else
        {
            send(sock, adminMessage, sizeof(adminMessage), 0);
        }
        
        bzero(&adminMessage, sizeof(adminMessage));
        bzero(&fromAdmin, sizeof(fromAdmin));

    }

}

void *web_connection_handler(void *argv)
{
    int sock = *(int*)argv;
    char webMessage[2048];

    recv(sock , webMessage, sizeof(webMessage), 0);
    printf("-------------\nFrom web page\n-------------\n%s\n\n", webMessage);

    char response[2048];
    createHTTPResponse(response);
    
    //printf("rsp\n%s\n", response);
    
    send(sock, response, sizeof(response), 0);

    bzero(&webMessage, sizeof(webMessage));
    close(sock);
    pthread_exit(NULL);
}

void *connection_handler(void *socket_desc)
{
    //Get the socket descriptor
    int sock = *(int*)socket_desc;
    int read_size;
    char client_message[2000];

    //Receive a message from client
    while( (read_size = recv(sock , client_message , 2000 , 0)) > 0 )
    {
        fprintf(stdout, "--------------\nClient NO: %d\n--------------\n%s\n\n", sock, client_message);
        newData = 1;

        char buffer[10];
        snprintf(buffer, sizeof(buffer), "%d", sock);

        pthread_mutex_lock(&lock_data);
        strcat(data[clientsConnectedNr], "Client ID: ");
        strcat(data[clientsConnectedNr], buffer);
        strcat(data[clientsConnectedNr], " - ");
        strcat(data[clientsConnectedNr++], client_message);
        pthread_mutex_unlock(&lock_data);

        FILE *fp;

        fp = fopen("config.txt", "a+");
        fprintf(fp, "%s", client_message);
        fprintf(fp, "\n");
        fclose(fp);
        
        if(read_size <= 0)
        {
            puts("Client disconnected");
            fflush(stdout);
        } else if(read_size == -1)
        {
            perror("recv failed");
        }

        char messageToCLient[2048] = {"Sample message."};

        send(sock , messageToCLient , strlen(messageToCLient) , 0);
        bzero(&client_message, sizeof(client_message));
    }

    //Free the socket pointer
    free(socket_desc);

    return 0;
}

