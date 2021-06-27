#include <stdio.h>	//printf
#include <stdlib.h>
#include <string.h>	//strlen
#include <sys/socket.h>	//socket
#include <arpa/inet.h>	//inet_addr
#include <unistd.h>
#include <pthread.h>
#include <time.h>

void write_file(int);
void main_menu();
void read_file_contents(char* buffer)
{
    FILE * fp;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;

    fp = fopen("config.txt", "r");
    if (fp == NULL)
        printf("Cannot open config.txt\n");

    while ((read = getline(&line, &len, fp)) != -1) {
        strcat(buffer, line);
    }

    fclose(fp);
    if (line)
        free(line);
    //exit(EXIT_SUCCESS);
}

int main(int argc , char *argv[])
{
    main_menu();
    
    return 0;
}

void main_menu()
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
    timeout.tv_sec = 20;
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

    while(1)
    {
        fprintf(stdout, "1. Standard EXEC_MODE\n");
        fprintf(stdout, "2. Remove last wind turbine\n");
        fprintf(stdout, "3. Set RPM\n");
        fprintf(stdout, "\n0. Exit\n");

        int option;
        scanf("%d", &option);

        if (option == 1)
        {
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

                    char mailExec[4096];
                    char fileCOntents[2048];

                    read_file_contents(fileCOntents);
                    //fprintf(stdout, "%s\n", fileCOntents);
                    strcpy(mailExec, "python3 ../email-script.py \"");
                    strcat(mailExec, fileCOntents);
                    strcat(mailExec, "\"");

                    system("python3 -m smtpd -c DebuggingServer -n localhost:1025");
                    system(mailExec);

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

            //close(sock);
        }

        if (option == 2)
        {
            while(1)
            {
                char message[2048];
                char rmeol[2048] = "REMOVE_LAST";

                send(sock, rmeol, sizeof(rmeol), 0);
                recv(sock, message, sizeof(message), 0);

                fprintf(stdout, "%s\n", message);
                
                bzero(&message, sizeof(message));
                break;
            }

            //close(sock);
            //break;
        }

        if (option == 3)
        {
            char message[2048];
            char rmeol[2048] = "SET_RPM";
            char opt2[10];
            char rpm[10];

            send(sock, rmeol, sizeof(rmeol), 0);
            recv(sock, message, sizeof(message), 0);

            fprintf(stdout, "%s\n", message);
            
            bzero(&message, sizeof(message));
            
            fprintf(stdout, "%s\n", "Choose an ID: ");
            scanf("%s", opt2);

            fprintf(stdout, "%s\n", "RPM value: ");
            scanf("%s", rpm);

            send(sock, opt2, sizeof(opt2), 0);
            send(sock, rpm, sizeof(rpm), 0);

            //close(sock);
            //break;
        }

        if (option == 0)
            break;

    }
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