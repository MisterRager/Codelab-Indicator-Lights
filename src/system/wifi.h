#include <esp_log.h>
#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_event_loop.h>
#include <esp_wifi_types.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>

#include "nvs.h"
#include "event_group_listener.h"

void connect_wifi();

esp_err_t register_network_ready_listener(void (*listener)());