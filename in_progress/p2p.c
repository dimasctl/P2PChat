#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFSIZE 1024

void *listen_for_peers(void *arg);
void *connect_to_peer(void *arg);

int main(int argc, char const *argv[]) {
    pthread_t listener_thread, client_thread;

    // Create a thread to listen for incoming connections (as a server)
    if (pthread_create(&listener_thread, NULL, listen_for_peers, NULL) != 0) {
        perror("Failed to create listener thread");
        return 1;
    }

    // Optionally: Allow user to specify a peer's IP address to connect to
    if (argc == 2) {
        char *peer_ip = (char *)argv[1];
        if (pthread_create(&client_thread, NULL, connect_to_peer, (void *)peer_ip) != 0) {
            perror("Failed to create client thread");
            return 1;
        }
        pthread_join(client_thread, NULL); // Wait for client thread to finish
    }

    // Keep the listener thread running
    pthread_join(listener_thread, NULL);

    return 0;
}

// This function listens for incoming connections from peers
void *listen_for_peers(void *arg) {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[BUFSIZE] = {0};
    char *response = "Hello from the listening peer";

    // Create socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        pthread_exit(NULL);
    }

    // Bind to any local address and specified port
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        close(server_fd);
        pthread_exit(NULL);
    }

    // Start listening for incoming connections
    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        close(server_fd);
        pthread_exit(NULL);
    }

    printf("Listening for peers...\n");

    // Accept connections in a loop
    while (1) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
            perror("Accept failed");
            continue;
        }

        // Read message from peer
        read(new_socket, buffer, BUFSIZE);
        printf("Received message: %s\n", buffer);

        // Respond to the peer
        send(new_socket, response, strlen(response), 0);
        printf("Sent response to peer\n");

        // Close the connection
        close(new_socket);
    }

    close(server_fd);
    pthread_exit(NULL);
}

// This function connects to another peer and exchanges messages
void *connect_to_peer(void *arg) {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char *message = "Hello from connecting peer";
    char buffer[BUFSIZE] = {0};
    char *peer_ip = (char *)arg;

    // Create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        pthread_exit(NULL);
    }

    // Define server address of the peer we want to connect to
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert the peer's IP address from text to binary form
    if (inet_pton(AF_INET, peer_ip, &serv_addr.sin_addr) <= 0) {
        printf("Invalid address or address not supported\n");
        close(sock);
        pthread_exit(NULL);
    }

    // Try to connect to the peer
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection failed");
        close(sock);
        pthread_exit(NULL);
    }

    // Send a message to the peer
    send(sock, message, strlen(message), 0);
    printf("Message sent to peer at %s\n", peer_ip);

    // Read response from peer
    read(sock, buffer, BUFSIZE);
    printf("Received response from peer: %s\n", buffer);

    close(sock);
    pthread_exit(NULL);
}

