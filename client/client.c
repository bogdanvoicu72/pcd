
#include <stdio.h>	//printf
#include <string.h>	//strlen
#include <sys/socket.h>	//socket
#include <arpa/inet.h>	//inet_addr
#include <unistd.h>

//trimitem data catre server cu timeout de 20 de sec
int SockSend(int sock, char* Rqst, short lenRqst)
{
    int shortRetval = -1;
    struct timeval tv;
    tv.tv_sec = 20;
    tv.tv_usec = 0;

    if(setsockopt(sock,SOL_SOCKET, SO_SNDTIMEO,(char *)&tv, sizeof (tv)) < 0)
    {
        printf("Time out\n");
        return -1;
    }
    shortRetval = send(sock, Rqst, lenRqst,0);
    return shortRetval;

}

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

    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_family = AF_INET;
    server.sin_port = htons( 8888 );

    //Connect to remote server
    if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        perror("connect failed. Error");
        return 1;
    }

    puts("Connected\n");



    int vitezaVant=21;
    int vitezaEoliana=15;
    int curentProdus=10;
    int SendToServer[100] = {vitezaVant,vitezaEoliana,curentProdus};

    //keep communicating with server
    while(1)
    {
        printf("Datele trimise sunt: ",vitezaEoliana,vitezaVant,curentProdus  );
        gets(SendToServer);
        SockSend(sock, SendToServer, strlen(SendToServer));

        //Send some data
        if( send(sock , message , strlen(message) , 0) < 0)
        {
            puts("Send failed");
            return 1;
        }

        //Receive a reply from the server
        if( recv(sock , server_reply , 2000 , 0) < 0)
        {
            puts("recv failed");
            break;
        }

        puts("Server reply :");
        puts(server_reply);



    }

    close(sock);
    return 0;
}