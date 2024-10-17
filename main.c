#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#include <pthread.h>

#define PORT 8080

#include "include/misc.h"
client_info* client_list = NULL;
#include "include/server.h"
#include "include/client.h"

pthread_t server_thread, client_thread;

typedef struct con {
    char ip[16];
    int port;
    struct con* next;
} Connection;

typedef struct {
    char* username;
    char* user_id;
    Connection* connections;
} ConnectionList;

int main() {
    printf("\033[H\033[J");
    printf("Welcome to P2PChat!\n");
    fflush(stdout);

    // Create a server thread
    if (pthread_create(&server_thread, NULL, server, NULL) != 0) {
        perror("pthread_create");
        exit(EXIT_FAILURE);
    }

    sleep(1);
    send_broadcast();
    sleep(1);
    while (1) {
        client_info* current = print_clients(client_list);
        chat_with(current);
    }
    return 0;
}
