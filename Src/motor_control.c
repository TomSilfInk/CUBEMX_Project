/**
  ******************************************************************************
  * @file           : motor_control.c
  * @brief          : Implémentation de la machine à états pour le moteur
  ******************************************************************************
  */

  #include "SR04.h"
  #include "gpio.h"
  #include "usart.h"
  #include "tim.h"
  #include <stdio.h>
  #include <unistd.h>
  #include <stdint.h>
  #include <stdlib.h>
  #include <string.h>
  #include "motor_control.h"
  #include "utils.h"
  
  #define MAX_BUFFER_SIZE 50
  /* Définition des actions */
  typedef enum {
    A_NOP = 0,       
    A_ON,
    A_INIT,       // Pas d'action
    A_SET_DISTANCE_MODE,    // Activer le mode distance
    A_SET_MANUAL_MODE,        // Activer le mode UART
    A_STOP                  // Arrêter le système
  } MotorAction;
  
  typedef enum {
    S_FORGET = 0,
    S_INIT,        // État initial
    S_MODE_DISTANCE,   // Mode distance
    S_MODE_MANUAL,
    S_DEATH,           // État final
    STATE_NB           // Nombre d'états
  } MotorState;
  
  // Énumération des événements
  typedef enum {
    E_INIT = 0,
    E_SET_MODE_DISTANCE_FROM_UART, // Événement de mode distance
    E_SET_MODE_MANUAL_FROM_UART,
    E_SET_MODE_STOP_FROM_UART,
    EVENT_NB             // Nombre d'événements
  } MotorEvent;
  
  /* Structure de transition */
  typedef struct {
    MotorState destinationState;
    MotorAction action;
  } MotorTransition;
  
  /* Variables privées */
  static MotorState currentState = S_DEATH;
  static uint32_t lastTimeUpdate = 0;  // Pour limiter les mises à jour
  
  /* Matrice de transition - machine à états */
  static MotorTransition stateMatrix[STATE_NB][EVENT_NB] = {
    // État S_INIT
    [S_DEATH][E_INIT] = {S_INIT, A_ON},
    [S_INIT][E_SET_MODE_STOP_FROM_UART] = {S_DEATH, A_STOP},

    // Etat init à distance
    [S_INIT][E_SET_MODE_DISTANCE_FROM_UART] = {S_MODE_DISTANCE, A_SET_DISTANCE_MODE},
    [S_MODE_DISTANCE][E_SET_MODE_MANUAL_FROM_UART] = {S_MODE_MANUAL, A_SET_MANUAL_MODE},
    [S_MODE_DISTANCE][E_SET_MODE_STOP_FROM_UART] = {S_DEATH, A_STOP},
    
    // Etat init à mannuel
    [S_INIT][E_SET_MODE_MANUAL_FROM_UART] = {S_MODE_MANUAL, A_SET_MANUAL_MODE},
    [S_MODE_MANUAL][E_SET_MODE_DISTANCE_FROM_UART] = {S_MODE_DISTANCE, A_SET_DISTANCE_MODE},
    [S_MODE_MANUAL][E_SET_MODE_STOP_FROM_UART] = {S_DEATH, A_STOP},
  };
  
  /* Prototypes des fonctions privées */
static void Motor_run(MotorEvent motorEvent);  
static void Motor_performAction(MotorAction action);

/*Fonction local pour positionner le moteur*/

/*Fonction local pour faire les actions du moteurs*/
static void Motor_start();
static void MotorControl_HandleDistanceMode(void);
static void MotorControl_HandleManualMode(void);
static void Motor_stop();


  /* Implémentation des fonctions publiques pour actionner le moteur */
  
  extern void Motor_ON()
  {
    TRACE("Demarrage moteur\n");
    Motor_run(E_INIT);
  }

  extern void Motor_OFF()
  {
    TRACE("Arret moteur\n");
    Motor_run(E_SET_MODE_STOP_FROM_UART);
  }

  extern void Motor_setDistanceMode()
  {
    TRACE("Mode distance\n");
    Motor_run(E_SET_MODE_DISTANCE_FROM_UART);
  }
  extern void Motor_setManualMode()
  {
    TRACE("Mode manuel\n");
    Motor_run(E_SET_MODE_MANUAL_FROM_UART);
  }

    extern void MotorControl_SetUartAngle(uint8_t angle) 
  {
    // Limiter l'angle entre 0 et 180
    if (angle < 0)
        angle = 0;
    if (angle > 180)
        angle = 180;
  
    uint16_t pulse_width = 5 + (angle * 2) / 45;
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, pulse_width);
  }
  /*-----------------------------------------*/
  /* Fin des actions pour controler le moteur*/


  /*----------------------------------------*/
  /* Fonction privés fonctionnement du moteur*/
  static void Motor_start()
  {
    currentState = S_INIT;
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_RESET);  // Allumer LED verte
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_RESET);  // Éteindre LED bleue
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);    // Allumer LED orange
    char msg_init[MAX_BUFFER_SIZE] = "STM32_MODE_INIT : INIT\r\n";
    HAL_UART_Transmit(&huart2, (uint8_t*)msg_init, strlen(msg_init), HAL_MAX_DELAY);
    TRACE("Demarrage\n");
  }

  static void Motor_stop(){
    currentState = S_DEATH;
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_SET);  // Éteindre LED verte
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_SET);  // Éteindre LED bleue
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);  // Éteindre LED orange
    char msg_stop[MAX_BUFFER_SIZE] = "STM32_MODE_STOP : STOP\r\n";
    HAL_UART_Transmit(&huart2, (uint8_t*)msg_stop, strlen(msg_stop), HAL_MAX_DELAY);
    TRACE("Arret\n");
  }

  static void MotorControl_HandleDistanceMode(void) {
    currentState = S_MODE_DISTANCE;
    // Lire la distance du capteur
    uint32_t distance = SR04_GetDistance();
    int angle;
    // Convertion de la distance en angle

    if(distance >= 25){
      angle = 0; 
      }else if(distance <=5){
        angle = 180;
      }else{
        angle = 180 - ((distance - 5)*180)/20;
    }
    
    // Envoyer la distance via l'UART de manière périodique
    uint32_t currentTime = HAL_GetTick();

    if(currentTime - lastTimeUpdate > 200){
      lastTimeUpdate = currentTime; 
      char msg[MAX_BUFFER_SIZE];

      snprintf(msg,MAX_BUFFER_SIZE,"%d\r\n",angle);

      HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), 100);

    }

  }
  
  static void  MotorControl_HandleManualMode(void) {
    currentState = S_MODE_MANUAL;
    // Lire l'angle de l'UART
    uint8_t uartAngle;
    HAL_UART_Receive(&huart2, &uartAngle, 1, HAL_MAX_DELAY);
  }

  /*-----------------------------------------------------------*/
  /* Fin des fonctions privées pour controler le moteur*/




  /*-------------------------------------------------------------*/

  // Fonction qui devra être appelé une fois que les données auront été reçues via l'uart


  
  /*-----------------------------------------------------------*/
  /*Fin des fonctions pour positionner le moteur*/


  /*-----------------------------------------------------------*/
  /* Fonctions pour faire fonctionner la MAE*/
  static void Motor_performAction(MotorAction action)
  {
    switch(action)
    {
      case A_NOP:
        break;
      case A_ON: 
        Motor_start();
        break;
      case A_SET_DISTANCE_MODE:
        MotorControl_HandleDistanceMode();
        break;
      case A_SET_MANUAL_MODE:
        MotorControl_HandleManualMode();
        break;
      case A_STOP:
        Motor_stop();
        break;
      default:
        TRACE("Unknown action %d, pb de MaE\r\n", action);
        // Pas d'action
        break;
    }
  }
  static void Motor_run(MotorEvent motorEvent)
  {
    MotorTransition* myTrans;
    while(currentState != S_DEATH)
    {
      // Lire l'événement de l'UART
      myTrans = &stateMatrix[currentState][motorEvent];
      if(myTrans->destinationState != S_FORGET)
      {
        Motor_performAction(myTrans->action);
        currentState = myTrans->destinationState;
      }else
      {
        TRACE("Unknown event %d in state %d\r\n", motorEvent, currentState);
      }
    }
  }

  /*-----------------------------------------------------------*/
  /* Fin des fonctions pour faire fonctionner la MAE*/
  