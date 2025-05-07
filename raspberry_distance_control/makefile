# Définir le compilateur
CC = arm-linux-gnueabihf-gcc

# Définir les flags de compilation
CFLAGS = -Wall -Wextra -g

# Définir les bibliothèques à linker
LIBS = -lgpiod

# Définir les fichiers sources
SRCS = uart_handler.c servo_controller.c distance_angle_converter.c main.c

# Définir le nom de l'exécutable
TARGET = raspberry_distance_control

# Règle par défaut
all: $(TARGET)

# Règle pour compiler l'exécutable
$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS) $(LIBS)

# Règle pour nettoyer les fichiers objets et l'exécutable
clean:
	rm -f $(TARGET) *.o
