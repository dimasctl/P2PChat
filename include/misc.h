#ifndef MISC_H
#define MISC_H

typedef struct info {
    // store ip, port, and id
    char* ip;
    int port;
    char* id;
    struct info* next;
} client_info;


char* get_uuid();
void process_messages(client_info** head, char*** messages);
client_info* print_clients(client_info* head);

#endif
