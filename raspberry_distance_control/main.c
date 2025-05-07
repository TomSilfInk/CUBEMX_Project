#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include "uart_handler.h"
#include "servo_controller.h"
#include "distance_angle_converter.h"

int main() {
    // Initialisation
    if (uart_init() != 0) {
        fprintf(stderr, "Erreur lors de l'initialisation de l'UART\n");
        return 1;
    }

    if (servo_init() != 0) {
        fprintf(stderr, "Erreur lors de l'initialisation du servo\n");
        uart_close();
        return 1;
    }

    // Boucle principale
    while (1) {
        // Lire la distance depuis l'UART
        uint32_t distance = uart_read_distance();

        // Convertir la distance en angle
        int angle = distance_to_angle(distance);

        // Définir l'angle du servo local
        servo_set_angle(angle);

        // Envoyer l'angle au STM32
        uart_send_angle(angle);

        printf("Distance: %u cm, Angle: %d deg\n", distance, angle);

        usleep(10000); // Attendre 10ms
    }

    // Nettoyage
    uart_close();
    servo_cleanup();

    return 0;
}