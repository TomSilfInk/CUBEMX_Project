/**
  ******************************************************************************
  * @file           : motor_control.h
  * @brief          : Header pour le contrôleur moteur
  ******************************************************************************
  */

  #ifndef MOTOR_CONTROL_H
  #define MOTOR_CONTROL_H
  
  #include <stdint.h>
  
  // Énumération des états
  typedef enum {
    S_INIT = 0,        // État initial
    S_MODE_DISTANCE,   // Mode distance
    S_MODE_UART,       // Mode UART
    S_MODE_TEST,       // Nouveau mode test
    S_DEATH,           // État final
    STATE_NB           // Nombre d'états
  } MotorState;
  
  // Énumération des événements
  typedef enum {
    E_BUTTON_PRESS = 0,  // Appui sur bouton poussoir
    E_UART_COMMAND,      // Commande UART reçue
    E_STOP,              // Arrêt du système
    EVENT_NB             // Nombre d'événements
  } MotorEvent;
  
  // Fonctions publiques
  void MotorControl_Init(void);
  void MotorControl_Update(void);
  void MotorControl_ButtonPressed(void);
  void MotorControl_SetUartAngle(uint8_t angle);
  MotorState MotorControl_GetState(void);
  void Motor_Init(void);
  void Motor_SetPosition(int angle);
  void Motor_Test(void);
  
  #endif /* MOTOR_CONTROL_H */