#include "motor.h"
#include "tim.h" // Pour accéder aux fonctions liées au timer

<<<<<<< HEAD
<<<<<<< HEAD
/**
 * @brief Initialise le moteur (servo-moteur).
 * Configure le timer et démarre le signal PWM.
 */
void Motor_Init(void)
{
    // Démarrer le signal PWM pour le servo-moteur
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
}

/**
 * @brief Définit la position du servo-moteur.
 * @param angle Angle souhaité (entre 0° et 180°).
 */
void Motor_SetPosition(int angle)
{
    // Vérifier que l'angle est valide
=======
=======
>>>>>>> f6d5b62d867fac1ec06c554a7e4a381785e19f46
void Motor_Init(void)
{
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
}

void Motor_SetPosition(int angle)
{
<<<<<<< HEAD
>>>>>>> Correction
=======
>>>>>>> f6d5b62d867fac1ec06c554a7e4a381785e19f46
    if (angle < 0)
        angle = 0;
    if (angle > 180)
        angle = 180;

<<<<<<< HEAD
<<<<<<< HEAD
    // Calculer la largeur d'impulsion correspondante
    uint16_t pulse_width = 5 + (angle * 2) / 45; // Conversion angle -> pulse_width

    // Appliquer la nouvelle position au servo
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, pulse_width);
}

/**
 * @brief Effectue un balayage automatique du servo-moteur entre 0° et 180°.
 */
void Motor_Sweep(void)
{
    static int angle = 0;       // Angle actuel
    static int direction = 1;  // Direction de variation (1 = augmenter, -1 = diminuer)

    // Mettre à jour l'angle
    angle += direction;

    // Inverser la direction si on atteint les limites (0° ou 180°)
=======
=======
>>>>>>> f6d5b62d867fac1ec06c554a7e4a381785e19f46
    uint16_t pulse_width = 5 + (angle * 2) / 45;
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, pulse_width);
}

void Motor_Sweep(void)
{
    static int angle = 0;
    static int direction = 1;

    angle += direction;

<<<<<<< HEAD
>>>>>>> Correction
=======
>>>>>>> f6d5b62d867fac1ec06c554a7e4a381785e19f46
    if (angle >= 180 || angle <= 0)
    {
        direction = -direction;
    }

<<<<<<< HEAD
<<<<<<< HEAD
    // Appliquer la nouvelle position
    Motor_SetPosition(angle);

    // Attendre un moment pour que le servo atteigne la position
    HAL_Delay(20); // 20 ms (ajustez si nécessaire)
=======
    Motor_SetPosition(angle);
    HAL_Delay(20);
>>>>>>> Correction
=======
    Motor_SetPosition(angle);
    HAL_Delay(20);
>>>>>>> f6d5b62d867fac1ec06c554a7e4a381785e19f46
}