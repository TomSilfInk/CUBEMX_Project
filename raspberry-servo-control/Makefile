EXEC = executable.elf

# Compilateur
CC = gcc
LD = gcc

# Flags
CCFLAGS += -std=c99 -Wall -pedantic -D_GNU_SOURCE
LDFLAGS += -lgpiod -lpthread

# Dossiers
SRC_DIR = src
BUILD_DIR = build

# Sources et objets
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

# Création du dossier build si nécessaire
$(shell mkdir -p $(BUILD_DIR))

# Règles de compilation
all: $(EXEC)

$(EXEC): $(OBJS)
	$(LD) $(OBJS) -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CCFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR) $(EXEC)

.PHONY: all clean
