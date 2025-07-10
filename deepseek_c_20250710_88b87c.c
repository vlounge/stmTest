QueueHandle_t xSdQueue;  // Очередь для команд SD-карты
TaskHandle_t xSdTaskHandle;

typedef enum {
    SD_WRITE_FILE,
    SD_READ_FILE,
    SD_CHECK_STATUS
} SdCommand_t;

typedef struct {
    SdCommand_t cmd;
    char filename[32];
    void* data;
    size_t size;
} SdMessage_t;