/**
 * @file main.c
 * @brief Main routine
 *
 * @section License
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * Copyright (C) 2010-2025 Oryx Embedded SARL. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * @author Oryx Embedded SARL (www.oryx-embedded.com)
 * @version 2.5.0
 **/

//Dependencies
#include <stdlib.h>
#include "stm32h7xx.h"
#include "stm32h7xx_hal.h"
#include "stm32h735g_discovery.h"
#include "stm32h735g_discovery_lcd.h"
#include "stm32_lcd.h"
#include "debug.h"
#include "fatfs.h"

//Global variables
uint_t lcdLine = 0;
uint_t lcdColumn = 0;


static void MX_GPIO_Init(void);
static void MX_SDMMC1_SD_Init(void);


SD_HandleTypeDef hsd1;

FRESULT res; /* FatFs function common result code */
FRESULT resTest;
uint32_t byteswritten, bytesread; /* File write/read counts */
uint8_t wtext[] = "STM32 FATFS works great!"; /* File write buffer */
uint8_t rtext[_MAX_SS];/* File read buffer */
FIL file;



/**
 * @brief System clock configuration
 **/

void SystemClock_Config(void)
{
   RCC_OscInitTypeDef RCC_OscInitStruct = {0};
   RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
   RCC_PeriphCLKInitTypeDef RCC_PeriphClkInitStruct = {0};

   //Supply configuration update enable
   HAL_PWREx_ConfigSupply(PWR_DIRECT_SMPS_SUPPLY);

   //Configure voltage scaling
   __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);
   while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY));

   //Enable HSE oscillator and activate PLL with HSE as source
   RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI48 | RCC_OSCILLATORTYPE_HSE;
   RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
   RCC_OscInitStruct.HSIState = RCC_HSI_OFF;
   RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
   RCC_OscInitStruct.CSIState = RCC_CSI_OFF;
   RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
   RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
   RCC_OscInitStruct.PLL.PLLM = 5;
   RCC_OscInitStruct.PLL.PLLN = 104;
   RCC_OscInitStruct.PLL.PLLFRACN = 0;
   RCC_OscInitStruct.PLL.PLLP = 1;
   RCC_OscInitStruct.PLL.PLLR = 2;
   RCC_OscInitStruct.PLL.PLLQ = 4;
   RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
   RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_2;

   HAL_RCC_OscConfig(&RCC_OscInitStruct);

   //Select PLL as system clock source and configure bus clocks dividers
   RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK |
      RCC_CLOCKTYPE_D1PCLK1 | RCC_CLOCKTYPE_PCLK1 |
      RCC_CLOCKTYPE_PCLK2 | RCC_CLOCKTYPE_D3PCLK1;

   RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
   RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
   RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
   RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
   RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
   RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
   RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

   HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3);

   //Select clock source for RNG peripheral
   RCC_PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RNG;
   RCC_PeriphClkInitStruct.RngClockSelection = RCC_RNGCLKSOURCE_HSI48;

   HAL_RCCEx_PeriphCLKConfig(&RCC_PeriphClkInitStruct);

   //Enable CSI clock
   __HAL_RCC_CSI_ENABLE();
   //Enable SYSCFG clock
   __HAL_RCC_SYSCFG_CLK_ENABLE() ;

   //Enable the I/O compensation cell
   HAL_EnableCompensationCell();
}


/**
 * @brief MPU configuration
 **/

void MPU_Config(void)
{
   MPU_Region_InitTypeDef MPU_InitStruct;

   //Disable MPU
   HAL_MPU_Disable();

   //AHB SRAM2 (no cache)
   MPU_InitStruct.Enable = MPU_REGION_ENABLE;
   MPU_InitStruct.Number = MPU_REGION_NUMBER0;
   MPU_InitStruct.BaseAddress = 0x30004000;
   MPU_InitStruct.Size = MPU_REGION_SIZE_16KB;
   MPU_InitStruct.SubRegionDisable = 0;
   MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL1;
   MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
   MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
   MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
   MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
   MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
   HAL_MPU_ConfigRegion(&MPU_InitStruct);

   //OctoSPI RAM
   MPU_InitStruct.Enable = MPU_REGION_ENABLE;
   MPU_InitStruct.Number = MPU_REGION_NUMBER1;
   MPU_InitStruct.BaseAddress = 0x70000000;
   MPU_InitStruct.Size = MPU_REGION_SIZE_16MB;
   MPU_InitStruct.SubRegionDisable = 0;
   MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL1;
   MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
   MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
   MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
   MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;
   MPU_InitStruct.IsBufferable = MPU_ACCESS_BUFFERABLE;
   HAL_MPU_ConfigRegion(&MPU_InitStruct);

   //LCD frame buffer
   MPU_InitStruct.Enable = MPU_REGION_ENABLE;
   MPU_InitStruct.Number = MPU_REGION_NUMBER2;
   MPU_InitStruct.BaseAddress = 0x70000000;
   MPU_InitStruct.Size = MPU_REGION_SIZE_4MB;
   MPU_InitStruct.SubRegionDisable = 0;
   MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
   MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
   MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
   MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
   MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;
   MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
   HAL_MPU_ConfigRegion(&MPU_InitStruct);

   //Enable MPU
   HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}

void HAL_SD_MspInit(SD_HandleTypeDef* hsd)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if(hsd->Instance == SDMMC1)
    {

        __HAL_RCC_SDMMC1_CLK_ENABLE();
        __HAL_RCC_GPIOC_CLK_ENABLE();
        __HAL_RCC_GPIOD_CLK_ENABLE();


        GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF12_SDMMC1;
        HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);


        GPIO_InitStruct.Pin = GPIO_PIN_2;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
    }
}

static void MX_SDMMC1_SD_Init(void)
{
    hsd1.Instance = SDMMC1;
    hsd1.Init.ClockEdge = SDMMC_CLOCK_EDGE_RISING;
    hsd1.Init.ClockPowerSave = SDMMC_CLOCK_POWER_SAVE_DISABLE;
    hsd1.Init.BusWide = SDMMC_BUS_WIDE_1B;
    hsd1.Init.HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_DISABLE;
    hsd1.Init.ClockDiv = 4;

    HAL_Delay(100);

    if(HAL_SD_Init(&hsd1) != HAL_OK)
    {
        uint32_t error = HAL_SD_GetError(&hsd1);
        Error_Handler();
    }

    // Переключаемся на 4-битный режим
    if(HAL_SD_ConfigWideBusOperation(&hsd1, SDMMC_BUS_WIDE_4B) != HAL_OK)
    {
        Error_Handler();
    }
}

void SD(void){
	HAL_SD_CardStateTypeDef state = HAL_SD_GetCardState(&hsd1);
	    if(state != HAL_SD_CARD_TRANSFER)
	    {
	        //Error_Handler();
	    }

	    // Монтирование файловой системы
	    FRESULT res = f_mount(&SDFatFS, SDPath, 1);
	    if(res != FR_OK)
	    {


	        // Попытка форматирования
	        if(f_mkfs(SDPath, FM_FAT32, 0, rtext, sizeof(rtext)) != FR_OK)
	        {
	            //Error_Handler();
	        }

	        // Повторная попытка монтирования
	        if(f_mount(&SDFatFS, SDPath, 0) != FR_OK)
	        {
	            //Error_Handler();
	        }
	    }
	    if(f_open(&file, "TEST.TXT", FA_WRITE | FA_CREATE_ALWAYS) == FR_OK)
	        {
	            UINT bytes_written;
	            if(f_write(&file, wtext, strlen((char*)wtext), &bytes_written) != FR_OK || bytes_written != strlen((char*)wtext))
	            {
	            	//Error_Handler();
	            }
	            f_close(&file);

	        }
	        else
	        {
	        	//Error_Handler();
	        }
}



void sdTask(void *param)
{
	HAL_SD_CardStateTypeDef state = HAL_SD_GetCardState(&hsd1);
		    if(state != HAL_SD_CARD_TRANSFER)
		    {
		        //Error_Handler();
		    }

		    // Монтирование файловой системы
		    FRESULT res = f_mount(&SDFatFS, SDPath, 1);
		    if(res != FR_OK)
		    {


		        // Попытка форматирования
		        if(f_mkfs(SDPath, FM_FAT32, 0, rtext, sizeof(rtext)) != FR_OK)
		        {
		            //Error_Handler();
		        }

		        // Повторная попытка монтирования
		        if(f_mount(&SDFatFS, SDPath, 0) != FR_OK)
		        {
		            //Error_Handler();
		        }
		    }
		    if(f_open(&file, "TEST.TXT", FA_WRITE | FA_CREATE_ALWAYS) == FR_OK)
		        {
		            UINT bytes_written;
		            if(f_write(&file, wtext, strlen((char*)wtext), &bytes_written) != FR_OK || bytes_written != strlen((char*)wtext))
		            {
		            	//Error_Handler();
		            }
		            f_close(&file);

		        }
		        else
		        {
		        	//Error_Handler();
		        }

   //Endless loop
   while(1)
   {

      //Loop delay
      osDelayTask(100);
   }
}

int_t main(void)
{
    // Инициализация системы
    MPU_Config();
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_SDMMC1_SD_Init();
    MX_FATFS_Init();


    debugInit(115200);
    // Работа с SD-картой


    //Set task parameters
	taskParams = OS_TASK_DEFAULT_PARAMS;
	taskParams.stackSize = 500;
	taskParams.priority = OS_TASK_PRIORITY_NORMAL;
	//Create user task
	taskId = osCreateTask("SD Task", sdTask, NULL, &taskParams);
	//Failed to create the task?
	if(taskId == OS_INVALID_TASK_ID)
	{
	  //Debug message
	  TRACE_ERROR("Failed to create task!\r\n");
	}
	
	//Start the execution of tasks
	osStartKernel();

       //This function should never return
       return 0;

    /*
    xSdQueue = xQueueCreate(10, sizeof(SdMessage_t));
    if(xSdQueue == NULL){
    	Error_Handler();
    }
    xTaskCreate(vSdCardTask,"SD Task",1024,NULL,2,&xSdTaskHandle);

    vTaskStartScheduler();
    while(1)
    {
        //BSP_LED_Toggle(LED1);
        //HAL_Delay(10000);
        //SD();
    }*/
}
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, Detectn_Pin|LCD_DISP_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOE, ARD_D8_Pin|STMOD_17_Pin|STMOD_19_Pin|STMOD_18_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOG, LCD_BL_CTRL_Pin|ARD_D7_Pin|MEMS_LED_Pin|ARD_D4_Pin
                          |ARD_D2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, USER_LED2_Pin|USER_LED1_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(STMOD_20_GPIO_Port, STMOD_20_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOH, LCD_RST_Pin|USB_FS_PWR_EN_Pin, GPIO_PIN_RESET);

  GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_9 |GPIO_PIN_10 |GPIO_PIN_11 |GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_SDMMC1;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_2;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

}
void Error_Handler(void)
{
    printf("Fatal error!\n");
    while(1)
    {
        BSP_LED_Toggle(LED2);
        HAL_Delay(100);
    }
}

