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

#define BIT_HAS_IP BIT_NTH(1)
#define MAX_TICKS_WAIT 10000

struct led_state ring;

static void on_connect_task(void *args)
{
    ESP_LOGI(TAG, "on_connect_task start");
    int wait_status;
    EventGroupHandle_and_EventBits *ip_status = args;

    register_bits_on_ip_gotten_event(ip_status);

    wait_status = xEventGroupGetBits(ip_status->xEventGroup);
    ESP_LOGI(TAG, "IP Status is [%d]", wait_status);

    uint wait_ticks = 50;

    while (!(wait_status & BIT_HAS_IP))
    {
        ESP_LOGI(
            TAG,
            "Waiting to get ip address on group [%d] for bits [%d] for [%d] ticks",
            (uint) ip_status->xEventGroup,
            ip_status->uxBitsToSet,
            wait_ticks);

        wait_status = xEventGroupWaitBits(
            ip_status->xEventGroup,
            ip_status->uxBitsToSet,
            0 /* clear on exit */,
            1 /* wait for all bits */,
            wait_ticks /* Number of ticks to wait */
        );

        wait_ticks += wait_ticks;

        if (wait_ticks > MAX_TICKS_WAIT) {
            wait_ticks = MAX_TICKS_WAIT;
        }

        ESP_LOGI(TAG, "Status is [%d]", wait_status);
    }

    ESP_LOGI(TAG, "Connected!");

    ring_spinning_rainbow(&ring);

    vTaskDelete(NULL);
}

static EventGroupHandle_and_EventBits messages_ip_status;

void app_main()
{
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

    // Set up listener for IP Address received
    messages_ip_status = (EventGroupHandle_and_EventBits){
        .xEventGroup = xEventGroupCreate(),
        .uxBitsToSet = BIT_HAS_IP,
    };

    ESP_LOGI(
        TAG,
        "Listen for IP with handle [%d] on bits [%d]",
        (uint) messages_ip_status.xEventGroup, messages_ip_status.uxBitsToSet);

    xEventGroupClearBits(messages_ip_status.xEventGroup, messages_ip_status.uxBitsToSet);
    register_bits_on_ip_gotten_event(&messages_ip_status);

    xTaskCreate(on_connect_task, "On-Connect", 4096, &messages_ip_status, 5, NULL);

    connect_wifi();
    ESP_LOGI(TAG, "Finished setting up, wait for network...");
}