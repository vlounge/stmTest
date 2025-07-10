/* Includes */
#include "usb_host.h"
#include "ff.h"
#include "sdmmc.h"

/* Global variables */
FATFS fsSd;  // SD card filesystem
FATFS fsUsb; // USB flash drive filesystem
char SDPath[4] = "0:/";
char USBPath[4] = "1:/";
ApplicationTypeDef Appli_state = APPLICATION_IDLE;

/* USB Host Callbacks */
void MX_USB_HOST_Process(void);

/**
  * @brief  Copy files from SD to USB
  */
void CopySDtoUSB(void)
{
    DIR dir;
    FILINFO fno;
    FIL fsrc, fdst;
    uint8_t buffer[4096];
    UINT br, bw;
    
    // Open SD directory
    if(f_opendir(&dir, SDPath) != FR_OK) {
        printf("Cannot open SD directory\r\n");
        return;
    }
    
    // Enumerate files
    while(f_readdir(&dir, &fno) == FR_OK && fno.fname[0]) {
        if(fno.fattrib & AM_DIR) continue; // Skip directories
        
        char src_path[256];
        char dst_path[256];
        
        sprintf(src_path, "%s%s", SDPath, fno.fname);
        sprintf(dst_path, "%s%s", USBPath, fno.fname);
        
        // Open source file (SD)
        if(f_open(&fsrc, src_path, FA_READ) != FR_OK) {
            printf("Cannot open source file: %s\r\n", src_path);
            continue;
        }
        
        // Create destination file (USB)
        if(f_open(&fdst, dst_path, FA_CREATE_ALWAYS | FA_WRITE) != FR_OK) {
            printf("Cannot create destination file: %s\r\n", dst_path);
            f_close(&fsrc);
            continue;
        }
        
        // Copy file content
        printf("Copying %s -> %s\r\n", src_path, dst_path);
        do {
            f_read(&fsrc, buffer, sizeof(buffer), &br);
            if(br > 0) {
                f_write(&fdst, buffer, br, &bw);
                if(bw != br) {
                    printf("Write error\r\n");
                    break;
                }
            }
        } while(br > 0);
        
        f_close(&fsrc);
        f_close(&fdst);
    }
    
    f_closedir(&dir);
    printf("Copy complete\r\n");
}

/**
  * @brief  Main program
  */
int main(void)
{
    // HAL initialization
    HAL_Init();
    SystemClock_Config();
    MPU_Config();
    MX_GPIO_Init();
    
    // Initialize SD card
    if(MX_SDMMC1_SD_Init() != HAL_OK) {
        Error_Handler();
    }
    if(f_mount(&fsSd, SDPath, 1) != FR_OK) {
        printf("Failed to mount SD card\r\n");
    }
    
    // Initialize USB Host
    MX_USB_HOST_Init();
    
    printf("Waiting for USB flash drive...\r\n");
    
    // Main loop
    while (1)
    {
        MX_USB_HOST_Process();
        
        // When USB device is connected and ready
        if(Appli_state == APPLICATION_READY) {
            // Mount USB drive
            if(f_mount(&fsUsb, USBPath, 1) == FR_OK) {
                printf("USB flash drive mounted\r\n");
                CopySDtoUSB();
                f_mount(NULL, USBPath, 0); // Unmount USB
                printf("Done. You can remove USB drive now.\r\n");
                Appli_state = APPLICATION_DISCONNECT;
            }
        }
        
        HAL_Delay(100);
    }
}
