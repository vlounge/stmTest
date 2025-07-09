/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    sdmmc.c
  * @brief   This file provides code for the configuration
  *          of the SDMMC instances.
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
#include "sdmmc.h"

/* USER CODE BEGIN 0 */
volatile sd_app_state_e gSD_app_state = SD_APP_STATE_CARD_DETECTED;
//static void _reset_sd_gpios(void);
/* USER CODE END 0 */

SD_HandleTypeDef hsd1;

/* SDMMC1 init function */

void MX_SDMMC1_SD_Init(void)
{
  /* USER CODE BEGIN SDMMC1_Init 0 */
  /* USER CODE END SDMMC1_Init 0 */

  /* USER CODE BEGIN SDMMC1_Init 1 */
   switch(gSD_app_state)
   {
      case SD_APP_STATE_CARD_READY:
      default:
         return; // HAL_OK. нечего инитить, уже все и так готово
      case SD_APP_STATE_CARD_EJECTED:
         if( HAL_GPIO_ReadPin(SD1_D3_GPIO_Port, SD1_D3_Pin) == GPIO_PIN_RESET )
            return; // HAL_ERROR. карты нет, инициализация невозможна
         else
            gSD_app_state = SD_APP_STATE_CARD_DETECTED;
         // и проваливаемся ниже
      case SD_APP_STATE_CARD_DETECTED:
      {
         gSD_app_state = SD_APP_STATE_CARD_TRY_INIT;

         hsd1.Instance = SDMMC1;
         hsd1.Init.ClockEdge = SDMMC_CLOCK_EDGE_RISING;
         hsd1.Init.ClockPowerSave = SDMMC_CLOCK_POWER_SAVE_DISABLE;
         hsd1.Init.BusWide = SDMMC_BUS_WIDE_4B;
         hsd1.Init.HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_DISABLE;
         hsd1.Init.ClockDiv = 0;
         // пытаемся проинитить gpio и SD карту
         if( HAL_SD_Init(&hsd1) == HAL_OK )
            gSD_app_state = SD_APP_STATE_CARD_READY;
         else
         {
            //HAL_SD_DeInit(&hsd1);
            gSD_app_state = SD_APP_STATE_CARD_EJECTED;
         }
      }
   }

#if 0
  /* USER CODE END SDMMC1_Init 1 */
  hsd1.Instance = SDMMC1;
  hsd1.Init.ClockEdge = SDMMC_CLOCK_EDGE_RISING;
  hsd1.Init.ClockPowerSave = SDMMC_CLOCK_POWER_SAVE_DISABLE;
  hsd1.Init.BusWide = SDMMC_BUS_WIDE_4B;
  hsd1.Init.HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_DISABLE;
  hsd1.Init.ClockDiv = 0;
  if (HAL_SD_Init(&hsd1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SDMMC1_Init 2 */
#endif
  /* USER CODE END SDMMC1_Init 2 */

}

void HAL_SD_MspInit(SD_HandleTypeDef* sdHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
  if(sdHandle->Instance==SDMMC1)
  {
  /* USER CODE BEGIN SDMMC1_MspInit 0 */

  /* USER CODE END SDMMC1_MspInit 0 */

  /** Initializes the peripherals clock
  */
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SDMMC;
    PeriphClkInitStruct.SdmmcClockSelection = RCC_SDMMCCLKSOURCE_PLL;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
      Error_Handler();
    }

    /* SDMMC1 clock enable */
    __HAL_RCC_SDMMC1_CLK_ENABLE();

    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    /**SDMMC1 GPIO Configuration
    PC8     ------> SDMMC1_D0
    PC9     ------> SDMMC1_D1
    PC10     ------> SDMMC1_D2
    PC11     ------> SDMMC1_D3
    PC12     ------> SDMMC1_CK
    PD2     ------> SDMMC1_CMD
    */
    GPIO_InitStruct.Pin = SD1_D0_Pin|SD1_D1_Pin|SD1_D2_Pin|SD1_D3_Pin
                          |SD1_CK_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF12_SDIO1;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = SD1_CMD_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF12_SDIO1;
    HAL_GPIO_Init(SD1_CMD_GPIO_Port, &GPIO_InitStruct);

    /* SDMMC1 interrupt Init */
    HAL_NVIC_SetPriority(SDMMC1_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(SDMMC1_IRQn);
  /* USER CODE BEGIN SDMMC1_MspInit 1 */

  /* USER CODE END SDMMC1_MspInit 1 */
  }
}

void HAL_SD_MspDeInit(SD_HandleTypeDef* sdHandle)
{

  if(sdHandle->Instance==SDMMC1)
  {
  /* USER CODE BEGIN SDMMC1_MspDeInit 0 */

  /* USER CODE END SDMMC1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_SDMMC1_CLK_DISABLE();

    /**SDMMC1 GPIO Configuration
    PC8     ------> SDMMC1_D0
    PC9     ------> SDMMC1_D1
    PC10     ------> SDMMC1_D2
    PC11     ------> SDMMC1_D3
    PC12     ------> SDMMC1_CK
    PD2     ------> SDMMC1_CMD
    */
    HAL_GPIO_DeInit(GPIOC, SD1_D0_Pin|SD1_D1_Pin|SD1_D2_Pin|SD1_D3_Pin
                          |SD1_CK_Pin);

    HAL_GPIO_DeInit(SD1_CMD_GPIO_Port, SD1_CMD_Pin);

    /* SDMMC1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(SDMMC1_IRQn);
  /* USER CODE BEGIN SDMMC1_MspDeInit 1 */
  //_reset_sd_gpios();
  /* USER CODE END SDMMC1_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

//void _reset_sd_gpios(void)
//{
//  GPIO_InitTypeDef GPIO_InitStruct = {0};
//
//  __HAL_RCC_GPIOC_CLK_ENABLE();
//  __HAL_RCC_GPIOD_CLK_ENABLE();
//
//  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
//  GPIO_InitStruct.Pull = GPIO_NOPULL;
//
//  GPIO_InitStruct.Pin = SD1_D0_Pin |SD1_D1_Pin | SD1_D2_Pin | SD1_CK_Pin;
//  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
//
//  GPIO_InitStruct.Pin = SD1_CMD_Pin;
//  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
//
//  GPIO_InitStruct.Pin = SD1_D3_Pin;
//  GPIO_InitStruct.Mode = GPIO_MODE_INPUT; //GPIO_MODE_IT_RISING;
//  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
//  HAL_GPIO_Init(SD1_D3_GPIO_Port, &GPIO_InitStruct);
//}

/* USER CODE END 1 */
