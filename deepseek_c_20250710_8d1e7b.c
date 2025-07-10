void sdTask(void *param)
{
    MX_SDMMC1_SD_Init();
    MX_FATFS_Init();
    uint8_t prev_sd_status = 0;
    uint8_t current_sd_status = 0;
    uint8_t mounted = 0;
    
    // Endless loop
    while(1)
    {
        HAL_SD_CardStateTypeDef state = HAL_SD_GetCardState(&hsd1);
        // Более надежная проверка состояния карты
        current_sd_status = (state == HAL_SD_CARD_TRANSFER || state == HAL_SD_CARD_READY);
        
        // Изменилось ли состояние
        if(current_sd_status != prev_sd_status)
        {
            // Вставлена ли SD карта
            if(current_sd_status)
            {
                // Повторная инициализация только если предыдущая попытка не удалась
                if(HAL_SD_GetState(&hsd1) != HAL_SD_STATE_READY)
                {
                    MX_SDMMC1_SD_Init();
                }
                
                MX_FATFS_Init();
                mounted = (f_mount(&SDFatFS, SDPath, 1) == FR_OK);
            }
            else
            {
                // Деинсталляция
                if(mounted)
                {
                    f_mount(NULL, SDPath, 0);
                    mounted = 0;
                }
                // Деинициализация только если карта была инициализирована
                if(HAL_SD_GetState(&hsd1) != HAL_SD_STATE_RESET)
                {
                    HAL_SD_DeInit(&hsd1);
                }
            }
            prev_sd_status = current_sd_status;
        }

        if(mounted && (HAL_SD_GetCardState(&hsd1) == HAL_SD_CARD_TRANSFER))
        {
            // Добавляем проверку ошибок при операциях с файлом
            FRESULT res = f_open(&file, "TEST.TXT", FA_WRITE | FA_CREATE_ALWAYS);
            if(res == FR_OK)
            {
                UINT bytes_written;
                res = f_write(&file, wtext, strlen((char*)wtext), &bytes_written);
                if(res != FR_OK || bytes_written != strlen((char*)wtext))
                {
                    // Обработка ошибки записи
                    f_close(&file);
                }
                else
                {
                    f_close(&file);
                }
            }
            // Если не удалось открыть файл, просто продолжаем (возможно, карта извлечена)
        }
        
        // Loop delay
        osDelayTask(100);
    }
}