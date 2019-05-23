#include <string.h>
#include <esp_err.h>
#include <esp_log.h>
#include <esp_http_client.h>
#include <cJSON.h>

typedef struct
{
    char *url;
    void (*on_success_fn)(cJSON *);
    void (*configure_client_fn)(esp_http_client_handle_t);
} http_request;

esp_err_t data_get(http_request *request);