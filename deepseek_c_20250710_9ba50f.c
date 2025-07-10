void sdWriteFile(const char* filename, const void* data, size_t size) {
    FIL file;
    UINT bytesWritten;
    
    if (f_open(&file, filename, FA_WRITE | FA_CREATE_ALWAYS) == FR_OK) {
        f_write(&file, data, size, &bytesWritten);
        f_close(&file);
        printf("File %s written (%u bytes)\n", filename, bytesWritten);
    } else {
        printf("Error opening %s\n", filename);
    }
}

void sdReadFile(const char* filename, void* buffer, size_t size) {
    FIL file;
    UINT bytesRead;
    
    if (f_open(&file, filename, FA_READ) == FR_OK) {
        f_read(&file, buffer, size, &bytesRead);
        f_close(&file);
        printf("File %s read (%u bytes)\n", filename, bytesRead);
    } else {
        printf("Error reading %s\n", filename);
    }
}

void sdCheckStatus() {
    HAL_SD_CardStateTypeDef state = HAL_SD_GetCardState(&hsd1);
    printf("SD card state: %d\n", state);
}