#include "../include/criptografia.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#include <pthread.h>


// Function to measure time taken between two points
#include <time.h>
#include <sys/time.h>
struct timeval starta;
void get_time_dif(struct timeval* start) {
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

int send_message(char* message, char* ip, int port) {
    int sock = 0;
    struct sockaddr_in serv_addr;

    // Create a socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return 1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return 1;
    }

    // Connect to server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return 1;
    }

    // Generate the key and IV
    size_t shared_key_len = AES_256_KEY_SIZE;
    unsigned char iv[AES_BLOCK_SIZE];
    unsigned char* key = ecdh(&shared_key_len, &sock);
    RAND_bytes(iv, AES_BLOCK_SIZE);

    // Encrypt the message
    int message_size = strlen(message) + 1;
    unsigned char* ciphertext = encrypt_message((unsigned char*)message, &message_size, key, iv);

    // Send the IV
    send(sock, iv, AES_BLOCK_SIZE, 0);

    // Send message size to server
    message_size = htonl(message_size);
    send(sock, &message_size, sizeof(int), 0);


    // Send message to server
    send(sock, ciphertext, message_size, 0);
    fflush(stdout);

    // Close the socket
    close(sock);
    return 0;
}

void send_broadcast() {
    // Create a socket
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        printf("Error creating socket\n");
        return;
    }

    // Enable broadcast
    int broadcast = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast)) < 0) {
        printf("Error setting broadcast\n");
        return;
    }

    // Set the server address
    struct sockaddr_in server;
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(12345);
    server.sin_addr.s_addr = inet_addr("255.255.255.255");

    // Send the message
    char message[] = "Discovery";
    sendto(sock, message, strlen(message), 0, (struct sockaddr*)&server, sizeof(server));

    // Close the socket
    close(sock);
    return;
}
