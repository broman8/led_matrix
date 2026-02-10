#pragma once

#include "driver/gpio.h"
#include "led_strip.h"
#include <stdint.h>

// Hardware Configuration
#define MATRIX_WIDTH 64
#define MATRIX_HEIGHT 8
#define LED_STRIP_LEN (MATRIX_WIDTH * MATRIX_HEIGHT)

// Function Prototypes
void led_matrix_init(int gpio_pin);
void led_matrix_clear();
void led_matrix_set_pixel(int x, int y, uint8_t r, uint8_t g, uint8_t b);
void led_matrix_refresh();
void led_matrix_draw_text(const char *text, int x_offset, uint8_t r, uint8_t g,
                          uint8_t b, bool scroll = false, int delay_ms = 50);
