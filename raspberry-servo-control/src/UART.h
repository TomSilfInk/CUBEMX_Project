#ifndef UART_H
#define UART_H

int uart_init(const char *device, int baudrate);
int uart_send(int fd, const char *data, int length);
int uart_receive(int fd, char *buffer, int length);
void uart_close(int fd);

#endif