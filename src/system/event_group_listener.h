#include "freertos/event_groups.h"

#ifndef __EVENT_GROUP_LISTENER__
#define __EVENT_GROUP_LISTENER__

typedef struct {
    EventGroupHandle_t xEventGroup;
    EventBits_t uxBitsToSet;
} EventGroupHandle_and_EventBits;

#endif
