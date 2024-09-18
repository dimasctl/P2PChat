#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#include <pthread.h>

#define PORT 8080

void* server(void* arg);
void* client(void* arg);
void* handle_client(void* arg);
void* receive_message(void* arg);

pthread_t server_thread, client_thread;

int main() {
    system("clear");
    printf("Welcome to P2PChat!\n");
    printf("Would you like to start a connection? (y/n): ");
    fflush(stdout);

    char response;
    scanf("%c", &response);
    getchar();
    // Create a server thread
    if (pthread_create(&server_thread, NULL, server, NULL) != 0) {
        perror("pthread_create");
        exit(EXIT_FAILURE);
    }

    if (response == 'y') {
        // Create a client thread
        if (pthread_create(&client_thread, NULL, client, NULL) != 0) {
            perror("pthread_create");
            exit(EXIT_FAILURE);
        }
        // Wait for the client thread to finish
        pthread_join(client_thread, NULL);
    }
    // Wait for the server thread to finish
    pthread_join(server_thread, NULL);

    return 0;
}


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
        if (pthread_create(&client_thread, NULL, handle_client, &new_socket) != 0) {
            perror("pthread_create");
            exit(EXIT_FAILURE);
        }
    }
    close(server_fd);

    return NULL;
}

void* client(void* arg) {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};
    char hello[1024];

    // Get ip address and port number from the user
    printf("Enter the ip address of the server: ");
    fflush(stdout);
    char ip[16];
    fgets(ip, sizeof(ip), stdin);
    ip[strlen(ip) - 1] = '\0';

    printf("Enter the port number of the server: ");
    fflush(stdout);
    int port;
    scanf("%d", &port);
    getchar();


    // Create a socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return NULL;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return NULL;
    }

    // Connect to server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return NULL;
    }

    printf("Connected to server. You can now send messages to the server.\nType 'exit' to close the connection.\n");

    // create a thread to receive messages from the server
    pthread_t receive_thread;
    if (pthread_create(&receive_thread, NULL, receive_message, &sock) != 0) {
        perror("pthread_create");
        exit(EXIT_FAILURE);
    }

    while (strcmp(hello, "exit") != 0) {
        // Get user input
        fgets(hello, sizeof(hello), stdin);

        // Send message to server
        send(sock, hello, strlen(hello), 0);

        if (strcmp(hello, "exit") == 0) {
            printf("Closing connection...\n");
            break;
        }
    }

    // Close the socket
    close(sock);
    return NULL;
}

void* handle_client(void* arg) {
    int new_socket = *(int*)arg;
    char buffer[1024] = {0};
    char message[1024];
    // Create a thread to receive messages from the client
    pthread_t receive_thread;
    if (pthread_create(&receive_thread, NULL, receive_message, &new_socket) != 0) {
        perror("pthread_create");
        exit(EXIT_FAILURE);
    }
    while (1) {
        // Get user input
        fgets(message, sizeof(message), stdin);

        // Sending a message to the client
        send(new_socket, message, strlen(message), 0);

        if (strcmp(message, "exit") == 0) {
            printf("Closing connection...\n");
            break;
        }
    }

    // Closing the socket
    close(new_socket);
    return NULL;
}

void* receive_message(void* arg) {
    int sock = *(int*)arg;
    char buffer[1024] = {0};
    while (1) {
        // Receive message from server
        int valread = read(sock, buffer, 1024);
        if (valread > 0) {
            buffer[valread] = '\0';
            printf("Received: %s\n", buffer);
        }

        if (strcmp(buffer, "exit") == 0) {
            printf("Closing connection...\n");
            break;
        }

        memset(buffer, 0, sizeof(buffer));
    }

    // Close the socket
    close(sock);
    return NULL;
}
