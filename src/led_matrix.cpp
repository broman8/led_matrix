#include "led_matrix.h"
#include "esp_log.h"
#include "font8x8_basic.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdlib.h>
#include <string.h>


static const char *TAG = "led_matrix";
static led_strip_handle_t led_strip;

// 32x8 Matrix Mapping (Column Major ZigZag)
// x: 0..31 (Left to Right), y: 0..7 (Top to Bottom)
static uint32_t get_pixel_index(int x, int y) {
  // Safety check
  if (x < 0 || x >= MATRIX_WIDTH || y < 0 || y >= MATRIX_HEIGHT) {
    return LED_STRIP_LEN; // Return out of bounds index
  }

  // Rotate 180 degrees
  x = MATRIX_WIDTH - 1 - x;
  y = MATRIX_HEIGHT - 1 - y;

  if (x % 2 == 0) {
    // Even Column: Downwards (0 -> 7)
    return (x * 8) + y;
  } else {
    // Odd Column: Upwards (7 -> 0)
    return (x * 8) + (7 - y);
  }
}

void led_matrix_init(int gpio_pin) {
  ESP_LOGI(TAG, "Initializing LED Matrix on GPIO %d", gpio_pin);

  led_strip_config_t strip_config = {
      .strip_gpio_num = gpio_pin,
      .max_leds = LED_STRIP_LEN,
      .led_model = LED_MODEL_WS2812,
      .color_component_format = LED_STRIP_COLOR_COMPONENT_FMT_GRB,
      .flags = {.invert_out = false},
  };

  led_strip_rmt_config_t rmt_config = {
      .clk_src = RMT_CLK_SRC_DEFAULT,
      .resolution_hz = 10 * 1000 * 1000, // 10MHz
      .mem_block_symbols = 0,            // Default
      .flags = {.with_dma = false},
  };

  ESP_ERROR_CHECK(
      led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));
  led_matrix_clear();
}

void led_matrix_clear() { led_strip_clear(led_strip); }

void led_matrix_set_pixel(int x, int y, uint8_t r, uint8_t g, uint8_t b) {
  uint32_t idx = get_pixel_index(x, y);
  if (idx < LED_STRIP_LEN) {
    led_strip_set_pixel(led_strip, idx, r, g, b);
  }
}

void led_matrix_refresh() { led_strip_refresh(led_strip); }

void led_matrix_draw_text(const char *text, int x_offset, uint8_t r, uint8_t g,
                          uint8_t b, bool scroll, int delay_ms) {
  int original_len = strlen(text);
  char *decoded = (char *)malloc(original_len + 1);
  if (!decoded)
    return;

  int len = 0;
  for (int i = 0; i < original_len; i++) {
    if ((unsigned char)text[i] == 0xC3 && i + 1 < original_len) {
      i++;
      unsigned char c2 = (unsigned char)text[i];
      if (c2 == 0x84)
        decoded[len++] = 128; // Ä
      else if (c2 == 0x96)
        decoded[len++] = 129; // Ö
      else if (c2 == 0x9C)
        decoded[len++] = 130; // Ü
      else if (c2 == 0xA4)
        decoded[len++] = 131; // ä
      else if (c2 == 0xB6)
        decoded[len++] = 132; // ö
      else if (c2 == 0xBC)
        decoded[len++] = 133; // ü
      else
        decoded[len++] = '?';
    } else {
      decoded[len++] = text[i];
    }
  }
  decoded[len] = '\0';

  int text_len = len;
  int text_pixel_width = text_len * 8; // 8x8 font

  int current_offset;
  int end_offset = text_pixel_width;

  if (scroll) {
    current_offset = -MATRIX_WIDTH;
  } else {
    current_offset = x_offset;
  }

  bool running = true;
  while (running) {

    if (scroll) {
      for (uint32_t i = 0; i < LED_STRIP_LEN; i++) {
        led_strip_set_pixel(led_strip, i, 0, 0, 0);
      }
    }

    // Render frame
    for (int x = 0; x < MATRIX_WIDTH; x++) {
      int text_col = x + current_offset;

      if (text_col >= 0 && text_col < text_pixel_width) {
        int char_idx = text_col / 8;
        int col_in_char = text_col % 8;

        if (char_idx < text_len) {
          unsigned char c = decoded[char_idx];
          const uint8_t *bitmap = NULL;

          if (c >= 32 && c <= 126) {
            bitmap = font8x8_basic[c - 32];
          } else if (c >= 128 && c <= 133) {
            bitmap = font8x8_basic[96 + (c - 128)];
          }

          if (bitmap != NULL) {
            for (int y = 0; y < MATRIX_HEIGHT; y++) {
              uint8_t row_data = bitmap[y];
              if (row_data & (0x80 >> col_in_char)) {
                led_matrix_set_pixel(x, y, r, g, b);
              }
            }
          }
        }
      }
    }

    if (scroll) {
      led_matrix_refresh();
      vTaskDelay(pdMS_TO_TICKS(delay_ms));
      current_offset++;
      if (current_offset > end_offset) {
        running = false;
      }
    } else {
      running = false; // Static mode runs once
    }
  }
  free(decoded);
}
