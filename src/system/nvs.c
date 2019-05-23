#include "nvs.h"

static int initialized = 0;
/**
 * It seems like a bit of functionality in the esp-idf depends on the
 * non-volatile-storage partition being available for use. Of primary
 * interest is WiFi. This is lifted from the "getting started" example
 * in the esp-idf documentation.
 */
void initialize_nvs_flash() {
    if (initialized)
    {
        return;
    }

    initialized = 1;

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
}