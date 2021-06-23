#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h>


int main(int argc, char ** argv)
{
    int port;
    int sock = -1;
    struct sockaddr_in address;
    struct hostent * host;
    int len;

    //verificam parametrii de pe linia de comanda

    if(argc !=4)
    {
        printf("usage: %s hostname port text\n", argv[0]);
        return -1;
    }

    //obtinem portul daca nu e ok programul se va inchide
    if (sscanf(argv[2], "%d", &port) <= 0)
    {
        fprintf(stderr, "%s: error: wrong parameter: port\n", argv[0]);
        return -2;
    }
    //creeam socket
    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock <= 0)
    {
        fprintf(stderr, "%s: error: cannot create socket\n", argv[0]);
        return -3;
    }
    //incercam conexiunea la server
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    host = gethostbyname(argv[1]);
    if (!host)
    {
        fprintf(stderr, "%s: error: unknown host %s\n", argv[0], argv[1]);
        return -4;
    }
    memcpy(&address.sin_addr, host->h_addr_list[0], host->h_length);

    //cy socket, addresa host ne putem conecta la server

    if (connect(sock, (struct sockaddr *)&address, sizeof(address)))
    {
        fprintf(stderr, "%s: error: cannot connect to host %s\n", argv[0], argv[1]);
        return -5;
    }
    //trimitem text la server

    len = strlen(argv[3]);
    write(sock, &len, sizeof(int));
    write(sock, argv[3], len);

    close(sock);
    return 0;
}