#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <poll.h>
#include <ctype.h>
#include "UART.h"
#include "servo.h"
#include <termios.h>

#define SERVO_PIN 12  // GPIO12
#define MODE_DISTANCE 1
#define MODE_MANUAL 2
#define MODE_STOP 3

struct servo_ctx servo;
volatile int running = 1;
volatile int current_mode = 0;  // Mode actuel (0 = non défini)

// Configuration du terminal pour la lecture sans mise en mémoire tampon
void setup_terminal_input() {
    struct termios new_t, old_t;
    tcgetattr(STDIN_FILENO, &old_t);
    new_t = old_t;
    new_t.c_lflag &= ~(ICANON | ECHO);  // Désactiver l'echo et le mode canonique
    tcsetattr(STDIN_FILENO, TCSANOW, &new_t);
}

// Restaurer les paramètres du terminal
void restore_terminal_input() {
    struct termios old_t;
    tcgetattr(STDIN_FILENO, &old_t);
    old_t.c_lflag |= (ICANON | ECHO);  // Réactiver l'echo et le mode canonique
    tcsetattr(STDIN_FILENO, TCSANOW, &old_t);
}

// Fonction pour afficher le menu principal
void print_menu() {
    printf("\n=== Menu Principal ===\n");
    printf("1. Mode Distance\n");
    printf("2. Mode Manuel\n");
    printf("3. Mode Stop\n");
    printf("Entrez votre choix : ");
    fflush(stdout);
}

// Fonction pour gérer le mode distance
void handle_distance_mode(int uart_fd) {
    printf("\nMode Distance activé. En attente des données de la STM32...\n");
    char buffer[256];
    struct pollfd fds[1];
    fds[0].fd = uart_fd;
    fds[0].events = POLLIN;

    while (current_mode == MODE_DISTANCE) {
        int ret = poll(fds, 1, 100);  // Timeout de 100ms
        if (ret > 0 && (fds[0].revents & POLLIN)) {
            int bytes_read = uart_receive(uart_fd, buffer, sizeof(buffer) - 1);
            if (bytes_read > 0) {
                buffer[bytes_read] = '\0';
                int distance = atoi(buffer);
                printf("Distance reçue : %d\n", distance);

                // Positionner le moteur en fonction de la distance
                int angle = distance % 180;  // Exemple : convertir la distance en angle
                servo_set_angle(&servo, angle);
                printf("Moteur positionné à l'angle : %d\n", angle);
            }
        }
    }
}

// Fonction pour gérer le mode manuel
void handle_manual_mode(int uart_fd) {
    printf("\nMode Manuel activé. Entrez un angle (0-180) : ");
    fflush(stdout);
    char input_buffer[20];
    while (current_mode == MODE_MANUAL) {
        if (fgets(input_buffer, sizeof(input_buffer), stdin)) {
            int angle = atoi(input_buffer);
            if (angle >= 0 && angle <= 180) {
                // Positionner le moteur localement
                servo_set_angle(&servo, angle);
                printf("Moteur positionné à l'angle : %d\n", angle);

                // Envoyer la consigne à la STM32
                char uart_buffer[256];
                snprintf(uart_buffer, sizeof(uart_buffer), "%d\r\n", angle);
                uart_send(uart_fd, uart_buffer, strlen(uart_buffer));
                printf("Consigne envoyée à la STM32 : %d\n", angle);
            } else {
                printf("Angle invalide. Entrez une valeur entre 0 et 180 : ");
            }
        }
    }
}

// Fonction principale pour gérer les modes
void main_loop(int uart_fd) {
    char input_buffer[20];
    while (running) {
        print_menu();
        if (fgets(input_buffer, sizeof(input_buffer), stdin)) {
            int choice = atoi(input_buffer);
            char command[256];

            switch (choice) {
                case MODE_DISTANCE:
                    current_mode = MODE_DISTANCE;
                    snprintf(command, sizeof(command), "MODE:DISTANCE\r\n");
                    uart_send(uart_fd, command, strlen(command));
                    handle_distance_mode(uart_fd);
                    break;

                case MODE_MANUAL:
                    current_mode = MODE_MANUAL;
                    snprintf(command, sizeof(command), "MODE:MANUAL\r\n");
                    uart_send(uart_fd, command, strlen(command));
                    handle_manual_mode(uart_fd);
                    break;

                case MODE_STOP:
                    current_mode = MODE_STOP;
                    snprintf(command, sizeof(command), "MODE:STOP\r\n");
                    uart_send(uart_fd, command, strlen(command));
                    printf("Mode Stop activé. Moteur arrêté.\n");
                    break;

                default:
                    printf("Choix invalide. Veuillez réessayer.\n");
                    break;
            }
        }
    }
}

int main(void) {
    int uart_fd;

    // Initialisation UART
    uart_fd = uart_init("/dev/ttyAMA0", B115200);
    if (uart_fd < 0) {
        fprintf(stderr, "Erreur : Impossible d'initialiser l'UART\n");
        return EXIT_FAILURE;
    }

    // Initialisation Servo
    if (servo_init(&servo, SERVO_PIN) < 0) {
        fprintf(stderr, "Erreur : Impossible d'initialiser le servo moteur\n");
        uart_close(uart_fd);
        return EXIT_FAILURE;
    }

    // Séquencer l'initialisation
    printf("Voulez-vous initialiser le système ? (o/n) : ");
    char confirmation;
    scanf(" %c", &confirmation);
    if (confirmation == 'o' || confirmation == 'O') {
        char init_command[] = "MODE:INIT\r\n";
        uart_send(uart_fd, init_command, strlen(init_command));
        printf("Système initialisé avec succès.\n");
    } else {
        printf("Initialisation annulée. Fermeture du programme.\n");
        uart_close(uart_fd);
        servo_cleanup(&servo);
        return EXIT_SUCCESS;
    }

    // Lancer la boucle principale
    main_loop(uart_fd);

    // Nettoyage
    servo_cleanup(&servo);
    uart_close(uart_fd);
    return EXIT_SUCCESS;
}