#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#include <pthread.h>

#define PORT 8080

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
        send_broadcast();
        sleep(5);
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

// Function to get the input from the user dynamically
char* get_input() {
  // Initialize the input buffer
  char* input = NULL;
  size_t len = 0;
  ssize_t read;

  read = getline(&input, &len, stdin);
  // If there is only a newline character, try again
  if (read == 1) {
    free(input);
    return get_input();
  }

  if (read == -1) {
    perror("getline");
    exit(EXIT_FAILURE);
  }

  // Remove the newline character from the input
  input[strlen(input) - 1] = '\0';

  return input;
}
