void vSdCardTask(void *pvParameters) {
    SdMessage_t msg;
    
    for (;;) {
        // Ждем команды из очереди
        if (xQueueReceive(xSdQueue, &msg, portMAX_DELAY) {
            switch (msg.cmd) {
                case SD_WRITE_FILE:
                    sdWriteFile(msg.filename, msg.data, msg.size);
                    break;
                    
                case SD_READ_FILE:
                    sdReadFile(msg.filename, msg.data, msg.size);
                    break;
                    
                case SD_CHECK_STATUS:
                    sdCheckStatus();
                    break;
            }
        }
    }
}