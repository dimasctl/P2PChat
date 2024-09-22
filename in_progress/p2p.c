#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#include <pthread.h>

#define PORT 8080

#include "server.h"
#include "client.h"

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

char* get_input();

int main() {
    system("clear");
    printf("Welcome to P2PChat!\n");
    fflush(stdout);

    // Create a server thread
    if (pthread_create(&server_thread, NULL, server, NULL) != 0) {
        perror("pthread_create");
        exit(EXIT_FAILURE);
    }

    while (1) {
        printf("You can now enter your message:\n");
        fflush(stdout);
        char* message;
        // Read the message from the user including spaces, allocate memory dynamically
        message = get_input();


        printf("For which IP address do you want to send the message?\n");
        fflush(stdout);
        char ip[16];
        scanf("%s", ip);

        printf("For which port do you want to send the message?\n");
        fflush(stdout);
        int port;
        scanf("%d", &port);

        // run the client function
        client(message, ip, port);

        // Free the memory allocated by getline
        free(message);
    }
    // Wait for the server thread to finish
    pthread_join(server_thread, NULL);

    return 0;
}

// Function to get the input from the user dynamically
char* get_input() {
    size_t size = 10;
    char* input = malloc(size);
    if (input == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    size_t len = 0;
    int c;

    //if the first character is '\n', ignore it
    if ((c = getchar()) == '\n') {
        c = getchar();
    }

    while ((c = getchar()) != '\n' && c != EOF) {
        if (len + 1 >= size) {
            size *= 2;
            char* tmp = realloc(input, size);
            if (!tmp) {
                free(input);
                printf("Memory reallocation failed\n");
                return NULL;
            }
            input = tmp;
        }
        input[len++] = c;
    }

    input[len] = '\0';
    return input;
}
