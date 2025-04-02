#include "SR04.h"
#include "stm32f4xx_hal.h"
#include "tim.h"

void SR04_Init(void) {
    // Configurer la broche Trigger en sortie
    GPIO_InitTypeDef GPIO_InitStruct = {0};
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
}

uint32_t SR04_GetDistance(void) {
    uint32_t start_time = 0, stop_time = 0;

    // Envoyer un signal Trigger (10 µs HIGH)
    HAL_GPIO_WritePin(SR04_TRIGGER_PORT, SR04_TRIGGER_PIN, GPIO_PIN_SET);
    HAL_Delay(0.01); // 10 µs
    HAL_GPIO_WritePin(SR04_TRIGGER_PORT, SR04_TRIGGER_PIN, GPIO_PIN_RESET);

    // Attendre que le signal Echo passe à HIGH
    while (HAL_GPIO_ReadPin(SR04_ECHO_PORT, SR04_ECHO_PIN) == GPIO_PIN_RESET);

    // Démarrer le timer
    start_time = __HAL_TIM_GET_COUNTER(&htim2);

    // Attendre que le signal Echo passe à LOW
    while (HAL_GPIO_ReadPin(SR04_ECHO_PORT, SR04_ECHO_PIN) == GPIO_PIN_SET);

    // Arrêter le timer
    stop_time = __HAL_TIM_GET_COUNTER(&htim2);

    // Calculer la durée du signal Echo
    uint32_t time_elapsed = stop_time - start_time;

    // Convertir la durée en distance (vitesse du son = 343 m/s)
    uint32_t distance = (time_elapsed * 0.0343) / 2; // en cm

    return distance;
}