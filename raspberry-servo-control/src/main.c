#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h> 
#include <poll.h>
#include "UART.h"
#include "servo.h"
#include <termios.h>

#define SERVO_PIN 12  // GPIO12

struct servo_ctx servo;
pthread_t servo_thread;
pthread_t uart_rx_thread;
volatile int running = 1;

void *servo_task(void *arg)
{
    int angle = *((int *)arg);
    servo_set_angle(&servo, angle);
    return NULL;
}

void *uart_rx_task(void *arg)
{
    int uart_fd = *((int *)arg);
    char buffer[256];
    int bytes_read;
    struct pollfd fds[1];
    
    fds[0].fd = uart_fd;
    fds[0].events = POLLIN;
    
    while (running) {
        // Vérifie s'il y a des données à lire avec un timeout de 100ms
        int ret = poll(fds, 1, 100);
        if (ret > 0 && (fds[0].revents & POLLIN)) {
            // Lire les données disponibles
            bytes_read = uart_receive(uart_fd, buffer, sizeof(buffer) - 1);
            if (bytes_read > 0) {
                buffer[bytes_read] = '\0';
                
                // Supprimer les caractères de nouvelle ligne et retour chariot
                // pour avoir uniquement le nombre
                char *clean_str = buffer;
                while (*clean_str) {
                    if (*clean_str == '\r' || *clean_str == '\n') {
                        *clean_str = '\0';
                        break;
                    }
                    clean_str++;
                }
                
                // Convertir en nombre
                int received_angle = atoi(buffer);
                
                // Afficher uniquement si ce n'est pas 0 ou si c'est vraiment "0"
                if (received_angle != 0 || (buffer[0] == '0' && buffer[1] == '\0')) {
                    printf("Angle reçu: %d\n", received_angle);
                    
                    // Mettre à jour l'angle du servo
                    int angle_copy = received_angle;
                    
                    // Arrêt du thread précédent si existant
                    servo.running = 0;
                    if (servo_thread) {
                        pthread_join(servo_thread, NULL);
                    }
                    
                    // Démarrage nouveau thread pour le servo
                    servo.running = 1;
                    if (pthread_create(&servo_thread, NULL, servo_task, &angle_copy) != 0) {
                        perror("pthread_create (servo_thread)");
                    }
                }
            }
        }
    }
    return NULL;
}

int main(void)
{
    int uart_fd;
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

    // Démarrer le thread de réception UART
    if (pthread_create(&uart_rx_thread, NULL, uart_rx_task, &uart_fd) != 0) {
        perror("pthread_create (uart_rx_thread)");
        servo_cleanup(&servo);
        uart_close(uart_fd);
        return EXIT_FAILURE;
    }

    // Message de démarrage
    printf("Programme de contrôle servo lancé\n");
    printf("Réception des angles via UART\n");
    
    // Boucle principale simplifiée - uniquement attente de réception UART
    while(running) {
        // Attendre les commandes UART au lieu de générer des angles
        sleep(1);
    }

    // Nettoyage
    running = 0;
    pthread_join(uart_rx_thread, NULL);
    servo_cleanup(&servo);
    uart_close(uart_fd);
    return 0;
}