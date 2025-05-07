#ifndef UART_HANDLER_H
#define UART_HANDLER_H

#include <stdint.h>

// Définir le port série et le débit en bauds
#define UART_PORT "/dev/ttyS0"
#define BAUD_RATE 115200

// Fonction pour initialiser l'UART
int uart_init(void);

// Fonction pour lire la distance depuis l'UART
uint32_t uart_read_distance(void);

// Fonction pour envoyer l'angle au STM32
int uart_send_angle(int angle);

// Fonction pour fermer l'UART
void uart_close(void);

#endif