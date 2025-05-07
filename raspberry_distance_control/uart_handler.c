#include "uart_handler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>

static int uart_fd;

int uart_init(void) {
    struct termios options;

    // Ouvrir le port UART
    uart_fd = open(UART_PORT, O_RDWR | O_NOCTTY | O_NDELAY);
    if (uart_fd == -1) {
        perror("Erreur lors de l'ouverture du port UART");
        return -1;
    }

    // Configurer les options du port UART
    tcgetattr(uart_fd, &options);
    cfsetospeed(&options, BAUD_RATE);
    cfsetispeed(&options, BAUD_RATE);

    options.c_cflag &= ~PARENB;
    options.c_cflag &= ~CSTOPB;
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;
    options.c_cflag &= ~CRTSCTS;
    options.c_cflag |= CREAD | CLOCAL;

    options.c_lflag &= ~ICANON;
    options.c_lflag &= ~ECHO;
    options.c_lflag &= ~ECHOE;
    options.c_lflag &= ~ECHONL;
    options.c_lflag &= ~ISIG;

    options.c_iflag &= ~(IXON | IXOFF | IXANY);
    options.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL);

    options.c_oflag &= ~OPOST;

    if (tcsetattr(uart_fd, TCSANOW, &options) != 0) {
        perror("Erreur lors de la configuration du port UART");
        close(uart_fd);
        return -1;
    }

    return 0;
}

uint32_t uart_read_distance(void) {
    char buffer[256] = 0 ;
    ssize_t bytes_read;
    uint32_t distance;
    char *endptr;

    bytes_read = read(uart_fd, buffer, sizeof(buffer) - 1);

    if (bytes_read > 0) {
        buffer[bytes_read] = '\0';
        distance = strtoul(buffer, &endptr, 10);

        if (*endptr == '\0') {
            return distance;
        } else {
            fprintf(stderr, "Erreur de conversion: %s\n", buffer);
            return 0;
        }
    } else {
        return 0;
    }
}

int uart_send_angle(int angle) {
    char buffer[32];
    sprintf(buffer, "%d\r\n", angle);
    ssize_t bytes_written = write(uart_fd, buffer, strlen(buffer));
    if (bytes_written == -1) {
        perror("Erreur lors de l'écriture sur le port UART");
        return -1;
    }
    return 0;
}

void uart_close(void) {
    close(uart_fd);
}