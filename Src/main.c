/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "SR04.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "motor_control.h"
#include "utils.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* Enumération pour les modes de fonctionnement du système */
typedef enum {
  MODE_DISTANCE = 0,  // Mode 1: servo basé sur la distance
  MODE_UART = 1       // Mode 2: servo basé sur les commandes UART
} SystemMode;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
// Définition du bouton poussoir (à ajuster selon votre schéma)
#define BUTTON_PORT GPIOA
#define BUTTON_PIN GPIO_PIN_0
#define MAX_BUFFER_SIZE 50
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
// Variables pour la réception UART
uint8_t rxBuffer[10];
uint8_t rxData;
uint8_t rxIndex = 0;
uint8_t rxComplete = 0;

/* Variables globales du système*/
SystemMode currentMode = MODE_DISTANCE;  // Mode par défaut
uint8_t uartAngle = 90;                 // Position par défaut en mode UART
uint32_t lastDistanceReport = 0;        // Pour envoyer la distance sur UART toutes les secondes
uint32_t lastButtonCheck = 0;           // Pour le debounce du bouton
volatile int running = 1;               // Flag pour le fonctionnement

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void Blink_LEDs(void);
void Send_UART_Message(void);
void Process_UART_Command(void);
void Error_Handler(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
 int main(void)
 {
   /* MCU Configuration--------------------------------------------------------*/
   /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
   HAL_Init();
 
   /* Configure the system clock */
   SystemClock_Config();
 
   /* Initialize all configured peripherals */
   MX_GPIO_Init();
   MX_USART2_UART_Init();
   MX_TIM3_Init();
   MX_TIM2_Init();
   
   /* Initialisation du capteur, du moteur et du contrôleur */
   SR04_Init();         // Initialiser le capteur SR04
   
   /* Configuration de la réception UART par interruption */
   HAL_UART_Receive_IT(&huart2, &rxData, 1);
   
   /* Message d'accueil */
   char welcome[] = "\r\n=== Projet STM32 - Controle de servo ===\r\n"
                    "Mode 1: Capteur SR04 -> bouton poussoir\r\n"
                    "Mode 2: Commande UART -> entrer angle (0-180)\r\n"
                    "Mode 3: TEST -> tous les modules actifs\r\n";
   HAL_UART_Transmit(&huart2, (uint8_t*)welcome, strlen(welcome), HAL_MAX_DELAY);
   
   /*Infinite loop*/
   while (1)
   {
     // Traiter les commandes UART reçues
     if (rxComplete) {
       Process_UART_Command();
       rxComplete = 0;
     }
     
     HAL_Delay(20);
   }
 }

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 192;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
// Fonction conservée (toujours utile pour les tests)
void Blink_LEDs(void)
{
  HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_14); // Toggle LED bleue
  HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_15); // Toggle LED verte
}

// Fonction conservée (toujours utile pour les tests)
void Send_UART_Message(void)
{
  char message[] = "LEDs toggled!\r\n";
  HAL_UART_Transmit(&huart2, (uint8_t*)message, sizeof(message) - 1, HAL_MAX_DELAY);
}



void Process_UART_Command(void)
{
  // Terminer la chaîne de caractères
  rxBuffer[rxIndex] = '\0';

  // Debug
  
  if(strncmp((char*) rxBuffer, "MODE:DISTANCE", strlen("MODE:DISTANCE")) == 0){
    Motor_setDistanceMode(); // Passer en mode distance

  
  }else if(strncmp((char*) rxBuffer, "MODE:MANUAL", strlen("MODE:MANUAL")) == 0){
    Motor_setManualMode(); // Passer en mode manuel


  }else if(strncmp((char*) rxBuffer, "MODE:STOP", strlen("MODE:STOP")) == 0){
    Motor_OFF(); // Arrêter le moteur


  }else if(strncmp((char*) rxBuffer, "MODE:INIT", strlen("MODE:INIT")) == 0){
    Motor_ON(); // Démarrer le moteur
  }


  else{
    char errorMsg[MAX_BUFFER_SIZE];
    snprintf(errorMsg, sizeof(errorMsg), "Erreur, voici la commande reçu : %s\n", rxBuffer);
    HAL_UART_Transmit(&huart2, (uint8_t*)errorMsg, strlen(errorMsg), HAL_MAX_DELAY);
  }
  rxIndex = 0; // Réinitialiser l'index du buffer
  memset(rxBuffer, 0, sizeof(rxBuffer)); // Réinitialiser le buffer
}

// Callback d'interruption UART
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == USART2) {
    // Si on reçoit un retour chariot ou nouvelle ligne, traiter la commande
    if (rxData == '\r' || rxData == '\n') {
      if (rxIndex > 0) {
        rxComplete = 1;
      }
    }
    // Sinon, ajouter le caractère au buffer
    else if (rxIndex < sizeof(rxBuffer) - 1) {
      rxBuffer[rxIndex++] = rxData;
    }
    
    // Relancer la réception
    HAL_UART_Receive_IT(&huart2, &rxData, 1);
  }
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
