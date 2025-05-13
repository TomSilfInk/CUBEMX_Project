#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h> 
#include <poll.h>
#include "UART.h"
#include "servo.h"
#include <termios.h>
#include <ctype.h>  // Pour isdigit()
#include <unistd.h> // Pour sleep()

#define SERVO_PIN 12  // GPIO12
#define MODE_UART_TO_SERVO 1
#define MODE_TERMINAL_TO_UART 2

struct servo_ctx servo;
pthread_t servo_thread;
pthread_t uart_rx_thread;
pthread_t keyboard_thread;
volatile int running = 1;
volatile int current_angle = 0;  // Pour stocker l'angle actuel
volatile int current_mode = MODE_UART_TO_SERVO; // Mode par défaut

void *servo_task(void *arg)
{
    int angle = *((int *)arg);
    servo_set_angle(&servo, angle);
    return NULL;
}

// Fonction pour extraire un nombre d'une chaîne
int extract_number(const char *buffer)
{
    int value = 0;
    int found = 0;
    
    for (int i = 0; buffer[i] != '\0'; i++) {
        if (isdigit(buffer[i])) {
            value = value * 10 + (buffer[i] - '0');
            found = 1;
        } else if (found) {
            // Si on a déjà trouvé des chiffres et qu'on rencontre un non-chiffre, on s'arrête
            break;
        }
    }
    
    return found ? value : -1;
}

// Configuration du terminal pour la lecture sans mise en mémoire tampon
void setup_terminal_input() {
    struct termios new_t, old_t;
    
    // Obtenir les paramètres actuels
    tcgetattr(STDIN_FILENO, &old_t);
    new_t = old_t;
    
    // Désactiver l'echo et le mode canonique
    new_t.c_lflag &= ~(ICANON | ECHO);
    
    // Appliquer les nouveaux paramètres
    tcsetattr(STDIN_FILENO, TCSANOW, &new_t);
}

// Restaurer les paramètres du terminal
void restore_terminal_input() {
    struct termios old_t;
    
    // Obtenir les paramètres actuels
    tcgetattr(STDIN_FILENO, &old_t);
    
    // Réactiver l'echo et le mode canonique
    old_t.c_lflag |= (ICANON | ECHO);
    
    // Appliquer les paramètres restaurés
    tcsetattr(STDIN_FILENO, TCSANOW, &old_t);
}

// Thread pour la saisie clavier
void *keyboard_task(void *arg) {
    int uart_fd = *((int *)arg);
    char c;
    char input_buffer[20] = {0};
    int input_pos = 0;
    char uart_buffer[256];
    
    setup_terminal_input();
    
    while(running) {
        if (read(STDIN_FILENO, &c, 1) > 0) {
            if (c == 'a' || c == 'A') {
                current_mode = MODE_UART_TO_SERVO;
                printf("\nMode 1 activé: Réception UART vers Servo\n");
                snprintf(uart_buffer, sizeof(uart_buffer), "Mode 1 activé: Attente d'angle (1-179)\r\n");
                uart_send(uart_fd, uart_buffer, strlen(uart_buffer));
                input_pos = 0;
                memset(input_buffer, 0, sizeof(input_buffer));
            }
            else if (c == 'b' || c == 'B') {
                current_mode = MODE_TERMINAL_TO_UART;
                printf("\nMode 2 activé: Terminal vers UART\n");
                printf("Entrez un angle (1-179): ");
                fflush(stdout);
                input_pos = 0;
                memset(input_buffer, 0, sizeof(input_buffer));
            }
            else if (current_mode == MODE_TERMINAL_TO_UART) {
                // Mode 2: Traitement de la saisie clavier
                if (c == '\n' || c == '\r') {
                    // Traiter l'entrée complète
                    input_buffer[input_pos] = '\0';
                    int angle = atoi(input_buffer);
                    
                    if (angle >= 1 && angle <= 179) {
                        // Envoyer l'angle via UART
                        snprintf(uart_buffer, sizeof(uart_buffer), "%d\r\n", angle);
                        uart_send(uart_fd, uart_buffer, strlen(uart_buffer));
                        printf("\nAngle %d envoyé via UART\n", angle);
                        printf("Entrez un angle (1-179): ");
                    } else {
                        printf("\nAngle invalide. Entrez une valeur entre 1 et 179: ");
                    }
                    
                    input_pos = 0;
                    memset(input_buffer, 0, sizeof(input_buffer));
                    fflush(stdout);
                }
                else if (c == 127 || c == 8) {  // Backspace
                    if (input_pos > 0) {
                        input_pos--;
                        input_buffer[input_pos] = '\0';
                        printf("\b \b");  // Effacer le caractère à l'écran
                        fflush(stdout);
                    }
                }
                else if (isdigit(c) && input_pos < sizeof(input_buffer) - 1) {
                    input_buffer[input_pos++] = c;
                    printf("%c", c);  // Afficher le caractère tapé
                    fflush(stdout);
                }
            }
        }
        sleep(10000);  // Pause court pour ne pas surcharger le CPU
    }
    
    restore_terminal_input();
    return NULL;
}

void *uart_rx_task(void *arg)
{
    int uart_fd = *((int *)arg);
    char buffer[256];
    char response[256];
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
    pthread_join(keyboard_thread, NULL);
    if (servo_thread) {
        pthread_join(servo_thread, NULL);
    }
    restore_terminal_input();
    servo_cleanup(&servo);
    uart_close(uart_fd);
    return 0;
}