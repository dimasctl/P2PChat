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
        (*head)->next = NULL;
        (*head)->ip = ip;
        (*head)->port = port;
        (*head)->id = id;
        return;
    }

    client_info* current = *head;
    while (current->next != NULL) {
        // if the client is already in the list, update the ip and port
        if (strcmp(current->id, id) == 0) {
            current->ip = ip;
            current->port = port;
            return;
        }
        current = current->next;
    }

    // if the client is already in the list, update the ip and port
    if(strcmp(current->id, id) == 0) {
        current->ip = ip;
        current->port = port;
        return;
    }
    // add the client to the end of the list
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
    for (int i = 0; i < num_messages; i += 2) {
        if (messages[i] == NULL || strlen(messages[i]) == 0) {
            break;
        }
        char* ip1 = messages[i];      // IP address in position i
        char* message1 = messages[i + 1];  // Message in position i + 1

        for (int j = i + 2; j < num_messages; j += 2) {
            if (messages[j] == NULL || strlen(messages[j]) == 0) {
                break;
            }
            char* ip2 = messages[j];      // IP address in position j
            char* message2 = messages[j + 1];  // Message in position j + 1

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
