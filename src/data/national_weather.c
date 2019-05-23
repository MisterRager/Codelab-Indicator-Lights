#include "national_weather.h"

#define TAG "NWS Data"
#define URI_FORECAST_TEMPLATE "https://api.weather.gov/gridpoints/%s/%d,%d/forecast"

void fetch_forecasts(
    int forecast_office_len,
    char *forecast_office,
    int grid_x,
    int grid_y,
    http_response_callback response_callback
)
{

    char *url = (char *)malloc(
        (
            sizeof(URI_FORECAST_TEMPLATE) / sizeof(char)) -
        5 + forecast_office_len + (grid_x / 10 + 1) + (grid_y / 10 + 1));

    sprintf(url, URI_FORECAST_TEMPLATE, forecast_office, grid_x, grid_y);
    ESP_ERROR_CHECK(
        data_get(
            &(http_request){
                .url = url,
                .on_success_fn = response_callback,
                .configure_client_fn = NULL,
            }));
}