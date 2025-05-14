/**
  ******************************************************************************
  * @file           : motor_control.h
  * @brief          : Header pour le contrôleur moteur
  ******************************************************************************
  */

  #ifndef MOTOR_CONTROL_H
  #define MOTOR_CONTROL_H
  
  
  #include <stdint.h>
  
 extern void Motor_ON();
 extern void Motor_OFF();
 extern void Motor_setDistanceMode();
 extern void Motor_setManualMode();
 extern void MotorControl_SetUartAngle(uint8_t angle);

  #endif /* MOTOR_CONTROL_H */