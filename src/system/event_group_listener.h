#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#ifndef __EVENT_GROUP_LISTENER__
#define __EVENT_GROUP_LISTENER__

#define BIT_NTH(n) (1 << n)

typedef struct {
    EventGroupHandle_t xEventGroup;
    EventBits_t uxBitsToSet;
} EventGroupHandle_and_EventBits;

#endif
