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
    http_response_callback response_callback);