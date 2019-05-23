#include <string.h>
#include <esp_err.h>
#include <esp_log.h>
#include <esp_http_client.h>

typedef void (*http_response_callback)(int, char *);

typedef struct
{
    char *url;
    http_response_callback on_success_fn;
    void (*configure_client_fn)(esp_http_client_handle_t);
} http_request;

esp_err_t data_get(http_request *request);