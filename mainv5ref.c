/**
 * @file main.c
 * @brief Main routine with osTask implementation
 **/

#include <stdlib.h>
#include "stm32h7xx.h"
#include "stm32h7xx_hal.h"
#include "stm32h735g_discovery.h"
#include "stm32h735g_discovery_lcd.h"
#include "stm32_lcd.h"
#include "debug.h"
#include "fatfs.h"
#include "cmsis_os2.h"

// Global variables
uint_t lcdLine = 0;
uint_t lcdColumn = 0;
SD_HandleTypeDef hsd1;
FRESULT res;
uint8_t wtext[] = "STM32 FATFS works great!";
uint8_t rtext[_MAX_SS];
FIL file;

// Task handles
osThreadId_t sdTaskHandle;
const osThreadAttr_t sdTask_attributes = {
    .name = "sdTask",
    .stack_size = 1024,
    .priority = osPriorityNormal,
};

// Function prototypes
static void MX_GPIO_Init(void);
static void MX_SDMMC1_SD_Init(void);
void SD(void);
void Error_Handler(void);

void SystemClock_Config(void) {
    // ... (оставить без изменений)
}

void MPU_Config(void) {
    // ... (оставить без изменений)
}

void HAL_SD_MspInit(SD_HandleTypeDef* hsd) {
    // ... (оставить без изменений)
}

static void MX_SDMMC1_SD_Init(void) {
    // ... (оставить без изменений)
}

void SD(void) {
    HAL_SD_CardStateTypeDef state = HAL_SD_GetCardState(&hsd1);
    if(state != HAL_SD_CARD_TRANSFER) {
        Error_Handler();
    }

    res = f_mount(&SDFatFS, SDPath, 1);
    if(res != FR_OK) {
        if(f_mkfs(SDPath, FM_FAT32, 0, rtext, sizeof(rtext)) != FR_OK) {
            Error_Handler();
        }
        if(f_mount(&SDFatFS, SDPath, 0) != FR_OK) {
            Error_Handler();
        }
    }
    
    if(f_open(&file, "TEST.TXT", FA_WRITE | FA_CREATE_ALWAYS) == FR_OK) {
        UINT bytes_written;
        if(f_write(&file, wtext, strlen((char*)wtext), &bytes_written) != FR_OK || 
           bytes_written != strlen((char*)wtext)) {
            Error_Handler();
        }
        f_close(&file);
    } else {
        Error_Handler();
    }
}

void sdTask(void *argument) {
    // Initialize SD card
    MX_SDMMC1_SD_Init();
    MX_FATFS_Init();
    
    while(1) {
        // Process SD card operations
        SD();
        
        // Delay for 1 second
        osDelay(1000);
    }
}

int main(void) {
    // Hardware initialization
    HAL_Init();
    SystemClock_Config();
    MPU_Config();
    MX_GPIO_Init();
    debugInit(115200);

    // Create SD card task
    sdTaskHandle = osThreadNew(sdTask, NULL, &sdTask_attributes);
    if (sdTaskHandle == NULL) {
        Error_Handler();
    }

    // Start kernel
    osKernelStart();
    
    // Should never reach here
    while(1);
}

static void MX_GPIO_Init(void) {
    // ... (оставить без изменений)
}

void Error_Handler(void) {
    printf("Fatal error!\n");
    while(1) {
        BSP_LED_Toggle(LED2);
        HAL_Delay(100);
    }
}
