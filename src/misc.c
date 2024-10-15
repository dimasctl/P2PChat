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
    if (*head == NULL) {
        *head = (client_info*)malloc(sizeof(client_info));
        (*head)->next = NULL;
        (*head)->ip = ip;
        (*head)->port = port;
        (*head)->id = id;
        return;
    }

    client_info* current = *head;
    while (current->next != NULL) {
        if (strcmp(current->next->ip, ip) == 0 && current->next->port == port) {
            return;
        }
        current = current->next;
    }

    current->next = (client_info*)malloc(sizeof(client_info));
    current = current->next;
    current->ip = ip;
    current->port = port;
    current->id = id;
    current->next = NULL;
}

client_info* print_clients(client_info* head) {
    int select=0;
    while (1) {
        // clear the screen
        printf("\033[H\033[J");

        // print my uuid
        printf("My UUID: %s\n", get_uuid());

        client_info* current = head;
        int i=0;
        while (current != NULL) {
            // if the client is selected, print the client in green
            if (select == i) {
                printf("\033[0;32m");
            }
            printf("Client %d: %s:%d %s\n", i, current->ip, current->port, current->id);
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
        // set input to receive line
        system("stty cooked");

        if (c == 'q') {
            exit(EXIT_SUCCESS);
        }
        if (c == 'w' || c == 'h') {
            select++;
            if (select == i) {
                select = 0;
            }
        }
        if (c == 's' || c == 'j') {
            select--;
            if (select == -1) {
                select = i - 1;
            }
        }
        if (c == ' ' || c == '\n') {
            // return the selected client
            current = head->next;
            for (int j=0; j<select; j++) {
                current = current->next;
            }
            return current;
        }
        if (c == 'r') {
            send_broadcast();
            printf("Broadcast sent\n");
            sleep(5);
        }
    }
}

void process_messages(client_info** head, char*** messages_array) {
    char** messages = *messages_array;
    // print all the messages
    printf("----------------------------------------\n");
    for (int i = 0; i < 10; i++) {
        if (messages[i] == NULL || strlen(messages[i]) == 0) {
            break;
        }
        printf("%s\n", messages[i]); fflush(stdout);
    }
    int num_messages = 10;
    for (int i = 0; i < num_messages; i += 2) {
        if (messages[i] == NULL || strlen(messages[i]) == 0) {
            break;
        }
        char* ip1 = messages[i];      // IP address in position i
        char* message1 = messages[i + 1];  // Message in position i + 1
        printf("i: %d - ip1: %s - message1: %s\n", i, ip1, message1); fflush(stdout);

        for (int j = i + 2; j < num_messages; j += 2) {
            if (messages[j] == NULL || strlen(messages[j]) == 0) {
                break;
            }
            char* ip2 = messages[j];      // IP address in position j
            char* message2 = messages[j + 1];  // Message in position j + 1
            printf("j: %d - ip2: %s - message2: %s\n", j, ip2, message2); fflush(stdout);

            // Compare the two IP addresses
            if (ip1 == NULL || ip2 == NULL) {
                break;
            }
            if (strcmp(ip1, ip2) == 0) {
                // check which message is the uuid and which is the port
                char* uuid;
                char* port;
                if (strlen(message1) == UUID_LEN) {
                    uuid = message1;
                    port = message2;
                }
                else {
                    uuid = message2;
                    port = message1;
                }
                // add the client to the list
                add_client(head, ip1, atoi(port), uuid);

                // clear the messages
                messages[i] = NULL;
                messages[i + 1] = NULL;
                messages[j] = NULL;
                messages[j + 1] = NULL;

                break;
            }
        }
    }
}
