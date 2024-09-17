#ifndef CRIPTOGRAFIA_H
#define CRIPTOGRAFIA_H

#define AES_256_KEY_SIZE 32
#define AES_BLOCK_SIZE 16

void print_hex(unsigned char *arr, int len);
unsigned char **generate_key_iv();
int encrypt_message(unsigned char *message, int message_len, unsigned char *key, unsigned char *iv, unsigned char *ciphertext);
int decrypt_message(unsigned char *ciphertext, int ciphertext_len, unsigned char *key, unsigned char *iv, unsigned char *plaintext);

#endif
