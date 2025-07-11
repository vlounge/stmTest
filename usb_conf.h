#pragma once
#define USBH_MAX_NUM_ENDPOINTS      2
#define USBH_MAX_NUM_INTERFACES     2
#define USBH_DEBUG_LEVEL            0
#define USBH_USE_OS                 0

// Memory management
#define USBH_malloc    malloc
#define USBH_free      free
#define USBH_memset    memset
#define USBH_memcpy    memcpy

// Callbacks
#define USBH_UsrLog(...)    printf(__VA_ARGS__)
#define USBH_UsrError(...)  printf("ERROR: " __VA_ARGS__)

#include "stm32h7xx_hal.h"
