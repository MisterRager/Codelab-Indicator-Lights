#include <string.h>
#include <esp_log.h>
#include <esp_err.h>
#include <esp_http_client.h>

#include "data_http.h"

void fetch_forecasts(
    int forecast_office_len,
    char *forecast_office,
    int grid_x,
    int grid_y,
    void (*response_callback)(cJSON *));

void read_forecasts(
    cJSON *json_root,
    void (*forecast_callback_fn)(int, cJSON *));