#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_log.h"

#include "system/event_group_listener.h"
#include "system/wifi.h"
#include "system/nvs.h"

const char *TAG = "Main";

#define BIT_HAS_IP BIT_NTH(0)

void app_main()
{
    EventGroupHandle_and_EventBits ip_status;
    int wait_status;

    ESP_LOGI(TAG, "Main!!");
    initialize_nvs_flash();
    connect_wifi();

    ESP_LOGI(TAG, "Finished setting up, wait for network...");

    ip_status.xEventGroup = xEventGroupCreate();
    ip_status.uxBitsToSet = BIT_HAS_IP;

    register_bits_on_ip_gotten_event(ip_status);

    do
    {
        ESP_LOGI(TAG, "Waiting for network to get ip address");
        wait_status = xEventGroupWaitBits(
            ip_status.xEventGroup,
            ip_status.uxBitsToSet,
            0 /* clear on exit */,
            1 /* wait for all bits */,
            200 /* Number of ticks to wait */
        );
    } while(!(wait_status | BIT_HAS_IP));

    ESP_LOGI(TAG, "Connected!");
}