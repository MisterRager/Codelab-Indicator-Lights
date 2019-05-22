#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_log.h"

#include "system/event_group_listener.h"
#include "system/wifi.h"
#include "system/nvs.h"
#include "ui/color_ring.h"

#include "../lib/ESP32-NeoPixel-WS2812-RMT/ws2812_control.h"

const char *TAG = "Main";

#define BIT_HAS_IP BIT_NTH(0)

struct led_state ring;

inline union Pixel rgb_spectrum(int max, int n) {
    int scaled = n * 764 / max;
    return (union Pixel){
        .components = (PixelBGRW){
            .r = scaled < 255 ? (255 - scaled)
                     : (scaled >= 510 ? scaled - 509 : 0),
            .g = scaled <= 255 ? scaled
                               : (scaled >= 510 ? 0 : 510 - scaled),
            .b = scaled <= 255 ? 0
                               : (scaled <= 510 ? scaled - 255 : 764 - scaled),
        }};
}

static void rainbow() {
    for (int k = 0; k < NUM_LEDS; k++)
    {
        ring.leds[k] = rgb_spectrum(NUM_LEDS, k);
        ring.leds[k].components.r /= 32;
        ring.leds[k].components.g /= 32;
        ring.leds[k].components.b /= 32;
    }
    ws2812_write_leds(ring);
}

TaskHandle_t task_handle_spinning_rainbow = NULL;
void task_spin_rainbow(void *args)
{
    rainbow();

    while(1)
    {
        vTaskDelay(6);
        ESP_LOGI(TAG, "SPIN");
        ring_rotate(NUM_LEDS, &ring);
        ws2812_write_leds(ring);
    }
}

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
    } while(!(wait_status | BIT_HAS_IP));

    ESP_LOGI(TAG, "Connected!");

    if (task_handle_spinning_rainbow)
    {
        vTaskDelete(task_handle_spinning_rainbow);
    }

    xTaskCreate(
        task_spin_rainbow,
        "Rainbow Spinner",
        4096,
        NULL,
        5,
        task_handle_spinning_rainbow);

    vTaskDelete(NULL);
}

void app_main()
{
    EventGroupHandle_and_EventBits ip_status;

    ESP_LOGI(TAG, "Hello!!");
    ESP_LOGI(TAG, "Initialize Lights");
    ws2812_control_init();

    // Orange status until connected
    ring_fill(pixel_rgb(30, 10, 0), NUM_LEDS, &ring);
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