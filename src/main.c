#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>

#include <esp_log.h>

#include "system/event_group_listener.h"
#include "system/wifi.h"
#include "system/nvs.h"
#include "ui/color_ring.h"

#include <ws2812_control.h>

const char *TAG = "Main";

#define BIT_HAS_IP BIT_NTH(0)

struct led_state ring;

static void on_connect_task(void *args)
{
    ESP_LOGI(TAG, "on_connect_task start");
    int wait_status;
    EventGroupHandle_and_EventBits *ip_status = args;

    register_bits_on_ip_gotten_event(ip_status);

    do
    {
        ESP_LOGI(TAG, "Waiting for network to get ip address");

        wait_status = xEventGroupWaitBits(
            ip_status->xEventGroup,
            ip_status->uxBitsToSet,
            0 /* clear on exit */,
            1 /* wait for all bits */,
            10 /* Number of ticks to wait */
        );
    } while (!(wait_status | BIT_HAS_IP));

    ESP_LOGI(TAG, "Connected!");

    ring_spinning_rainbow(&ring);

    vTaskDelete(NULL);
}

void app_main()
{
    EventGroupHandle_and_EventBits ip_status;

    ESP_LOGI(TAG, "Hello!!");
    ESP_LOGI(TAG, "Initialize Lights");
    ws2812_control_init();

    // Orange status until connected
    ring_fill(pixel_rgb(255, 200, 0), NUM_LEDS, &ring);
    ring_dim(0.02, NUM_LEDS, &ring);
    ws2812_write_leds(ring);

    ESP_LOGI(TAG, "Initialize NVS");
    initialize_nvs_flash();

    ESP_LOGI(TAG, "Initialize WiFi");

    ip_status.xEventGroup = xEventGroupCreate();
    ip_status.uxBitsToSet = BIT_HAS_IP;
    register_bits_on_ip_gotten_event(&ip_status);
    xTaskCreate(on_connect_task, "On-Connect", 4096, &ip_status, 5, NULL);

    connect_wifi();
    ESP_LOGI(TAG, "Finished setting up, wait for network...");
}