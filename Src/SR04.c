#include "SR04.h"
#include "stm32f4xx_hal.h"
#include "tim.h"

void SR04_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // Configurer la broche Trigger en sortie
    GPIO_InitStruct.Pin = SR04_TRIGGER_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(SR04_TRIGGER_PORT, &GPIO_InitStruct);

    // Configurer la broche Echo en entrée
    GPIO_InitStruct.Pin = SR04_ECHO_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(SR04_ECHO_PORT, &GPIO_InitStruct);

    // Démarrer le timer
    HAL_TIM_Base_Start(&htim2);
}

uint32_t SR04_GetDistance(void) {
    uint32_t start_time = 0, stop_time = 0;
    uint32_t timeout_start;

    // Envoyer un signal Trigger (10 µs HIGH)
    HAL_GPIO_WritePin(SR04_TRIGGER_PORT, SR04_TRIGGER_PIN, GPIO_PIN_SET);
    __HAL_TIM_SET_COUNTER(&htim2, 0); // Réinitialiser le compteur du Timer 2
    while (__HAL_TIM_GET_COUNTER(&htim2) < 10); // Attendre 10 µs
    HAL_GPIO_WritePin(SR04_TRIGGER_PORT, SR04_TRIGGER_PIN, GPIO_PIN_RESET);

    // Attendre que Echo passe à HIGH avec timeout
    timeout_start = HAL_GetTick();
    while (HAL_GPIO_ReadPin(SR04_ECHO_PORT, SR04_ECHO_PIN) == GPIO_PIN_RESET) {
        if (HAL_GetTick() - timeout_start > 100) return 0; // Timeout après 100ms
    }

    // Démarrer le timer
    start_time = __HAL_TIM_GET_COUNTER(&htim2);

    // Attendre que Echo passe à LOW avec timeout
    timeout_start = HAL_GetTick();
    while (HAL_GPIO_ReadPin(SR04_ECHO_PORT, SR04_ECHO_PIN) == GPIO_PIN_SET) {
        if (HAL_GetTick() - timeout_start > 100) return 0; // Timeout après 100ms
    }

    // Arrêter le timer
    stop_time = __HAL_TIM_GET_COUNTER(&htim2);

    // Vérifier que les valeurs sont valides
    if (start_time >= stop_time) return 0;

    // Calculer la durée du signal Echo
    uint32_t time_elapsed = stop_time - start_time;

    // Logs pour le débogage
    printf("Start: %lu, Stop: %lu, Elapsed: %lu\n", start_time, stop_time, time_elapsed);

    // Convertir la durée en distance (vitesse du son = 343 m/s)
    uint32_t distance = (uint32_t)(((float)time_elapsed * 0.0343f) / 2.0f);

    return distance;
}

