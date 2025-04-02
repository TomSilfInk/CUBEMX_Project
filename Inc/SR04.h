#ifndef SR04_H
#define SR04_H

#include "stm32f4xx_hal.h"
#include <stdint.h>

// Définir les broches utilisées pour le capteur SR04
#define SR04_TRIGGER_PIN GPIO_PIN_1
#define SR04_TRIGGER_PORT GPIOA
#define SR04_ECHO_PIN GPIO_PIN_5
#define SR04_ECHO_PORT GPIOA

// Prototypes des fonctions
void SR04_Init(void);
uint32_t SR04_GetDistance(void);

#endif // SR04_H