#include <stdio.h>	//printf
#include <stdlib.h>
#include <string.h>	//strlen
#include <sys/socket.h>	//socket
#include <arpa/inet.h>	//inet_addr
#include <unistd.h>
#include <pthread.h>
#include <time.h>

void write_file(int);

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
        perror("setsockopt failed\n");
    }
        
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_family = AF_INET;
    server.sin_port = htons( 9090 );

    //Connect to remote server
    if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        perror("Connect failed. Error");
        return 1;
    }

    puts("Connected");

    //keep communicating with server
    while(1)
    {
        char message[2048];
        char message2[2048];
        char dummyMessage[2048] = "Admin";

        send(sock, dummyMessage, sizeof(dummyMessage), 0);
        recv(sock, message, sizeof(message), 0);

        fprintf(stdout, "%s\n", message);

        if (strcmp(message, "FILE") == 0)
        {
            write_file(sock);

            FILE *f = fopen("config.txt", "rb");
            fseek(f, 0, SEEK_END);
            long fsize = ftell(f);
            fseek(f, 0, SEEK_SET);  /* same as rewind(f); */

            char *string = malloc(fsize + 1);
            fread(string, 1, fsize, f);
            fclose(f);

            string[fsize] = 0;

            fprintf(stdout, "%s\n", string);

            //char mailExec[4096];

            //system("python3 -m smtpd -c DebuggingServer -n localhost:1025");
            //system("python3 ../email-script.py asdasd");

        }
        else
        {
            recv(sock, message2, sizeof(message2), 0);
            fprintf(stdout, "%s\n", message2);
        }

        bzero(&message, sizeof(message));
        bzero(&message2, sizeof(message2));
        sleep(1);
    }

    close(sock);
    return 0;
}

void write_file(int sockfd)
{
    long long int countBytes = 0;
    int n;
    FILE *fp;
    char filename[64] = "config.txt";
    char buffer[2048];

    fp = fopen(filename, "wb");
    if (fp == NULL)
    {
        perror("Cannot create file");
        return;
    }
    
    while (1)
    {
        n = recv(sockfd, buffer, 2048, 0);
        countBytes += n;
        if (n <= 0)
        {
            break;
            return;
        }
        fwrite(buffer, n, 1, fp);
        bzero(buffer, 2048);
    }
    fprintf(stdout, "Received %lld bytes\n", countBytes);
    fclose(fp);

    return;
}