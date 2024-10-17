#include "../include/misc.h"
#include "../include/client.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define UUID_LEN 36
#define UUID_FILE "uuid.bin"

char* generate_uuid() {
    srand(time(NULL));

    char* uuid = (char*)malloc(UUID_LEN + 1);

    unsigned int a = rand();
    unsigned int b = rand();
    unsigned int c = rand();
    unsigned int d = rand();

    sprintf(uuid, "%08x-%04x-%04x-%04x-%04x%08x", a, (b >> 16) & 0xffff, b & 0xffff, (c >> 16) & 0xffff, c & 0xffff, d);

    FILE* file = fopen(UUID_FILE, "wb");
    fwrite(uuid, sizeof(char), UUID_LEN, file);
    fclose(file);

    return uuid;
}

char* get_uuid() {
    FILE* file = fopen(UUID_FILE, "rb");
    if (file == NULL) {
        char* uuid = generate_uuid();
        return uuid;
    }

    char* uuid = (char*)malloc(UUID_LEN + 1);
    fread(uuid, sizeof(char), UUID_LEN, file);
    uuid[UUID_LEN] = '\0';

    fclose(file);

    return uuid;
}

void add_client(client_info** head, char* ip, int port, char* id) {
    // if the list is empty, add the client to the head
    if (*head == NULL) {
        *head = (client_info*)malloc(sizeof(client_info));
        (*head)->ip = strdup(ip);
        (*head)->port = port;
        (*head)->id = strdup(id);
        (*head)->next = NULL;
        return;
    }

    // if the client is already in the list, update the ip and port
    client_info* current = *head;
    while (current != NULL) {
        if (strcmp(current->id, id) == 0) {
            current->ip = strdup(ip);
            current->port = port;
            return;
        }
        current = current->next;
    }

    // find the last client in the list
    current = *head;
    while (current->next != NULL) {
        current = current->next;
    }

    // add the client to the end of the list
    current->next = (client_info*)malloc(sizeof(client_info));
    current->next->ip = strdup(ip);
    current->next->port = port;
    current->next->id = strdup(id);
    current->next->next = NULL;
}

client_info* print_clients(client_info* head) {
    int select=0;
    while (1) {
        // clear the screen
        printf("\033[H\033[J");

        // print my uuid
        printf("       Your UUID: %s\n\n", get_uuid());

        // switch color to green
        printf("\033[0;32m");
        printf("                         WELCOME TO\n");
        printf("   ██████╗ ██████╗ ██████╗  ██████╗██╗  ██╗ █████╗ ████████╗\n");
        printf("   ██╔══██╗╚════██╗██╔══██╗██╔════╝██║  ██║██╔══██╗╚══██╔══╝\n");
        printf("   ██████╔╝ █████╔╝██████╔╝██║     ███████║███████║   ██║   \n");
        printf("   ██╔═══╝ ██╔═══╝ ██╔═══╝ ██║     ██╔══██║██╔══██║   ██║   \n");
        printf("   ██║     ███████╗██║     ╚██████╗██║  ██║██║  ██║   ██║   \n");
        printf("   ╚═╝     ╚══════╝╚═╝      ╚═════╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   \n");
        printf("q: Quit; r: Rescan; w/j/↑: Up; s/k/↓: Down; Space/Enter: Select\n");
        printf("\n");

        // switch color to white
        printf("\033[0m");

        client_info* current = head;
        int i=0;
        while (current != NULL) {
            // if the client is selected, print the client in black with green background
            if (select == i) {
                printf("\033[0;30;42m");
            }
            printf("Client %d: %s:%d %s", i, current->ip, current->port, current->id);
            if (strcmp(current->id, get_uuid()) == 0) {
                printf("  (You)  \n");
            } else {
                printf("         \n");
            }
            if (select == i) {
                printf("\033[0m");
            }
            current = current->next;
            i++;
        }

        // set input to receive single character
        system("stty cbreak");
        // read a single character
        char c = getchar();

        if (c == 'q') {
            exit(EXIT_SUCCESS);
        }
        if (c == 's' || c == 'j') {
            select++;
            if (select == i) {
                select = 0;
            }
        }
        if (c == 'w' || c == 'k') {
            select--;
            if (select == -1) {
                select = i - 1;
            }
        }
        if (c == ' ' || c == '\n') {
            // return the selected client
            current = head;
            for (int j=0; j<select; j++) {
                current = current->next;
            }
            printf("\033[0m");
            return current;
        }
        if (c == 'r') {
            send_broadcast();
            printf("\nRescanning...\n");
            sleep(5);
        }
        if (c == '\033') {
            getchar();
            switch(getchar()) {
                case 'A':
                    select--;
                    if (select == -1) {
                        select = i - 1;
                    }
                    break;
                case 'B':
                    select++;
                    if (select == i) {
                        select = 0;
                    }
                    break;
            }
        }
        // set input to receive line
        system("stty cooked");
    }
}

void process_messages(client_info** head, char*** messages_array) {
    char** messages = *messages_array;
    int num_messages = 50;
    // Check if two messages are from the same ip
    for (int i = 0; i < num_messages; i++) {
        if (messages[i] == NULL || strlen(messages[i]) == 0) {
            continue;
        }
        char* ip1 = messages[i];
        char* message1 = messages[i + 1];

        for (int j = i + 1; j < num_messages; j++) {
            if (messages[j] == NULL || strlen(messages[j]) == 0) {
                continue;
            }
            char* ip2 = messages[j];
            char* message2 = messages[j + 1];

            if (strcmp(ip1, ip2) == 0) {
                // Check which message is the client id and which is the port
                char* id;
                char* port;
                if (strlen(message1) == UUID_LEN) {
                    id = message1;
                    port = message2;
                } else {
                    id = message2;
                    port = message1;
                }

                add_client(head, ip1, atoi(port), id);

                // Clear the messages
                messages[i] = NULL;
                messages[i + 1] = NULL;
                messages[j] = NULL;
                messages[j + 1] = NULL;
            }
        }
    }
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

// Function to chat with a client
void chat_with(client_info* current) {
    // clear the screen
    printf("\033[H\033[J");

    // print the client's ip, port, and id
    printf("Client: %s:%d %s\n", current->ip, current->port, current->id); fflush(stdout);

    while (1) {
        // get the message from the user
        char* message = get_input();

        // if the message is ':q', quit the chat
        if (strcmp(message, ":q") == 0) {
            return;
        }

        // send the message to the client
        send_message(message, current->ip, current->port);

        // free the memory allocated by getline
        free(message);
    }
}
