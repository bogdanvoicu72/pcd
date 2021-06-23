#include <stdio.h>
#include <pthread.h>
#include <sys/socket.h>
#include <linux/in.h>

typedef struct {
    int sock;
    struct sockaddr address;
    int addr_len;

} connection_t;

void * function(void *);

//thread

void *process(void * ptr)
{
    char *buffer;
    int len;
    connection_t * conn;
    long addr = 0;

    if(!ptr) pthread_exit(0);
    conn = (connection_t *)ptr;

    //citim lungimea mesajului;
    read(conn-> sock, &len, sizeof (int ));
    if(len > 0)
    {
        addr = (long)((struct sockaddr_in *)&conn->address)->sin_addr.s_addr;
        buffer = (char *)malloc((len+1)*sizeof(char));
        buffer[len] = 0;
        // citim mesajul
        read(conn->sock, buffer, len);

        //printam mesajul
        printf("%d.%d.%d.%d: %s\n",
               (addr      ) & 0xff,
               (addr >>  8) & 0xff,
               (addr >> 16) & 0xff,
               (addr >> 24) & 0xff,
               buffer);
        free(buffer);
    }
    //inchidem socket si clean
    close(conn->sock);
    free(conn);
    pthread_exit(0);
}

int main(int argc, char  **argv)
{
    int sock = -1;
    struct sockaddr_in address;
    int port;
    connection_t * connection;
    pthread_t thread;

    //verificam argumentele de la linia de comanda
    if(argc !=2)
    {
        fprintf(stderr, "usage: %s port\n", argv[0]);
        return -1;
    }
    //obtinem portul
    if (sscanf(argv[1], "%d", &port) <= 0)
    {
        fprintf(stderr, "%s: error: wrong parameter: port\n", argv[0]);
        return -2;
    }

    //aici creeam socket
    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock <= 0)
    {
        fprintf(stderr, "%s: error: cannot create socket\n", argv[0]);
        return -3;
    }

    //bind la socket port

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);
    if (bind(sock, (struct sockaddr *)&address, sizeof(struct sockaddr_in)) < 0)
    {
        fprintf(stderr, "%s: error: cannot bind socket to port %d\n", argv[0], port);
        return -4;
    }

    //portul pentru ascultare
    if (listen(sock, 5) < 0)
    {
        fprintf(stderr, "%s: error: cannot listen on port\n", argv[0]);
        return -5;
    }
    printf("%s: ready and listening\n", argv[0]);

    while (1)
    {
        // se accepta noile conexiuni
        connection = (connection_t *)malloc(sizeof(connection_t));
        connection->sock = accept(sock, &connection->address, &connection->addr_len);
        if (connection->sock <= 0)
        {
            free(connection);
        }
        else
        {
            /* start a new thread but do not wait for it */
            pthread_create(&thread, 0, process, (void *)connection);
            pthread_detach(thread);
        }
    }

    return 0;
}