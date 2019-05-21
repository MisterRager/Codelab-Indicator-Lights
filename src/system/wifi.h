#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "esp_wifi_types.h"

#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "event_group_listener.h"


#define BIT_NTH(n) (1 << n)

#define BIT_WIFI_READY BIT_NTH(0)
#define BIT_WIFI_INITIALIZED BIT_NTH(1)

void connect_wifi();

esp_err_t register_bits_on_ip_gotten_event( EventGroupHandle_and_EventBits listener);