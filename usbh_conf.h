#pragma once

#include "stm32h7xx_hal.h"
#include "usbh_def.h"

// Memory management
#define USBH_malloc               malloc
#define USBH_free                 free
#define USBH_memset               memset
#define USBH_memcpy               memcpy

// Debug settings
#define USBH_DEBUG_LEVEL          0
#define USBH_USE_OS               0

// Class callbacks
#define USBH_UsrLog(...)          printf(__VA_ARGS__)
#define USBH_UsrWarn(...)         printf("WARN: " __VA_ARGS__)
#define USBH_UsrError(...)        printf("ERROR: " __VA_ARGS__)

// Class table (добавьте это)
extern USBH_ClassTypeDef  USBH_MSC_Class;
#define USBH_MSC_CLASS    &USBH_MSC_Class
