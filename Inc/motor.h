#ifndef MOTOR_H
#define MOTOR_H

#include "main.h"
#include "stm32f4xx_hal.h"

void Motor_Init(void);
void Motor_SetPosition(int angle);
void Motor_Sweep(void);



#endif // MOTOR_H