#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "ipc.h"


struct s_message
{
    int vitezaVant;
    int vitezaRotEoliana;
    int curentProdus;
};

int generateRandom(int lower, int upper, int count);
void createMessage(char* message, struct s_message s_message_);


int main() {
    int fd;
    struct sockaddr_un addr;
    int ret;
    char buff[8192];
    struct sockaddr_un from;
    int ok = 1;
    int len;

    if ((fd = socket(PF_UNIX, SOCK_DGRAM, 0)) < 0) {
        perror("socket");
        ok = 0;
    }

    if (ok) {
        memset(&addr, 0, sizeof(addr));
        addr.sun_family = AF_UNIX;
        strcpy(addr.sun_path, CLIENT_SOCK_FILE);
        unlink(CLIENT_SOCK_FILE);
        if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
            perror("bind");
            ok = 0;
        }
    }

    if (ok) {
        memset(&addr, 0, sizeof(addr));
        addr.sun_family = AF_UNIX;
        strcpy(addr.sun_path, SERVER_SOCK_FILE);
        if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
            perror("connect");
            ok = 0;
        }
    }

    if (ok) {
        char message[2000];

        int lower = 3, upper = 50, count =1;

        int val1 = generateRandom(lower,upper,count);
        int val2 = generateRandom(lower,upper,count);
        int val3 = generateRandom(lower,upper,count);
        struct s_message dummy_message;
        dummy_message.vitezaVant = val1;
        dummy_message.vitezaRotEoliana = val2;
        dummy_message.curentProdus = val3;

        createMessage(message, dummy_message);

        if (send(fd, message, strlen(message), 0) == -1) {
            perror("send");
            ok = 0;
        }
        printf ("sent info\n");
    }

    if (ok) {
        if ((len = recv(fd, buff, 8192, 0)) < 0) {
            perror("recv");
            ok = 0;
        }
        printf ("receive %d %s\n", len, buff);
    }

    if (fd >= 0) {
        close(fd);
    }

    unlink (CLIENT_SOCK_FILE);
    return 0;
}

int generateRandom(int lower, int upper, int count)
{
    int i;
    for(i=0; i<count; i++)
    {
        int num = (rand() % (upper-lower +1)) + lower;
        return num;
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
