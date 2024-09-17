CC = gcc
CFLAGS = -Wall -Wextra -I/usr/include/openssl
LDFLAGS = -L/usr/lib -lcrypto -lssl
SRC_DIR = ./src
BUILD_DIR = ./build
BIN = $(BUILD_DIR)/trabalho_redes

# Find all .c files in the SRC_DIR
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRCS))

# Target to build the binary
$(BIN): $(BUILD_DIR) $(OBJS) main.c
	$(CC) $(CFLAGS) main.c $(OBJS) -o $(BIN) $(LDFLAGS)

# Rule to compile each .c file into a .o file in the BUILD_DIR
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Rule to create the build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Target to clean the build directory
.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)
