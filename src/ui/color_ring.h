#include "../lib/ESP32-NeoPixel-WS2812-RMT/ws2812_control.h"

/**
 * Generate a color pixel of a "rainbow" gradient.
 * 
 * Given colors as tuples (R, G, B), there is a linear gradient from one color to the next:
 * 
 *   (255, 0, 0)   (0, 255, 0) .  (0, 0, 255)   (254, 0, 1)
 *      [Red----------Green----------Blue----------]
 * 
 * This gives 764 discrete colors. A pixel's color is calculated by "scaling" its index to this 764
 * number numberline.
 */
inline union Pixel rgb_spectrum(int rainbow_length, int index);

/**
 * Given Red, Green, and Blue values for a pixel, give us a Pixel
 */
union Pixel pixel_rgb(uint8_t red, uint8_t green, uint8_t blue);

/**
 * Scale the brightness of the whole ring by a float.
 */
void ring_dim(float level, int length, struct led_state *ring);

/**
 * Set each LED in the ring to the same color.
 */
void ring_fill(union Pixel color, int length, struct led_state *ring);

/**
 * Move every LED's pixel information down the line by 1, wrapping around.
 */
void ring_rotate(int length, struct led_state *ring);