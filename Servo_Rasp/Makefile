EXEC = executable.elf

# Compilateur croisé à utiliser (à adapter)
RASPBERRY_GCC = arm-linux-gnueabihf-gcc

# Sysroot (à adapter)
#RASPBERRY_SYSROOT = /<path>/<to>/<raspberry>/<rootfs>/

# Compilateur.
CC = $(RASPBERRY_GCC)
LD = $(RASPBERRY_GCC)
#CCFLAGS += --sysroot=$(RASPBERRY_SYSROOT)
CCFLAGS += -std=c99 -Wall -pedantic


#Organisation des sources
SRC = 	$(wildcard *.c) 

OBJ = 	$(SRC:.c=.o)


#Regles de compilations
all: $(EXEC)

$(EXEC): $(OBJ)
	$(LD) $^ -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CCFLAGS) -c $< -o $@
	
#Clean Project
clean:
	$(RM) *.o $(EXEC)
