#include<openssl/evp.h>
#include<openssl/aes.h>
#include<openssl/rand.h>

#define AES_256_KEY_SIZE 32
#define AES_BLOCK_SIZE 16

void print_hex(unsigned char *data, int len) {
    // Função para imprimir os dados em hexadecimal
    for (int i = 0; i < len; i++) {
        printf("%02x", data[i]);
    }
    printf("\n");
}

unsigned char** generate_key_iv() {
    // Função para gerar uma chave e um IV aleatórios

    unsigned char** key_iv = (unsigned char **)malloc(2 * sizeof(unsigned char *));
    key_iv[0] = (unsigned char *)malloc(AES_256_KEY_SIZE * sizeof(unsigned char));
    key_iv[1] = (unsigned char *)malloc(AES_BLOCK_SIZE * sizeof(unsigned char));

    FILE *file = fopen("chave.txt", "rb");
    if (file) {
        fread(key_iv[0], 1, AES_256_KEY_SIZE, file);
        fread(key_iv[1], 1, AES_BLOCK_SIZE, file);
        printf("Chave lida do arquivo: ");
        print_hex(key_iv[0], AES_256_KEY_SIZE);
        printf("IV lido do arquivo: ");
        print_hex(key_iv[1], AES_BLOCK_SIZE);
        fclose(file);
    }
    else {
        // Se o arquivo não existir, gerar uma chave e um IV aleatórios
        printf("Arquivo 'chave.txt' não encontrado\n");

        if (!RAND_bytes(key_iv[0], AES_256_KEY_SIZE) || !RAND_bytes(key_iv[1], AES_BLOCK_SIZE)) {
            printf("Erro ao inicializar a chave e o IV\n");
            return NULL;
        }

        // Salvar a chave e o IV em um arquivo
        file = fopen("chave.txt", "wb");
        fwrite(key_iv[0], 1, AES_256_KEY_SIZE, file);
        fwrite(key_iv[1], 1, AES_BLOCK_SIZE, file);
        fclose(file);

        printf("Chave gerada: ");
        print_hex(key_iv[0], AES_256_KEY_SIZE);
        printf("IV gerado: ");
        print_hex(key_iv[1], AES_BLOCK_SIZE);
    }

    return key_iv;
}

// Função para criptografar a mensagem usando openssl
int encrypt_message(unsigned char *message, int message_len, unsigned char *key, unsigned char *iv, unsigned char *ciphertext) {
    EVP_CIPHER_CTX *ctx;
    int len;
    int ciphertext_len;

    ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        printf("Erro ao criar o contexto de criptografia\n");
        return -1;
    }

    if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv)) {
        printf("Erro ao inicializar a criptografia\n");
        return -1;
    }

    if (1 != EVP_EncryptUpdate(ctx, ciphertext, &len, message, message_len)) {
        printf("Erro ao criptografar a mensagem\n");
        return -1;
    }
    ciphertext_len = len;

    if (1 != EVP_EncryptFinal_ex(ctx, ciphertext + len, &len)) {
        printf("Erro ao finalizar a criptografia\n");
        return -1;
    }
    ciphertext_len += len;

    EVP_CIPHER_CTX_free(ctx);

    return ciphertext_len;
}

// Função para descriptografar a mensagem usando openssl
int decrypt_message(unsigned char *ciphertext, int ciphertext_len, unsigned char *key, unsigned char *iv, unsigned char *plaintext) {
    EVP_CIPHER_CTX *ctx;
    int len;
    int plaintext_len;

    if (!(ctx = EVP_CIPHER_CTX_new())) {
        printf("Erro ao criar o contexto de criptografia\n");
        return -1;
    }

    if (1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv)) {
        printf("Erro ao inicializar a criptografia\n");
        return -1;
    }

    if (1 != EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len)) {
        printf("Erro ao descriptografar a mensagem\n");
        return -1;
    }
    plaintext_len = len;

    if (1 != EVP_DecryptFinal_ex(ctx, plaintext + len, &len)) {
        printf("Erro ao finalizar a criptografia\n");
        return -1;
    }
    plaintext_len += len;

    EVP_CIPHER_CTX_free(ctx);

    return plaintext_len;
}
