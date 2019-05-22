#include "color_ring.h"

union Pixel pixel_rgb(uint8_t red, uint8_t green, uint8_t blue)
{
    return (union Pixel){
        .components = (PixelBGRW){
            .r = red,
            .g = green,
            .b = blue,
        }
    };
}

inline union Pixel rgb_spectrum(int rainbow_length, int index) {
    int scaled = index * 764 / rainbow_length;
    return (union Pixel){
        .components = (PixelBGRW){
            .r = scaled < 255 ? (255 - scaled)
                     : (scaled >= 510 ? scaled - 509 : 0),
            .g = scaled <= 255 ? scaled
                               : (scaled >= 510 ? 0 : 510 - scaled),
            .b = scaled <= 255 ? 0
                               : (scaled <= 510 ? scaled - 255 : 764 - scaled),
        }};
}

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