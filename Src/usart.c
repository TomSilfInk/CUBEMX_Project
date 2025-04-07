/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    usart.c
  * @brief   This file provides code for the configuration
  *          of the USART instances.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "usart.h"
#include <stdio.h>
#include <string.h>
#include "gpio.h"
/* USER CODE BEGIN 0 */
extern uint8_t rxData;
/* USER CODE END 0 */

UART_HandleTypeDef huart2;

/* USART2 init function */

void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(uartHandle->Instance==USART2)
  {
  /* USER CODE BEGIN USART2_MspInit 0 */

  /* USER CODE END USART2_MspInit 0 */
    /* USART2 clock enable */
    __HAL_RCC_USART2_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**USART2 GPIO Configuration
    PA2     ------> USART2_TX
    PA3     ------> USART2_RX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* USART2 interrupt Init */
    HAL_NVIC_SetPriority(USART2_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USART2_IRQn);
  /* USER CODE BEGIN USART2_MspInit 1 */

  /* USER CODE END USART2_MspInit 1 */
  }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef* uartHandle)
{

  if(uartHandle->Instance==USART2)
  {
  /* USER CODE BEGIN USART2_MspDeInit 0 */

  /* USER CODE END USART2_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART2_CLK_DISABLE();

    /**USART2 GPIO Configuration
    PA2     ------> USART2_TX
    PA3     ------> USART2_RX
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_2|GPIO_PIN_3);

    /* USART2 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USART2_IRQn);
  /* USER CODE BEGIN USART2_MspDeInit 1 */

  /* USER CODE END USART2_MspDeInit 1 */
  }
}

/**
  * @brief  Fonction de test de l'UART avec écho
  * @retval None
  */
 /* USER CODE BEGIN 1 */
/**
  * @brief  Fonction de test de l'UART avec écho
  * @retval None
  */
void UART_Test(void)
{
  // Variables locales pour le test UART
  uint8_t test_buffer[64];
  uint8_t test_char;
  uint8_t test_idx = 0;
  
  // Message d'accueil
  char welcome[] = "\r\n=== Test UART ===\r\n"
                   "Entrez du texte et appuyez sur Entree. "
                   "Entrez 'q' pour quitter.\r\n> ";
  HAL_UART_Transmit(&huart2, (uint8_t*)welcome, strlen(welcome), HAL_MAX_DELAY);
  
  // Désactiver temporairement les interruptions UART existantes
  __HAL_UART_DISABLE_IT(&huart2, UART_IT_RXNE);
  
  while(1)
  {
    // Réinitialiser l'index et le buffer
    test_idx = 0;
    memset(test_buffer, 0, sizeof(test_buffer));
    
    // Lire des caractères jusqu'à CR ou LF
    while(1)
    {
      // Attendre qu'un caractère soit disponible (utilisation des registres directement)
      while(__HAL_UART_GET_FLAG(&huart2, UART_FLAG_RXNE) == RESET);
      
      // Lire le caractère reçu
      test_char = (uint8_t)(huart2.Instance->DR & 0x00FF);
      
      // Afficher l'écho du caractère reçu
      HAL_UART_Transmit(&huart2, &test_char, 1, 100);
      
      // Si c'est un retour chariot ou une nouvelle ligne
      if (test_char == '\r' || test_char == '\n')
      {
        HAL_UART_Transmit(&huart2, (uint8_t*)"\r\n", 2, 100); // Saut de ligne propre
        if (test_idx > 0) // Si on a reçu des caractères avant
          break;
      }
      // Sinon, ajouter au buffer
      else if (test_idx < sizeof(test_buffer) - 1)
      {
        test_buffer[test_idx++] = test_char;
      }
    }
    
    // Terminer la chaîne
    test_buffer[test_idx] = '\0';
    
    // Vérifier si l'utilisateur veut quitter
    if (test_idx == 1 && test_buffer[0] == 'q')
    {
      char exit_msg[] = "\r\nFin du test UART\r\n";
      HAL_UART_Transmit(&huart2, (uint8_t*)exit_msg, strlen(exit_msg), HAL_MAX_DELAY);
      break;
    }
    
    // Afficher la ligne reçue
    char response[100];
    sprintf(response, "Vous avez saisi: %s\r\n> ", test_buffer);
    HAL_UART_Transmit(&huart2, (uint8_t*)response, strlen(response), HAL_MAX_DELAY);
  }
  
  // Réactiver les interruptions UART pour le reste du programme
  __HAL_UART_ENABLE_IT(&huart2, UART_IT_RXNE);
  
  // Redémarrer la réception par interruption
  HAL_UART_Receive_IT(&huart2, (uint8_t*)&rxData, 1);
}
/* USER CODE END 1 */
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
