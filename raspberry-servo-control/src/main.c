#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "UART.h"
#include "servo.h"
#include <termios.h>

#define SERVO_PIN 12  // GPIO12

struct servo_ctx servo;
pthread_t servo_thread;

void *servo_task(void *arg)
{
    int angle = *((int *)arg);
    servo_set_angle(&servo, angle);
    return NULL;
}

int main(void)
{
    int uart_fd;
    char buffer[256];
    int angle = 0;
    
    // Initialisation UART
    uart_fd = uart_init("/dev/ttyAMA0", B115200);
    if (uart_fd < 0) {
        fprintf(stderr, "Failed to initialize UART\n");
        return EXIT_FAILURE;
    }

    // Initialisation Servo
    if (servo_init(&servo, SERVO_PIN) < 0) {
        fprintf(stderr, "Failed to initialize servo\n");
        uart_close(uart_fd);
        return EXIT_FAILURE;
    }

    // Boucle principale
    while(1) {
        // Mise à jour position servo
        angle = (angle + 45) % 180;  // Incrémente de 45° à chaque fois
        
        // Arrêt du thread précédent si existant
        servo.running = 0;
        if (servo_thread) {
            pthread_join(servo_thread, NULL);
        }
        
        // Démarrage nouveau thread pour le servo
        servo.running = 1;
        if (pthread_create(&servo_thread, NULL, servo_task, &angle) != 0) {
            perror("pthread_create");
            break;
        }

        // Envoi position via UART
        snprintf(buffer, sizeof(buffer), "Position servo: %d degrees\n", angle);
        uart_send(uart_fd, buffer, strlen(buffer));

        sleep(1);
    }

    // Nettoyage
    servo_cleanup(&servo);
    uart_close(uart_fd);
    return 0;
}