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

    while (1) {
        char* message;
        // Read the message from the user including spaces, allocate memory dynamically
        message = get_input();


        printf("For which IP address do you want to send the message? (Default: 127.0.0.1)\n");
        fflush(stdout);
        char ip[16];
        scanf("%s", ip);
        if (strcmp(ip, "") == 0) {
            strcpy(ip, "127.0.0.1");
        }

        printf("For which port do you want to send the message?\n");
        fflush(stdout);
        int port;
        scanf("%d", &port);

        // run the client function
        send_message(message, ip, port);

        // Free the memory allocated by getline
        free(message);
    }
    // Wait for the server thread to finish
    pthread_join(server_thread, NULL);

    return 0;
}
