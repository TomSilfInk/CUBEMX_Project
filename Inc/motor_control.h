/**
  ******************************************************************************
  * @file           : motor_control.h
  * @brief          : Header pour le contrôleur moteur
  ******************************************************************************
  */

  #ifndef MOTOR_CONTROL_H
  #define MOTOR_CONTROL_H
  
  
  #include <stdint.h>
  
  void extern Motor_ON();
  void extern Motor_OFF();
  void extern Motor_setDistanceMode();
  void extern Motor_setManualMode();
  #endif /* MOTOR_CONTROL_H */