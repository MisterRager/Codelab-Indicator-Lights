#include "national_weather.h"

#define TAG "NWS Data"
#define URI_FORECAST_TEMPLATE "https://api.weather.gov/gridpoints/%s/%d,%d/forecast"

void fetch_forecasts(
    int forecast_office_len,
    char *forecast_office,
    int grid_x,
    int grid_y,
    void (*response_callback)(cJSON *))
{
    char *url = (char *)malloc(
        (sizeof(URI_FORECAST_TEMPLATE) / sizeof(char)) -
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

void read_forecasts(
    cJSON *json_root,
    void (*forecast_callback_fn)(int, cJSON *))
{
    int forecast_index = 0;
    cJSON *properties, *periods, *forecast;

    if (!json_root)
    {
        ESP_LOGI(TAG, "no json to process, exiting forecast reader early");
        return;
    }

    properties = cJSON_GetObjectItem(json_root, "properties");

    if (!properties)
    {
        ESP_LOGI(TAG, "root node is missing node \"properties\", exit forecast reader early");
        free(json_root);
        return;
    }

    periods = cJSON_GetObjectItem(properties, "periods");

    if (!periods)
    {
        ESP_LOGI(TAG, "node \"properties\" is missing node \"periods\", exit forecast reader early");
        free(json_root);
        return;
    }

    cJSON_ArrayForEach(forecast, periods) {
        forecast_callback_fn(forecast_index, forecast);
        forecast_index += 1;
    }

    free(json_root);
}