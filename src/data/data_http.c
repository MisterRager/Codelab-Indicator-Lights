#include "data_http.h"

#define TAG "Data: HTTP"

#define BUFFER_LEN 32768
static char response_buffer[BUFFER_LEN];
static char *cursor = response_buffer;

static void (*response_callback_fn)(cJSON *);

static inline void reset_state()
{
    cursor = response_buffer;
    response_callback_fn = NULL;
}

static esp_err_t request_callback(esp_http_client_event_t *event)
{
    ESP_LOGI(TAG, "Got event! [%d]", event->event_id);

    switch (event->event_id)
    {
    case HTTP_EVENT_ON_DATA:
        ESP_LOGI(TAG, "Got some data! %d bytes", event->data_len);
        if (cursor == response_buffer)
        {
            memset(response_buffer, 0, BUFFER_LEN);
        }

        if (cursor - response_buffer > BUFFER_LEN)
        {
            ESP_LOGI(TAG, "Buffer full!");
            return ESP_ERR_NO_MEM;
        }

        memcpy(cursor, event->data, event->data_len);
        cursor += event->data_len;

        return ESP_OK;
    case HTTP_EVENT_ON_FINISH:
        ESP_LOGI(TAG, "Done! Got %d characters of data!", cursor - response_buffer);

        if (response_callback_fn)
        {
            //response_callback_fn(cursor - response_buffer, response_buffer);
            response_callback_fn(cJSON_Parse(response_buffer));
        }

        reset_state();
        return ESP_OK;
    case HTTP_EVENT_ERROR:
        ESP_LOGI(TAG, "Error doing HTTP Request!");
        reset_state();
        return ESP_OK;
    default:
        break;
    }
    return ESP_OK;
}

esp_err_t data_get(http_request *request)
{
    if (response_callback_fn)
    {
        return ESP_ERR_INVALID_STATE;
    }

    response_callback_fn = request->on_success_fn;
    // Build a client for the request
    esp_http_client_config_t config = (esp_http_client_config_t){
        .url = request->url,
        .event_handler = request_callback,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);

    // Let the caller configure the client a bit
    if (request->configure_client_fn)
    {
        request->configure_client_fn(client);
    }

    // Perform request
    return esp_http_client_perform(client);
}