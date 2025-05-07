#ifndef SERVO_CONTROLLER_H
#define SERVO_CONTROLLER_H

#include <stdint.h>

// Définir la broche GPIO pour le servo
#define SERVO_PIN 18

// Fonction pour initialiser le servo
int servo_init(void);

// Fonction pour définir l'angle du servo
int servo_set_angle(int angle);

// Fonction pour nettoyer et libérer les ressources
void servo_cleanup(void);

#endif