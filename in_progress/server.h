#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

void* handle_receive_message(void* arg) {
    int new_socket = *(int*)arg;
    char* buffer;

    // Receive messages bigger than 1024 bytes
    printf("Received message: ");
    // print the whole message
    while (1) {
        buffer = (char*)malloc(1024);
        memset(buffer, 0, 1024);
        int valread = read(new_socket, buffer, 1024);
        if (valread == 0) {
            break;
        }
        printf("%s", buffer);
        free(buffer);
    }
    printf("\n");
    // Closing the socket
    close(new_socket);
    return NULL;
}

// Creates a listening socket and accepts incoming connections
// by creating a new socket for each connection
void* server(void* arg) {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};
    char *hello = "Hello from server";

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

    printf("Server setup complete. Listening on port %d\n", ntohs(address.sin_port));

    // Continuously accept incoming connections
    while (1) {
        // Listening for incoming connections
        printf("Waiting for a connection...\n");
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

        printf("Connection established with the client\n");

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
