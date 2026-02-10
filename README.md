# LED Matrix Library for ESP-IDF

This is an ESP-IDF component for controlling WS2812B LED matrices (e.g., 8x32 or 8x64 panels) with a zigzag wiring layout.

## Features

*   **Easy Initialization**: Simple setup on any GPIO pin.
*   **Coordinate Mapping**: automatically handles column-major zigzag wiring (serpentine layout).
*   **Text Drawing**: Built-in 8x8 font support for drawing text.
*   **Pixel Control**: Direct control over individual pixels by (x, y) coordinates.

## Hardware Configuration

The library uses a default configuration defined in `flc_matrix.h`:

*   **Width**: 64 pixels (configurable via `MATRIX_WIDTH`)
*   **Height**: 8 pixels (configurable via `MATRIX_HEIGHT`)
*   **Wiring**: Start top-left, Column Major Zigzag (Even columns go down, Odd columns go up).

## Dependencies

This component requires the [espressif/led_strip](https://components.espressif.com/components/espressif/led_strip) component.

## Usage

### 1. Installation

You can install this component in your ESP-IDF project using one of the following methods.

#### Option A: IDF Component Manager (Recommended)
To ensure you always have the latest version from GitHub, create or edit the `idf_component.yml` file in your project's `main` directory (e.g., `main/idf_component.yml`) and add the following dependency:

```yaml
dependencies:
  led_matrix:
    git: https://github.com/broman8/led_matrix.git
    path: .
    version: "*"
```
*   **Note**: This will fetch the latest version from the default branch (`main`) whenever you run a build (if the commit hash changes) or update dependencies.

#### Option B: Git Submodule
If you use Git for your project, you can add this repository as a submodule:

```bash
git submodule add https://github.com/broman8/led_matrix.git components/led_matrix
```
To update to the latest version later:
```bash
git submodule update --remote components/led_matrix
```

#### Option C: Manual Copy
Copy this directory into the `components` folder of your ESP-IDF project.

### 2. CMakeLists.txt

Ensure your main `CMakeLists.txt` or component requires this library.

```cmake
idf_component_register(...
    REQUIRES led_matrix
)
```

### 3. Example Code

```c
#include "led_matrix.h"

void app_main(void) {
    // Initialize on GPIO 4
    led_matrix_init(4);

    // Clear screen
    led_matrix_clear();

    // Set a single pixel (x=0, y=0) to Red
    led_matrix_set_pixel(0, 0, 50, 0, 0);
    led_matrix_refresh();

    // Draw Text
    led_matrix_clear();
    led_matrix_draw_text("Hello World", 0, 0, 50, 0); // Blue text
    led_matrix_refresh();
}
```

## API Reference

*   `void led_matrix_init(int gpio_pin)`: Initializes the RMT LED strip driver.
*   `void led_matrix_clear()`: Turns off all LEDs (does not refresh automatically).
*   `void led_matrix_set_pixel(int x, int y, uint8_t r, uint8_t g, uint8_t b)`: Sets the color of a specific pixel.
*   `void led_matrix_draw_text(const char *text, int x_offset, uint8_t r, uint8_t g, uint8_t b)`: Draws a string starting at `x_offset`.
*   `void led_matrix_refresh()`: Sends the buffer to the LED strip.
