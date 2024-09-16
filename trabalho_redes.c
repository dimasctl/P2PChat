#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#include "criptografia.h"

// Este programa vai ler um input no terminal e vai retornar a mesma mensagem,
// mas separando a mensagem em pacotes de 4 bytes, e imprimirá cada pacote em
// uma linha diferente.

char *read_input() {
    // Função para ler o input do terminal independente do tamanho da mensagem
    char *input = NULL;
    size_t len = 0;
    ssize_t read;

    read = getline(&input, &len, stdin);
    if (read == -1) {
        printf("Erro ao ler a mensagem\n");
        exit(1);
    }
    
    return input;
}


// Função secundária que fica contando os segundos enquanto
// o programa está esperando o input do usuário
void *count_seconds(void *arg) {
    int seconds = 0;
    int imprimir = 0;
    while (1) {
        sleep(1);
        seconds++;
        if (imprimir) {
            printf("Segundos: %d\n", seconds);
            imprimir = 0;
        }
    }
}



void print_message(char *message, char *key, char *iv) {
    // Função para imprimir a mensagem em pacotes de 4 bytes
    // Como a mensagem é uma string, cada caractere ocupa 1 byte
    int i = 0;
    // Retirar o \n do final da mensagem
    message[strlen(message)-1] = '\0';

    printf("\n");
    // Separar a mensagem em pacotes de 10 bytes e criptografar cada pacote
    while (i < strlen(message)) {
        unsigned char *ciphertext = (unsigned char *)malloc(strlen(message) + AES_BLOCK_SIZE);
        int ciphertext_len = encrypt_message((unsigned char *)message + i, AES_BLOCK_SIZE, key, iv, ciphertext);
        if (ciphertext_len == -1) {
            printf("Erro ao criptografar a mensagem\n");
            return;
        }

        printf("Pacote %d: ", i / AES_BLOCK_SIZE);
        print_hex(ciphertext, ciphertext_len);
        printf("Pacote %d descriptografado: ", i / AES_BLOCK_SIZE);
        unsigned char *plaintext = (unsigned char *)malloc(ciphertext_len);
        int plaintext_len = decrypt_message(ciphertext, ciphertext_len, key, iv, plaintext);
        if (plaintext_len == -1) {
            printf("Erro ao descriptografar a mensagem\n");
            return;
        }

        // Adicionar o \0 no final da string
        plaintext[plaintext_len] = '\0';
        printf("%s\n\n", plaintext);

        i += AES_BLOCK_SIZE;
        free(ciphertext);
    }


    free(message);
    printf("Fim da mensagem\n");
}
int main() {
    // Iniciar a thread que conta os segundos
    //pthread_t thread;
    //pthread_create(&thread, NULL, count_seconds, NULL);

    // Gerar a chave e o IV
    unsigned char **key_iv = generate_key_iv();

    printf("\n\nDigite a mensagem: ");
    fflush(stdout);
    // Ler a mensagem do usuário e imprimir a mensagem em pacotes de 4 bytes
    char *message = read_input();
    print_message(message, key_iv[0], key_iv[1]);
    return 0;
}
