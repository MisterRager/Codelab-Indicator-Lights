#include "color_ring.h"

#define TAG "Color Ring"

#define BIT_STOP_SPIN BIT_NTH(1)
#define BIT_IS_SPINNING BIT_NTH(2)

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

EventGroupHandle_t group_handle_spin_ring = NULL;

void ring_fill_rainbow(struct led_state *ring_ptr)
{
    for (int k = 0; k < NUM_LEDS; k++)
    {
        ring_ptr->leds[k] = rgb_spectrum(NUM_LEDS, k);
    }
}

static void task_spin_ring(void *args)
{
    struct led_state *ring_ptr = (struct led_state *)args;

    ESP_LOGI(TAG, "Begin Spinning Task");

    while (!(xEventGroupGetBits(group_handle_spin_ring) & BIT_STOP_SPIN))
    {
        ws2812_write_leds(*ring_ptr);
        vTaskDelay(6);
        ring_rotate(NUM_LEDS, ring_ptr);
    }

    ESP_LOGI(TAG, "Stopped spinning");
    xEventGroupClearBits(group_handle_spin_ring, BIT_STOP_SPIN | BIT_IS_SPINNING);
    vTaskDelete(NULL);
}

void animate_ring_rotate(struct led_state *ring_ptr)
{
    ESP_LOGI(TAG, "Request spinning");

    if (!group_handle_spin_ring)
    {
        group_handle_spin_ring = xEventGroupCreate();
    }

    // Do not start spinning if it already is?
    if (xEventGroupGetBits(group_handle_spin_ring) & BIT_IS_SPINNING)
    {
        ESP_LOGI(TAG, "Already spinning!");
        return;
    }

    xEventGroupClearBits(group_handle_spin_ring, BIT_STOP_SPIN);
    xEventGroupSetBits(group_handle_spin_ring, BIT_IS_SPINNING);

    xTaskCreate(
        task_spin_ring,
        "LED Spinner",
        4096,
        ring_ptr,
        5,
        NULL);
}

void stop_animate_ring_rotate()
{
    ESP_LOGI(TAG, "Try to stop the spin");
    if (xEventGroupGetBits(group_handle_spin_ring) & BIT_IS_SPINNING)
    {
        ESP_LOGI(TAG, "Requesting spin stop...");
        xEventGroupSetBits(group_handle_spin_ring, BIT_STOP_SPIN);
    }
}