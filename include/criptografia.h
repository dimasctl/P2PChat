#ifndef CRIPTOGRAFIA_H
#define CRIPTOGRAFIA_H
#include <openssl/evp.h>
#include <openssl/rand.h>

#define AES_256_KEY_SIZE 32
#define AES_BLOCK_SIZE 16

void print_hex(unsigned char *arr, int len);
unsigned char** generate_key_iv();
unsigned char* encrypt_message(unsigned char *plaintext, int *len, unsigned char *key, unsigned char *iv);
unsigned char* decrypt_message(unsigned char *ciphertext, int *len, unsigned char *key, unsigned char *iv);
unsigned char* ecdh(size_t *secret_len, int *sock);

#endif
