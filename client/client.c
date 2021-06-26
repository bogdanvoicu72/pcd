#include <stdio.h>	//printf
#include <stdlib.h>
#include <string.h>	//strlen
#include <sys/socket.h>	//socket
#include <arpa/inet.h>	//inet_addr
#include <unistd.h>
#include <pthread.h>

struct s_message
{
    int vitezaVant;
    int vitezaRotEoliana;
    int curentProdus;
};

void createMessage(char* message, struct s_message s_message_);
void *handler_recv_thread(void *argv);

int main(int argc , char *argv[])
{
    int sock;
    struct sockaddr_in server;
    char message[1000] , server_reply[2000];

    //Create socket
    sock = socket(AF_INET , SOCK_STREAM , 0);
    if (sock == -1)
    {
        printf("Could not create socket");
    }
    puts("Socket created");

    struct timeval timeout;      
    timeout.tv_sec = 10;
    timeout.tv_usec = 0;
    
    if (setsockopt (sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
    {
        error("setsockopt failed\n");
    }
        
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_family = AF_INET;
    server.sin_port = htons( 8888 );

    //Connect to remote server
    if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        perror("Connect failed. Error");
        return 1;
    }

    puts("Connected");

    int vitezaVant=21;
    int vitezaEoliana=15;
    int curentProdus=10;
    int SendToServer[100] = {vitezaVant,vitezaEoliana,curentProdus};

    int *new_sock = &sock;
    pthread_t waitForMessageThread;
    pthread_create(&waitForMessageThread, NULL, handler_recv_thread, (void *) new_sock);

    //keep communicating with server
    while(1)
    {
        char message[2000];

        struct s_message dummy_message;
        dummy_message.vitezaVant = 21;
        dummy_message.vitezaRotEoliana = 15;
        dummy_message.curentProdus = 10;

        createMessage(message, dummy_message);

        if( send(sock , message , strlen(message) , 0) < 0)
        {
            puts("Send failed");
            return 1;
        }
        bzero(&message, sizeof(message));
        sleep(1);
    }

    close(sock);
    return 0;
}

void *handler_recv_thread(void *argv)
{
    int sock = *(int*)argv;

    char message[2048];

    while (1)
    {
        if (recv(sock , message , sizeof(message), 0) >= 0)
        {
            fprintf(stdout, "Server said: %s\n", message);

            // check if message == something
            bzero(&message, sizeof(message));
        };
    }
    

}

void createMessage(char* message, struct s_message s_message_)
{
    char bufferWindSpeed[256];
    char bufferTurbineRot[256];
    char bufferEnergyProduced[256];

    snprintf(bufferWindSpeed, sizeof(bufferWindSpeed), "%d", s_message_.vitezaVant);
    snprintf(bufferTurbineRot, sizeof(bufferTurbineRot), "%d", s_message_.vitezaRotEoliana);
    snprintf(bufferEnergyProduced, sizeof(bufferEnergyProduced), "%d", s_message_.curentProdus);

    strcat(message, "Viteza vant:            ");
    strcat(message, bufferWindSpeed);
    strcat(message, "kts\n");

    strcat(message, "Viteza rotatie eoliana: ");
    strcat(message, bufferTurbineRot);
    strcat(message, "rpm\n");

    strcat(message, "Curent produs:          ");
    strcat(message, bufferEnergyProduced);
    strcat(message, "W");
}
