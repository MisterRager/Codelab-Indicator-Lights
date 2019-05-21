#include "wifi.h"

#define WIFI_SSID CONFIG_WIFI_SSID
#define WIFI_PASS CONFIG_WIFI_PASSWORD

static const char *TAG = "ConnectWiFi";

static wifi_config_t wifi_config = {
    .sta = {
        .ssid = WIFI_SSID,
        .password = WIFI_PASS,
    }};

static EventGroupHandle_t wifi_event_group;

#define MAX_IP_LISTENERS 1

static EventGroupHandle_and_EventBits ip_listeners[MAX_IP_LISTENERS];
static int listener_count = 0;

static esp_err_t sys_event_handler(void *ctx, system_event_t *event)
{
    switch (event->event_id)
    {
    case SYSTEM_EVENT_WIFI_READY:
        ESP_LOGI(TAG, "Event: WiFi Ready");
        xEventGroupSetBits(wifi_event_group, BIT_WIFI_READY);
        break;
    case SYSTEM_EVENT_STA_START:
        ESP_LOGI(TAG, "Event: STA Start");
        return esp_wifi_connect();
    case SYSTEM_EVENT_STA_GOT_IP:
        ESP_LOGI(TAG, "Event: Got IP");

        for (int k = 0; k < listener_count; k++) {
            ESP_LOGI(TAG, "Toggle Bits for IP for listener [%d]", k);
            xEventGroupSetBits(ip_listeners[k].xEventGroup, ip_listeners[k].uxBitsToSet);
        }
        break;
    case SYSTEM_EVENT_STA_LOST_IP:
        ESP_LOGI(TAG, "Event: Lost IP");
        xEventGroupClearBits(wifi_event_group, BIT_WIFI_READY);
        break;
    case SYSTEM_EVENT_STA_STOP:
        ESP_LOGI(TAG, "Event: STA Stop");
        // TODO: add some sort of listener for this
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        ESP_LOGI(TAG, "Event: STA Disconnected");
        return esp_wifi_connect();
    default:
        break;
    }
    return ESP_OK;
}

#define MAX_WAIT_TICKS 100

static TaskHandle_t task_handle_connect_wifi = NULL;

static void connect_wifi_task(void *args)
{
    ESP_LOGI(TAG, "Start task: Connect to WiFi");
    EventBits_t status = 0;
    int wait_ticks = 10;
    int wait_ticks_next;
    ESP_ERROR_CHECK(esp_event_loop_init(sys_event_handler, wifi_event_group));

    status = xEventGroupGetBits(wifi_event_group);

    do
    {
        ESP_LOGI(TAG, "Wait for WiFi to be ready");

        status = xEventGroupWaitBits(
            wifi_event_group,
            BIT_WIFI_READY,
            0 /* Clear on Exit */,
            1 /* Wait for All */,
            wait_ticks);

        wait_ticks_next = wait_ticks * 1.5;
        wait_ticks = wait_ticks_next > MAX_WAIT_TICKS ? MAX_WAIT_TICKS : wait_ticks_next;
    } while (!(status | BIT_WIFI_READY));

    ESP_LOGI(TAG, "Done waiting, start init!");

    tcpip_adapter_init();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));

    ESP_LOGI(
        TAG,
        "Setting WiFi configuration SSID %s...",
        wifi_config.sta.ssid);

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));

    ESP_LOGI(TAG, "WiFi is configred. Start!");

    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "WiFi is set!");
    xEventGroupSetBits(wifi_event_group, BIT_WIFI_INITIALIZED);

    // Tasks have to loop - if this one isn't necessary, just suspend
    ESP_LOGI(TAG, "Exit WiFi Connect Task");

    vTaskDelete(task_handle_connect_wifi);
    task_handle_connect_wifi = NULL;
}

void connect_wifi()
{
    if (task_handle_connect_wifi)
    {
        return;
    }

    if (!wifi_event_group)
    {
        wifi_event_group = xEventGroupCreate();
    }

    xTaskCreate(
        connect_wifi_task,
        "connect_wifi",
        4096,
        NULL,
        5,
        task_handle_connect_wifi);
}

esp_err_t register_bits_on_ip_gotten_event( EventGroupHandle_and_EventBits listener)
{
    if (listener_count < MAX_IP_LISTENERS)
    {
        ESP_LOGI(TAG, "Registering to write bits [%d] for the given event group", listener.uxBitsToSet);

        ip_listeners[listener_count].xEventGroup = listener.xEventGroup;
        ip_listeners[listener_count].uxBitsToSet = listener.uxBitsToSet;
        listener_count += 1;

        return ESP_OK;
    } else {
        ESP_LOGE(TAG, "Cannot add another listener!");
        return ESP_ERR_NO_MEM;
    }
}

void clear_network_info_listeners()
{
    listener_count = 0;
}