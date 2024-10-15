#include "../include/criptografia.h"
#include "../include/misc.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>

#include <pthread.h>

void* listen_broadcast(void* arg);
extern client_info* client_list;

// Function to measure time taken between two points
#include <time.h>
#include <sys/time.h>
struct timeval start;
void get_time_diff(struct timeval* start) {
    struct timeval end;
    gettimeofday(&end, NULL);
    // Calculate the time taken. If it is bigger than 1 second, print the time in seconds
    if (end.tv_sec - start->tv_sec > 0) {
        printf("%ld seconds\n", end.tv_sec - start->tv_sec);
    } else {
        printf("%ld microseconds\n", end.tv_usec - start->tv_usec);
    }
    gettimeofday(start, NULL);
}

void* handle_receive_message(void* arg) {
    int new_socket = *(int*)arg;
    unsigned char* buffer;

    // Receive the key
    size_t shared_key_len = AES_256_KEY_SIZE;
    unsigned char* key = ecdh(&shared_key_len, &new_socket);

    // Receive the IV
    unsigned char iv[AES_BLOCK_SIZE];
    read(new_socket, iv, AES_BLOCK_SIZE);

    // Receive the message length
    int message_len;
    read(new_socket, &message_len, sizeof(int));
    message_len = ntohl(message_len);

    // Allocate memory for the message
    buffer = (unsigned char*)malloc(message_len * sizeof(unsigned char));

    // Receive the message
    read(new_socket, buffer, message_len);

    // Decrypt the message
    char* decrypted_message = (char*)decrypt_message(buffer, &message_len, key, iv);

    // Print the decrypted message
    printf("Message received: %s\n", decrypted_message);

    // Closing the socket
    close(new_socket);
    return NULL;
}

// Creates a listening socket and accepts incoming connections
// by creating a new socket for each connection
void* server() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Attaching socket to the port
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    // Automatically assign a port number
    address.sin_port = htons(0);

    // Bind the socket to the port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Get the port number assigned to the server
    getsockname(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);

    // start broadcasting listener
    int port = (int)ntohs(address.sin_port);
    pthread_t broadcast_thread;
    if (pthread_create(&broadcast_thread, NULL, listen_broadcast, &port) != 0) {
        perror("pthread_create");
        exit(EXIT_FAILURE);
    }

    printf("Server setup complete. Listening on port %d\n", ntohs(address.sin_port));

    // Continuously accept incoming connections
while (1) {
        // Listening for incoming connections
        if (listen(server_fd, 3) < 0) {
            perror("listen failed");
            exit(EXIT_FAILURE);
        }

        // Accepting incoming connection
        printf("Accepting incoming connection from the client at %s:%d\n", inet_ntoa(address.sin_addr), ntohs(address.sin_port));
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
            perror("accept failed");
            exit(EXIT_FAILURE);
        }


        // Create a thread to handle the client
        pthread_t client_thread;
        if (pthread_create(&client_thread, NULL, handle_receive_message, &new_socket) != 0) {
            perror("pthread_create");
            exit(EXIT_FAILURE);
        }
    }
    close(server_fd);

    return NULL;
}

void* listen_broadcast(void* arg) {
    int port = *(int*)arg;
    char port_str[6];
    sprintf(port_str, "%d", port);
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        printf("Error creating socket\n");
        return NULL;
    }

    // Configure the receiver address
    struct sockaddr_in receiver;
    receiver.sin_family = AF_INET;
    receiver.sin_port = htons(12345);
    receiver.sin_addr.s_addr = htonl(INADDR_ANY);

    // Bind the socket to the port
    if (bind(sock, (struct sockaddr*)&receiver, sizeof(receiver)) < 0) {
        printf("Error binding socket\n");
        close(sock);
        return NULL;
    }

    // Continuously listen for broadcast messages
    char** messages = (char**)malloc(10 * sizeof(char*));
    int pointer = 0;
    while (1) {
        char buffer[1024];
        struct sockaddr_in sender;
        socklen_t sender_len = sizeof(sender);

        // Receive the broadcast message
        int len = recvfrom(sock, buffer, 1024, 0, (struct sockaddr*)&sender, &sender_len);
        buffer[len] = '\0';

        if (strcmp(buffer, "Discovery") == 0) {
            char* message = get_uuid();
            // Send a response to the sender on the port 12345
            sender.sin_port = htons(12345);
            sendto(sock, port_str, strlen(port_str), 0, (struct sockaddr*)&sender, sender_len);
            sendto(sock, message, strlen(message), 0, (struct sockaddr*)&sender, sender_len);
        }
        else {
            // Save the IP address and message on an array
            char* ip = inet_ntoa(sender.sin_addr);
            char* message = (char*)malloc(len + 1);
            strcpy(message, buffer);
            message[len] = '\0';
            messages[pointer] = ip;
            messages[pointer +1] = message;
            pointer = (pointer + 2) % 10;

            // check if there are 2 messages from the same IP
            process_messages(&client_list, messages);
        }
    }
    close(sock);
    return NULL;
}
