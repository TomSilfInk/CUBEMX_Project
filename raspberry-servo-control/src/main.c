#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include "UART.h" // Assurez-vous que ce fichier contient les fonctions uart_init, uart_send, uart_close
#include "servo.h"

#define UART_DEVICE "/dev/ttyAMA0" // Port UART de la Raspberry Pi
#define BAUD_RATE B115200          // Vitesse de communication UART

struct servo_ctx servo;

void print_menu() {
    printf("\n=== Interface de Commande ===\n");
    printf("1. MODE:INIT\n");
    printf("2. MODE:DISTANCE\n");
    printf("3. MODE:MANUAL\n");
    printf("4. MODE:STOP\n");
    printf("5. Quitter\n");
    printf("Entrez votre choix : ");
}

int main() {
    int uart_fd;
    char command[50];
    int choice;

    // Initialisation de l'UART
    uart_fd = uart_init(UART_DEVICE, BAUD_RATE);
    if (uart_fd < 0) {
        fprintf(stderr, "Erreur : Impossible d'initialiser l'UART\n");
        return EXIT_FAILURE;
    }

    // Initialisation du servo moteur
    if (servo_init(&servo, SERVO_PIN) < 0) {
        fprintf(stderr, "Erreur : Impossible d'initialiser le servo moteur\n");
        uart_close(uart_fd);
        return EXIT_FAILURE;
    }

    printf("Interface UART initialisée sur %s à %d baud\n", UART_DEVICE, BAUD_RATE);

    while (1) {
        // Afficher le menu
        print_menu();

        // Lire le choix de l'utilisateur
        if (scanf("%d", &choice) != 1) {
            fprintf(stderr, "Entrée invalide. Veuillez entrer un nombre.\n");
            while (getchar() != '\n'); // Vider le buffer
            continue;
        }

        // Traiter le choix de l'utilisateur
        switch (choice) {
            case 1: // MODE:INIT
                strcpy(command, "MODE:INIT\r\n");
                servo_set_angle(&servo, 90); // Position initiale
                break;
            case 2: // MODE:DISTANCE
                strcpy(command, "MODE:DISTANCE\r\n");
                servo_set_angle(&servo, 45); // Exemple : angle basé sur une distance
                break;
            case 3: // MODE:MANUAL
                strcpy(command, "MODE:MANUAL\r\n");
                printf("Entrez un angle (0-180) : ");
                int angle;
                scanf("%d", &angle);
                if (angle >= 0 && angle <= 180) {
                    servo_set_angle(&servo, angle);
                } else {
                    printf("Angle invalide.\n");
                }
                break;
            case 4: // MODE:STOP
                strcpy(command, "MODE:STOP\r\n");
                servo.running = 0; // Arrêter le servo
                break;
            case 5: // Quitter
                printf("Fermeture de l'interface...\n");
                uart_close(uart_fd);
                servo_cleanup(&servo);
                return EXIT_SUCCESS;
            default:
                printf("Choix invalide. Veuillez réessayer.\n");
                continue;
        }

        // Envoyer la commande via UART
        if (uart_send(uart_fd, command, strlen(command)) < 0) {
            fprintf(stderr, "Erreur : Impossible d'envoyer la commande via UART\n");
        } else {
            printf("Commande envoyée : %s", command);
        }
    }

    // Fermer l'UART et nettoyer le servo
    uart_close(uart_fd);
    servo_cleanup(&servo);
    return EXIT_SUCCESS;
}