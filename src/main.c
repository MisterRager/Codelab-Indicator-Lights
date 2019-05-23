#include <esp_log.h>

#include "system/wifi.h"
#include "data/national_weather.h"
#include "ui/color_ring.h"

#define TAG "Main"
#define BRIGHTNESS 0.02

static struct led_state ring;

static inline union Pixel temperature_color(int temperature)
{
    return rgb_spectrum(
        60,
        temperature > 85 ? 0 : 85 - temperature
    );
}

static void consume_forecast(int index, cJSON *forecast)
{
    ESP_LOGI(TAG, "consume forecast #%d", index);
    if (index == 0)
    {
        stop_animate_ring_rotate();
        ring_fill(pixel_rgb(0, 0, 0), &ring);
        ws2812_write_leds(ring);
    }

    ring.leds[index] = temperature_color(
        cJSON_GetObjectItemCaseSensitive(
            forecast,
            "temperature")
            ->valueint);

    ring.leds[index].components.r *= BRIGHTNESS;
    ring.leds[index].components.g *= BRIGHTNESS;
    ring.leds[index].components.b *= BRIGHTNESS;

    ws2812_write_leds(ring);
}

static void on_recieve_forecast(cJSON *json)
{
    ESP_LOGI(TAG, "Fetched!");
    read_forecasts(json, consume_forecast);
}

static void task_fetch_forecasts(void *unused)
{
    fetch_forecasts(3, "TOP", 31, 80, on_recieve_forecast);
    vTaskDelete(NULL);
}

static void on_ready()
{
    ESP_LOGI(TAG, "Ready!");
    xTaskCreate(task_fetch_forecasts, "Fetch Forecasts", 4096, NULL, 5, NULL);

    ring_fill_rainbow(&ring);
    ring_dim(BRIGHTNESS, &ring);
    animate_ring_rotate(&ring);
}

void app_main()
{
    ESP_LOGI(TAG, "Hello!!");

    ws2812_control_init();
    ring_fill(pixel_rgb(255, 80, 0), &ring);
    ring_dim(BRIGHTNESS, &ring);
    ws2812_write_leds(ring);

    register_network_ready_listener(on_ready);
    connect_wifi();
}