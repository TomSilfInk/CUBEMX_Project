#ifndef MOTOR_H
#define MOTOR_H

#include "main.h"
<<<<<<< HEAD

/**
 * @brief Initialise le moteur (servo-moteur).
 */
void Motor_Init(void);

/**
 * @brief Définit la position du servo-moteur.
 * @param angle Angle souhaité (entre 0° et 180°).
 */
void Motor_SetPosition(int angle);

/**
 * @brief Effectue un balayage automatique du servo-moteur entre 0° et 180°.
 */
=======
void Motor_Init(void);
void Motor_SetPosition(int angle);
>>>>>>> Correction
void Motor_Sweep(void);

#endif // MOTOR_H