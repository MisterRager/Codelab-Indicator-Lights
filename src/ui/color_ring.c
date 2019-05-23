#include "color_ring.h"

static const char TAG[] = "Color Ring";

void ring_dim(float level, int length, struct led_state *ring)
{
    for (int k = 0; k < length; k++)
    {
        ring->leds[k].components.r *= level;
        ring->leds[k].components.g *= level;
        ring->leds[k].components.b *= level;
    }
}

void ring_fill(union Pixel color, int length, struct led_state *ring)
{
    for (int k = 0; k < length; k++)
    {
        ring->leds[k] = color;
    }
}

void ring_rotate(int length, struct led_state *ring)
{
    union Pixel first = ring->leds[0];
    for (int k = 0; k < length; k++)
    {
        if ((k + 1) < length)
        {
            ring->leds[k] = ring->leds[k + 1];
        }
        else
        {
            ring->leds[k] = first;
        }
    }
}

TaskHandle_t task_handle_spinning_rainbow = NULL;
static void task_spin_rainbow(void *args)
{
    struct led_state *ring_ptr = (struct led_state *)args;

    for (int k = 0; k < NUM_LEDS; k++)
    {
        ring_ptr->leds[k] = rgb_spectrum(NUM_LEDS, k);
    }

    ring_dim(0.1f, NUM_LEDS, ring_ptr);

    while (1)
    {
        ws2812_write_leds(*ring_ptr);
        vTaskDelay(6);
        ESP_LOGI(TAG, "SPIN");
        ring_rotate(NUM_LEDS, ring_ptr);
    }
}

void ring_spinning_rainbow(struct led_state *ring_ptr)
{
    if (task_handle_spinning_rainbow)
    {
        vTaskDelete(task_handle_spinning_rainbow);
    }

    xTaskCreate(
        task_spin_rainbow,
        "Rainbow Spinner",
        4096,
        ring_ptr,
        5,
        task_handle_spinning_rainbow);
}
