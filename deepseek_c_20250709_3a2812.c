/**
 * @file main.c
 * @brief Main routine with SD card fixes
 */

// ... [остальные include и объявления остаются без изменений]

void HAL_SD_MspInit(SD_HandleTypeDef* hsd)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if(hsd->Instance == SDMMC1)
    {
        // Включаем тактирование SDMMC1 и GPIO
        __HAL_RCC_SDMMC1_CLK_ENABLE();
        __HAL_RCC_GPIOC_CLK_ENABLE();
        __HAL_RCC_GPIOD_CLK_ENABLE();

        // Настройка пинов данных и CLK (PC8-PC12)
        GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF12_SDMMC1;
        HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

        // Настройка пина CMD (PD2) с подтяжкой
        GPIO_InitStruct.Pin = GPIO_PIN_2;
        GPIO_InitStruct.Pull = GPIO_PULLUP;  // Важно для стабильной работы!
        HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
    }
}

static void MX_SDMMC1_SD_Init(void)
{
    hsd1.Instance = SDMMC1;
    hsd1.Init.ClockEdge = SDMMC_CLOCK_EDGE_RISING;
    hsd1.Init.ClockPowerSave = SDMMC_CLOCK_POWER_SAVE_DISABLE;
    hsd1.Init.BusWide = SDMMC_BUS_WIDE_1B;  // Сначала 1-битный режим
    hsd1.Init.HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_DISABLE;
    hsd1.Init.ClockDiv = 4;  // Делитель частоты (начните с 4, можно увеличить при проблемах)

    // Даем карте время на инициализацию
    HAL_Delay(100);
    
    if(HAL_SD_Init(&hsd1) != HAL_OK)
    {
        uint32_t error = HAL_SD_GetError(&hsd1);
        printf("SD Init Error: 0x%lX\n", error);
        Error_Handler();
    }

    // Переключаемся на 4-битный режим
    if(HAL_SD_ConfigWideBusOperation(&hsd1, SDMMC_BUS_WIDE_4B) != HAL_OK)
    {
        printf("Failed to switch to 4-bit mode\n");
        Error_Handler();
    }
}

int_t main(void)
{
    // Инициализация системы (без изменений)
    MPU_Config();
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_SDMMC1_SD_Init();
    MX_FATFS_Init();
    SCB_EnableICache();
    SCB_EnableDCache();

    // Инициализация периферии
    debugInit(115200);
    BSP_LED_Init(LED1);
    BSP_LED_Init(LED2);

    // Работа с SD-картой
    printf("Initializing SD card...\n");
    
    // Проверка состояния карты
    HAL_SD_CardStateTypeDef state = HAL_SD_GetCardState(&hsd1);
    if(state != HAL_SD_CARD_TRANSFER)
    {
        printf("Card not ready. State: %d\n", state);
        Error_Handler();
    }

    // Монтирование файловой системы
    FRESULT res = f_mount(&SDFatFS, SDPath, 1);
    if(res != FR_OK)
    {
        printf("Mount failed (error %d). Trying to format...\n", res);
        
        // Попытка форматирования
        if(f_mkfs(SDPath, FM_FAT32, 0, rtext, sizeof(rtext)) != FR_OK)
        {
            printf("Format failed. Card error.\n");
            Error_Handler();
        }
        
        // Повторная попытка монтирования
        if(f_mount(&SDFatFS, SDPath, 0) != FR_OK)
        {
            printf("Mount after format failed.\n");
            Error_Handler();
        }
    }

    printf("SD card ready!\n");

    // Работа с файлом
    FIL file;
    if(f_open(&file, "TEST.TXT", FA_WRITE | FA_CREATE_ALWAYS) == FR_OK)
    {
        UINT bytes_written;
        if(f_write(&file, wtext, strlen((char*)wtext), &bytes_written) != FR_OK || bytes_written != strlen((char*)wtext))
        {
            printf("Write failed\n");
        }
        f_close(&file);
        printf("File written successfully\n");
    }
    else
    {
        printf("Failed to open file\n");
    }

    // Основной цикл
    while(1)
    {
        BSP_LED_Toggle(LED1);
        HAL_Delay(500);
    }
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