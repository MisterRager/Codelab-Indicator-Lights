#ifndef WS2812_CONTROL_H
#define WS2812_CONTROL_H
#include <stdint.h>

#ifndef NUM_LEDS
#define NUM_LEDS 22
#endif

// This structure is used for indicating what the colors of each LED should be set to.
// There is a 32bit value for each LED. Only the lower 3 bytes are used and they hold the
// Red (byte 2), Green (byte 1), and Blue (byte 0) values to be set.
typedef struct {
    uint8_t b;
    uint8_t g;
    uint8_t r;
    uint8_t w;
} PixelBGRW;

union Pixel {
    PixelBGRW components;
    uint32_t raw;
};

struct led_state {
    union Pixel leds[NUM_LEDS];
};

// Setup the hardware peripheral. Only call this once.
void ws2812_control_init(void);

// Update the LEDs to the new state. Call as needed.
// This function will block the current task until the RMT peripheral is finished sending 
// the entire sequence.
void ws2812_write_leds(struct led_state new_state);

#endif