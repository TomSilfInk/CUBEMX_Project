#include "motor.h"
#include "tim.h" // Pour accéder aux fonctions liées au timer

void Motor_Init(void)
{
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
}

void Motor_SetPosition(int angle)
{
    if (angle < 0)
        angle = 0;
    if (angle > 180)
        angle = 180;

    uint16_t pulse_width = 5 + (angle * 2) / 45;
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, pulse_width);
}

void Motor_Sweep(void)
{
    static int angle = 0;
    static int direction = 1;

    angle += direction;

    if (angle >= 180 || angle <= 0)
    {
        direction = -direction;
    }

    Motor_SetPosition(angle);
    HAL_Delay(20);
}