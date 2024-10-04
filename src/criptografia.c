#include "../include/criptografia.h"

#include <openssl/rand.h>
#include <openssl/err.h>
#include <openssl/conf.h>
#include <openssl/dh.h>
#include <openssl/ssl.h>

#include <arpa/inet.h>

void print_hex(unsigned char *data, int len) {
    // Função para imprimir os dados em hexadecimal
    for (int i = 0; i < len; i++) {
        printf("%02x", data[i]);
        fflush(stdout);
    }
    printf("\n");
    fflush(stdout);
}

// Function to handle errors
void handleErrors(void) {
    ERR_print_errors_fp(stderr);
    abort();
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

// Função para criptografar a mensagem usando aes-256-cbc
unsigned char* encrypt_message(unsigned char *plaintext, int* len, unsigned char* key, unsigned char* iv) {
    EVP_CIPHER_CTX *ctx;
    int c_len = *len + AES_BLOCK_SIZE, f_len = 0;

    unsigned char* ciphertext = (unsigned char *)malloc(c_len);

    // Create and initialize the context
    if(!(ctx = EVP_CIPHER_CTX_new())) handleErrors();

    // Initialize the encryption operation
    if(1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv)) handleErrors();

    // Provide the message to be encrypted, and obtain the encrypted output
    if(1 != EVP_EncryptUpdate(ctx, ciphertext, &c_len, plaintext, *len)) handleErrors();

    // Finalize the encryption
    if(1 != EVP_EncryptFinal_ex(ctx, ciphertext + c_len, &f_len)) handleErrors();
    *len = c_len + f_len;
    
    // Clean up
    EVP_CIPHER_CTX_free(ctx);

    return ciphertext;
}

// Função para descriptografar a mensagem usando openssl
unsigned char* decrypt_message(unsigned char* ciphertext, int* len, unsigned char* key, unsigned char* iv) {
    EVP_CIPHER_CTX *ctx;
    int p_len = *len, f_len = 0;
    unsigned char* plaintext = (unsigned char *)malloc(p_len);

    // Create and initialize the context
    if(!(ctx = EVP_CIPHER_CTX_new())) handleErrors();

    // Initialize the decryption operation
    if(1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv)) handleErrors();
    EVP_CIPHER_CTX_set_padding(ctx, 0);

    // Provide the ciphertext to be decrypted, and obtain the plaintext output
    if(1 != EVP_DecryptUpdate(ctx, plaintext, &p_len, ciphertext, *len)) handleErrors();

    // Finalize the decryption
    if(1 != EVP_DecryptFinal_ex(ctx, plaintext + p_len, &f_len)) handleErrors();

    *len = p_len + f_len;

    // Clean up
    EVP_CIPHER_CTX_free(ctx);

    return plaintext;
}

// Exchange public keys
EVP_PKEY* get_peerkey(int *sock, EVP_PKEY *pkey) {
    unsigned char* local_key = NULL;
    int local_key_len;

    // Encode the public key to DER format
    local_key_len = i2d_PUBKEY(pkey, &local_key);

    // Send the public key to the other peer
    send(*sock, &local_key_len, sizeof(int), 0);
    send(*sock, local_key, local_key_len, 0);

    // Free the memory
    free(local_key);

    // Get the peer's public key
    recv(*sock, &local_key_len, sizeof(int), 0);
    unsigned char* peer_key = (unsigned char*)malloc(local_key_len);
    recv(*sock, peer_key, local_key_len, 0);

    // Decode the public key
    const unsigned char* p = peer_key;
    EVP_PKEY* peerkey = d2i_PUBKEY(NULL, &p, local_key_len);

    // Free the memory
    free(peer_key);

    return peerkey;
}

// Elliptic Curve Diffie-Hellman key exchange (ECDH)
unsigned char *ecdh(size_t *secret_len, int *sock)
{
	EVP_PKEY_CTX *pctx, *kctx;
	EVP_PKEY_CTX *ctx;
	unsigned char *secret;
	EVP_PKEY *pkey = NULL, *peerkey, *params = NULL;
	/* NB: assumes pkey, peerkey have been already set up */

	/* Create the context for parameter generation */
	if(NULL == (pctx = EVP_PKEY_CTX_new_id(EVP_PKEY_EC, NULL))) handleErrors();

	/* Initialise the parameter generation */
	if(1 != EVP_PKEY_paramgen_init(pctx)) handleErrors();

	/* We're going to use the ANSI X9.62 Prime 256v1 curve */
	if(1 != EVP_PKEY_CTX_set_ec_paramgen_curve_nid(pctx, NID_X9_62_prime256v1)) handleErrors();

	/* Create the parameter object params */
	if (!EVP_PKEY_paramgen(pctx, &params)) handleErrors();

	/* Create the context for the key generation */
	if(NULL == (kctx = EVP_PKEY_CTX_new(params, NULL))) handleErrors();

	/* Generate the key */
	if(1 != EVP_PKEY_keygen_init(kctx)) handleErrors();
	if (1 != EVP_PKEY_keygen(kctx, &pkey)) handleErrors();

  // Get the public key from the other peer
  peerkey = get_peerkey(sock, pkey);

	/* Create the context for the shared secret derivation */
	if(NULL == (ctx = EVP_PKEY_CTX_new(pkey, NULL))) handleErrors();

	/* Initialise */
	if(1 != EVP_PKEY_derive_init(ctx)) handleErrors();

	/* Provide the peer public key */
	if(1 != EVP_PKEY_derive_set_peer(ctx, peerkey)) handleErrors();

	/* Determine buffer length for shared secret */
	if(1 != EVP_PKEY_derive(ctx, NULL, secret_len)) handleErrors();

	/* Create the buffer */
	if(NULL == (secret = OPENSSL_malloc(*secret_len))) handleErrors();

	/* Derive the shared secret */
	if(1 != (EVP_PKEY_derive(ctx, secret, secret_len))) handleErrors();

	EVP_PKEY_CTX_free(ctx);
	EVP_PKEY_free(peerkey);
	EVP_PKEY_free(pkey);
	EVP_PKEY_CTX_free(kctx);
	EVP_PKEY_free(params);
	EVP_PKEY_CTX_free(pctx);

	/* Never use a derived secret directly. Typically it is passed
	 * through some hash function to produce a key */
	return secret;
}
