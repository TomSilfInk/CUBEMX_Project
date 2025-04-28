#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include "UART.h"

int uart_init(const char *device, int baudrate)
{
    int fd = open(device, O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd == -1) {
        perror("open_port: Unable to open port");
        return -1;
    }

    struct termios options;
    tcgetattr(fd, &options);
    
    // Set baud rate à 115200
    cfsetispeed(&options, B115200);
    cfsetospeed(&options, B115200);
    
    // 8N1 Mode
    options.c_cflag &= ~PARENB;
    options.c_cflag &= ~CSTOPB;
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;
    
        // Désactive le contrôle de flux matériel
    #ifdef CRTSCTS
        options.c_cflag &= ~CRTSCTS;
    #endif
    
    // Active le récepteur et ignore les signaux modem
    options.c_cflag |= CREAD | CLOCAL;
    
    // Configuration pour raw input
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    
    // Configuration pour raw output
    options.c_oflag &= ~OPOST;
    
    // Appliquer les paramètres
    if (tcsetattr(fd, TCSANOW, &options) != 0) {
        perror("tcsetattr failed");
        return -1;
    }

    return fd;
}

int uart_send(int fd, const char *data, int length)
{
    return write(fd, data, length);
}

int uart_receive(int fd, char *buffer, int length)
{
    return read(fd, buffer, length);
}

void uart_close(int fd)
{
    close(fd);
}