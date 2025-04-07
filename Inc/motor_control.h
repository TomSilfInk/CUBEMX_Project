/**
  ******************************************************************************
  * @file           : motor_control.h
  * @brief          : Contrôleur du moteur avec machine à états
  ******************************************************************************
  */

  #ifndef MOTOR_CONTROL_H
  #define MOTOR_CONTROL_H
  
  #include "main.h"
  #include "stm32f4xx_hal.h"
  #include "SR04.h"
  #include "usart.h"
  
  /* Définition des états possibles du système */
  typedef enum {
    S_INIT = 0,             // État initial
    S_MODE_DISTANCE,        // Mode contrôle par distance
    S_MODE_UART,            // Mode contrôle par UART
    S_DEATH,                // État d'arrêt
    STATE_NB                // Nombre d'états
  } MotorState;
  
  /* Définition des événements possibles */
  typedef enum {
    E_BUTTON_PRESS = 0,     // Appui sur le bouton poussoir
    E_UART_COMMAND,         // Réception commande UART
    E_STOP,                 // Arrêt du système
    EVENT_NB                // Nombre d'événements
  } MotorEvent;
  
  /* Fonctions publiques */

  // Initialisation du moteur
  void Motor_Init(void);

  // Initialisation de la machine à états
  void MotorControl_Init(void);

  // Fonction de mise à jour du moteur
  void Motor_SetPosition(int angle);
  
  // Mise à jour du contrôleur
  void MotorControl_Update(void);
  
  // Notification d'appui sur le bouton
  void MotorControl_ButtonPressed(void);
  
  // Commande UART reçue
  void MotorControl_SetUartAngle(uint8_t angle);
  
  // Obtenir l'état courant
  MotorState MotorControl_GetState(void);

  // Fonction de test
  void Motor_Test(void);
  
  #endif /* MOTOR_CONTROL_H */