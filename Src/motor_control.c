/**
  ******************************************************************************
  * @file           : motor_control.c
  * @brief          : Implémentation de la machine à états pour le moteur
  ******************************************************************************
  */

  #include "motor_control.h"
  #include "SR04.h"
  #include "gpio.h"
  #include "usart.h"
  #include "tim.h"
  #include <stdio.h>
  #include <string.h>
  
  /* Définition des actions */
  typedef enum {
    A_NOP = 0,              // Pas d'action
    A_SET_DISTANCE_MODE,    // Activer le mode distance
    A_SET_UART_MODE,        // Activer le mode UART
    A_SET_TEST_MODE,        // Nouvelle action: activer le mode test
    A_UPDATE_MOTOR,         // Mettre à jour la position du moteur
    A_STOP                  // Arrêter le système
  } MotorAction;
  
  /* Structure de transition */
  typedef struct {
    MotorState destinationState;
    MotorAction action;
  } Transition;
  
  /* Variables privées */
  static MotorState currentState = S_INIT;
  static uint8_t uartAngle = 90;           // Position par défaut en mode UART
  static uint32_t lastDistanceUpdate = 0;  // Pour limiter les mises à jour
  static uint8_t buttonDebounceFlag = 0;   // Anti-rebond pour le bouton
  
  /* Matrice de transition - machine à états */
  static Transition stateMatrix[STATE_NB][EVENT_NB] = {
    // État S_INIT
    [S_INIT][E_BUTTON_PRESS] = {S_MODE_DISTANCE, A_SET_DISTANCE_MODE},
    [S_INIT][E_UART_COMMAND] = {S_MODE_UART, A_SET_UART_MODE},
    [S_INIT][E_STOP] = {S_DEATH, A_STOP},
    
    // État S_MODE_DISTANCE
    [S_MODE_DISTANCE][E_BUTTON_PRESS] = {S_MODE_UART, A_SET_UART_MODE},
    [S_MODE_DISTANCE][E_UART_COMMAND] = {S_MODE_UART, A_SET_UART_MODE},
    [S_MODE_DISTANCE][E_STOP] = {S_DEATH, A_STOP},
    
    // État S_MODE_UART
    [S_MODE_UART][E_BUTTON_PRESS] = {S_MODE_TEST, A_SET_TEST_MODE},
    [S_MODE_UART][E_STOP] = {S_DEATH, A_STOP},
    
    // Nouvel état S_MODE_TEST
    [S_MODE_TEST][E_BUTTON_PRESS] = {S_MODE_DISTANCE, A_SET_DISTANCE_MODE},
    [S_MODE_TEST][E_UART_COMMAND] = {S_MODE_TEST, A_NOP}, // Modifié: reste en mode TEST
    [S_MODE_TEST][E_STOP] = {S_DEATH, A_STOP}
  };
  
  /* Prototypes des fonctions privées */
  static void MotorControl_PerformAction(MotorAction action);
  static void MotorControl_ProcessEvent(MotorEvent event);
  static void MotorControl_HandleDistanceMode(void);
  static void MotorControl_HandleUartMode(void);
  static void MotorControl_HandleTestMode(void); // Nouveau prototype
  
  /* Implémentation des fonctions publiques */
  
  // Initialisation du moteur
  void Motor_Init(void)
  {
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
  }
  
  void MotorControl_Init(void) {
    // Initialisation de l'état
    currentState = S_INIT;
    
    // Transition vers l'état par défaut (mode distance)
    MotorControl_ProcessEvent(E_BUTTON_PRESS);
    
    // Initialiser les LEDs pour indiquer le mode
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_SET);    // LED bleue pour mode distance
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_RESET);  // LED verte éteinte
  }
  
  // Fonction pour positionner le moteur
  void Motor_SetPosition(int angle)
  {
    if (angle < 0)
        angle = 0;
    if (angle > 180)
        angle = 180;
  
    uint16_t pulse_width = 5 + (angle * 2) / 45;
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, pulse_width);
  }
  
  void MotorControl_Update(void) {
    // Exécuter les actions spécifiques à l'état courant
    switch (currentState) {
      case S_MODE_DISTANCE:
        MotorControl_HandleDistanceMode();
        break;
      
      case S_MODE_UART:
        MotorControl_HandleUartMode();
        break;
      
      case S_MODE_TEST:
        MotorControl_HandleTestMode(); // Nouveau gestionnaire pour le mode test
        break;
      
      default:
        // Aucune action pour les autres états
        break;
    }
  }
  
  void MotorControl_ButtonPressed(void) {
    // Vérification anti-rebond simple
    if (buttonDebounceFlag == 0) {
      buttonDebounceFlag = 1;
      
      // Déclencher l'événement d'appui sur le bouton
      MotorControl_ProcessEvent(E_BUTTON_PRESS);
      
      // Message de changement de mode
      char msg[50];
      sprintf(msg, "Changement de mode: %s\r\n", 
              (currentState == S_MODE_DISTANCE) ? "DISTANCE" : 
              (currentState == S_MODE_UART) ? "UART" : "TEST");
      HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
      
      // Réinitialiser le drapeau anti-rebond après un délai
      HAL_Delay(200);
      buttonDebounceFlag = 0;
    }
  }
  
  void MotorControl_SetUartAngle(uint8_t angle) {
    // Limiter l'angle entre 0 et 180
    if (angle > 180) angle = 180;
    
    // Stocker l'angle pour le mode UART
    uartAngle = angle;
    
    // Mettre à jour immédiatement la position du moteur
    Motor_SetPosition(uartAngle);
    
    // Si nous sommes en mode DISTANCE, changer de mode
    if (currentState == S_MODE_DISTANCE) {
      MotorControl_ProcessEvent(E_UART_COMMAND);
    } 
    // Sinon, envoyer juste l'angle sans texte
    else {
      // Envoyer uniquement l'angle
      char msg[10];
      sprintf(msg, "%d\r\n", uartAngle);
      HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), 100);
    }
  }
  
  MotorState MotorControl_GetState(void) {
    return currentState;
  }
  
  /* Implémentation des fonctions privées */
  
  static void MotorControl_PerformAction(MotorAction action) {
    switch (action) {
      case A_SET_DISTANCE_MODE:
        // Activer le mode distance
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_SET);    // LED bleue allumée
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_RESET);  // LED verte éteinte
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);  // LED orange éteinte
        
        // Message de confirmation court
        char msg1[] = "Mode: DISTANCE\r\n";
        HAL_UART_Transmit(&huart2, (uint8_t*)msg1, strlen(msg1), 100);
        break;
      
      case A_SET_UART_MODE:
        // Activer le mode UART
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_SET);    // LED verte allumée
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_RESET);  // LED bleue éteinte
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);  // LED orange éteinte
        
        // Positionner le moteur à la dernière valeur UART connue
        Motor_SetPosition(uartAngle);
        
        // Message de confirmation court
        char msg2[50];
        sprintf(msg2, "Mode: UART - Position: %d deg\r\n", uartAngle);
        HAL_UART_Transmit(&huart2, (uint8_t*)msg2, strlen(msg2), 100);
        break;
        
      case A_SET_TEST_MODE:
        // Activer le mode TEST
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);    // LED orange allumée
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_RESET);  // LED verte éteinte
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_RESET);  // LED bleue éteinte
        
        // Message de confirmation court
        char msg3[] = "Mode: TEST - Tous les modules actifs\r\n"
                     "Le moteur tourne, la distance est mesurée et l'UART est actif\r\n"
                     "Tapez 'q' pour quitter, '1'-'3' pour angles fixes\r\n";
        HAL_UART_Transmit(&huart2, (uint8_t*)msg3, strlen(msg3), 100);
        break;
      
      case A_STOP:
        // Arrêter le système
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_RESET);  // Éteindre LED verte
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_RESET);  // Éteindre LED bleue
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);  // Éteindre LED orange
        break;
      
      default:
        // Pas d'action
        break;
    }
  }
  
  static void MotorControl_ProcessEvent(MotorEvent event) {
    // Récupérer la transition pour l'état et l'événement courants
    Transition transition = stateMatrix[currentState][event];
    
    // Vérifier si nous avons une transition valide (destination != état courant)
    if (transition.destinationState != currentState) {
      // Exécuter l'action associée à cette transition
      MotorControl_PerformAction(transition.action);
      
      // Mettre à jour l'état courant
      currentState = transition.destinationState;
    }
  }
  
  static void MotorControl_HandleDistanceMode(void) {
    // Lire la distance du capteur
    uint32_t distance = SR04_GetDistance();
    
    // Calculer l'angle en fonction de la distance
    // Nouvelle plage: 0-25 cm => angle 180-0 degrés
    int angle;
    
    if (distance >= 25) {
      angle = 0; // Distance >= 25cm -> angle min (0)
    } else if (distance <= 5) {
      angle = 180; // Distance <= 5cm -> angle max (180)
    } else {
      // Conversion linéaire: distance -> angle
      // 5cm->180°, 25cm->0° => pente = -9 degrés/cm
      angle = 180 - ((distance - 5) * 180) / 20;
    }
    
    // Positionner le moteur
    Motor_SetPosition(angle);
    
    // Envoyer périodiquement UNIQUEMENT l'angle via UART
    uint32_t currentTime = HAL_GetTick();
    if (currentTime - lastDistanceUpdate > 1000) { // Toutes les secondes
        lastDistanceUpdate = currentTime;
        
        // Envoyer uniquement l'angle
        char msg[10];
        sprintf(msg, "%d\r\n", angle);
        HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    }
  }
  
  static void MotorControl_HandleUartMode(void) {
    // Envoyer périodiquement UNIQUEMENT l'angle
    uint32_t currentTime = HAL_GetTick();
    if (currentTime - lastDistanceUpdate > 1000) { // Toutes les secondes
        lastDistanceUpdate = currentTime;
        
        // Envoyer uniquement l'angle
        char msg[10];
        sprintf(msg, "%d\r\n", uartAngle);
        HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    }
  }
  
  // Nouvelle fonction pour gérer le mode test
  static void MotorControl_HandleTestMode(void) {
    // 1. Faire tourner le moteur (sweep)
    Motor_Test();
    
    // 2. Lire et afficher la distance du capteur
    uint32_t distance = SR04_GetDistance();
    
    // 3. Envoyer périodiquement UNIQUEMENT l'angle actuel
    uint32_t currentTime = HAL_GetTick();
    if (currentTime - lastDistanceUpdate > 100) { // Plus fréquent pour débogage
        lastDistanceUpdate = currentTime;
        
        // Récupérer l'angle actuel du balayage via la fonction d'accès
        int current_angle = Motor_GetTestAngle();
        
        // Envoyer uniquement l'angle
        char msg[10];
        sprintf(msg, "%d\r\n", current_angle);
        HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), 100);
    }
  }
  
  // Déclarez une variable globale pour stocker l'angle actuel du test
static int testCurrentAngle = 0;

void Motor_Test(void)
{
    static int direction = 1;

    testCurrentAngle += direction;

    if (testCurrentAngle >= 180 || testCurrentAngle <= 0)
    {
        direction = -direction;
    }

    Motor_SetPosition(testCurrentAngle);
}

// Fonction pour obtenir l'angle actuel du mode test
int Motor_GetTestAngle(void)
{
    return testCurrentAngle;
}