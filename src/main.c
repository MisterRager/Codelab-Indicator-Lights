#include <string.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>

#include <esp_log.h>

#include "system/event_group_listener.h"
#include "system/wifi.h"
#include "system/nvs.h"
#include "ui/color_ring.h"
#include "data/national_weather.h"

#include <ws2812_control.h>
#include <cJSON.h>

#define TAG "Main"

#define BIT_HAS_IP BIT_NTH(1)
#define MAX_TICKS_WAIT 10000

struct led_state ring;

typedef struct
{
    int length;
    char *string;
} length_string_t;

typedef struct
{
    char * name;
    int temp;
    char * temp_unit;
} forecast_t;

static inline forecast_t extract_forecast_data(cJSON *forecast)
{
    cJSON *name, *temp, *temp_unit;

    name = cJSON_GetObjectItem(forecast, "name");
    temp = cJSON_GetObjectItem(forecast, "temperature");
    temp_unit = cJSON_GetObjectItem(forecast, "temperatureUnit");

    return (forecast_t){
        .name = name->valuestring,
        .temp = temp->valueint,
        .temp_unit = temp_unit->valuestring,
    };
}

static void consume_forecast_task(void *args)
{
    stop_animate_ring_rotate();

    cJSON *root, *json, *forecast;
    length_string_t *forecast_json = args;

    ESP_LOGI(TAG, "Process %d char of json", forecast_json->length);

    root = json = cJSON_Parse(forecast_json->string);

    if (!json)
    {
        ESP_LOGE(TAG, "Failed to parse anything?");
        return;
    }

    json = cJSON_GetObjectItemCaseSensitive(json, "properties");

    if (!json)
    {
        ESP_LOGE(TAG, "Failed to find node \"properties\" in root");
        return;
    }

    json = cJSON_GetObjectItemCaseSensitive(json, "periods");
    if (!json)
    {
        ESP_LOGE(TAG, "Failed to find node \"periods\" in node \"properties\"");
        return;
    }

    if (json)
    {
        ESP_LOGI(TAG, "Found the forecasts, iterating");
        ring_fill(pixel_rgb(0, 0, 0), NUM_LEDS, &ring);

        forecast_t forecast_data;
        int scaled_temp;
        int index = 0;

        cJSON_ArrayForEach(forecast, json)
        {
            forecast_data = extract_forecast_data(forecast);

            scaled_temp = 85 - forecast_data.temp;
            ring.leds[index] = rgb_spectrum(60, scaled_temp > 0 ? scaled_temp : 0);

            index++;
        }

        ring_dim(0.02, NUM_LEDS, &ring);
        ws2812_write_leds(ring);

        animate_ring_rotate(&ring);
    }

    free(args);
    free(root);

    vTaskDelete(NULL);
}

static void on_receive_forecast(int forecast_len, char *forecast)
{
    ESP_LOGI(TAG, "Recieved %d characters!", forecast_len);

    length_string_t *boat = (length_string_t *)malloc(sizeof(length_string_t));

    boat->length = forecast_len;
    boat->string = forecast;

    ESP_LOGI(TAG, "launch forecast consume task");
    xTaskCreate(
        consume_forecast_task,
        "consume forecast",
        4096,
        boat,
        5,
        NULL);
}

static void fetch_forecasts_task(void *args)
{
    ring_fill_rainbow(&ring);
    ring_dim(0.02f, NUM_LEDS, &ring);
    animate_ring_rotate(&ring);

    fetch_forecasts(3, "TOP", 31, 80, on_receive_forecast);

    vTaskDelete(NULL);
}

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
            (uint)ip_status->xEventGroup,
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

        if (wait_ticks > MAX_TICKS_WAIT)
        {
            wait_ticks = MAX_TICKS_WAIT;
        }

        ESP_LOGI(TAG, "Status is [%d]", wait_status);
    }

    ESP_LOGI(TAG, "Connected!");
    xTaskCreate(fetch_forecasts_task, "fetch forecasts", 4096, NULL, 5, NULL);
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
        (uint)messages_ip_status.xEventGroup, messages_ip_status.uxBitsToSet);

    xEventGroupClearBits(messages_ip_status.xEventGroup, messages_ip_status.uxBitsToSet);
    register_bits_on_ip_gotten_event(&messages_ip_status);

    xTaskCreate(on_connect_task, "On-Connect", 4096, &messages_ip_status, 5, NULL);

    connect_wifi();
    ESP_LOGI(TAG, "Finished setting up, wait for network...");
}