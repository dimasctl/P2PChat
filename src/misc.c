#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define UUID_LEN 36
#define UUID_FILE "uuid.bin"

char* generate_uuid() {
    srand(time(NULL));

    char* uuid = (char*)malloc(UUID_LEN + 1);

    unsigned int a = rand();
    unsigned int b = rand();
    unsigned int c = rand();
    unsigned int d = rand();

    sprintf(uuid, "%08x-%04x-%04x-%04x-%04x%08x", a, (b >> 16) & 0xffff, b & 0xffff, (c >> 16) & 0xffff, c & 0xffff, d);

    FILE* file = fopen(UUID_FILE, "wb");
    fwrite(uuid, sizeof(char), UUID_LEN, file);
    fclose(file);

    return uuid;
}

char* get_uuid() {
    FILE* file = fopen(UUID_FILE, "rb");
    if (file == NULL) {
        char* uuid = generate_uuid();
        return uuid;
    }

    char* uuid = (char*)malloc(UUID_LEN + 1);
    fread(uuid, sizeof(char), UUID_LEN, file);
    uuid[UUID_LEN] = '\0';

    fclose(file);

    return uuid;
}
