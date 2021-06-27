#include<stdio.h>
#include<string.h>	//strlen
#include<stdlib.h>	//strlen
#include<sys/socket.h>
#include<arpa/inet.h>	//inet_addr
#include<unistd.h>	//write
#include<pthread.h> //for threading , link with lpthread

int adminIsConnected;

void *connection_handler(void *);
void *wait_for_web_connections(void *);
void *web_connection_handler(void *);
void *wait_for_admin_connection(void *);
void *admin_connection_handler(void *);

int main(int argc , char *argv[]) {

    adminIsConnected = 0;

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


    //post request

    char aszXmlData[]="<body><name>amlendra</name><age>25</age></body>";
    char aszXmlRequest[250]= {0};
    char aszServiceMethod[]="applicationform.svc/getdetail";
    char aszRequest[150]= {0};
    char aszHostIp[30]="127.0.0.1";
    char aszPort[]="8080";
    sprintf(aszRequest,"http://%s:%s/%s/%s HTTP/1.1",aszHostIp,aszPort,aszServiceMethod);
    printf("Method and Resource path is below:\n\n\n");
    printf("%s",aszRequest);
    strcat(aszHostIp,":");
    strcat(aszHostIp,aszPort);
    printf("\n\nHOST header is below:\n\n\n");
    printf("%s",aszHostIp);
    sprintf(aszXmlRequest,"POST %s\r\nHost: %s\r\nContent-Type: text/html\r\nContent-Length: %d\r\n\r\n%s\r\n",aszRequest,aszHostIp,strlen(aszXmlData),aszXmlData);
    printf("\n\n\nPOST Request which send to the server:\n\n");
    printf("%s",aszXmlRequest);






    return 0;
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
    if ((client_sock = accept(socket_desc, (struct sockaddr *) &client, (socklen_t *) &c)) && adminIsConnected > 0) {
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
    char adminMessage[2048];

    recv(sock , adminMessage, sizeof(adminMessage), 0);
    printf("-------------\nFrom admin\n-------------\n%s\n\n", adminMessage);

    bzero(&adminMessage, sizeof(adminMessage));
}

void *web_connection_handler(void *argv)
{
    int sock = *(int*)argv;
    char webMessage[2048];

    recv(sock , webMessage, sizeof(webMessage), 0);
    printf("-------------\nFrom web page\n-------------\n%s\n\n", webMessage);

    bzero(&webMessage, sizeof(webMessage));
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
        if(read_size <= 0)
        {
            puts("Client disconnected");
            fflush(stdout);
        } else if(read_size == -1)
        {
            perror("recv failed");
        }

        char messageToCLient[2048] = {"Sample message. Client does nothing"};

        send(sock , messageToCLient , strlen(messageToCLient) , 0);
        bzero(&client_message, sizeof(client_message));
    }

    //Free the socket pointer
    free(socket_desc);

    return 0;
}

