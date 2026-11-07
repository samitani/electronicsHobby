#ifndef LED_CONTROLLER_H
#define LED_CONTROLLER_H

#include <stdint.h>
#include <stdbool.h>
#include "timing_game.h"

#ifdef __cplusplus
extern "C" {
#endif

void led_controller_init(uint8_t pin, uint16_t num_leds, bool din_is_top);
void led_controller_set_brightness(uint8_t brightness);
void led_controller_apply_buffer(const rgb_color_t* buffer, int count);
void led_controller_clear(void);

#ifdef __cplusplus
}
#endif

#endif // LED_CONTROLLER_H
