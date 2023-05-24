# ESP32 ADC Button
Button component driver by ADC voltage sampling

## Feature
- Detect button press and release event and triger event callback
- Allow Pass custom parameter to event callback function
- Calculate button long press time and pass it to callback function
- Pass button open and close state to callback function
- Allow you group button by set group id
- Automatically reset the state of other buttons in the current group

## Dependences
- [ESP32 ADC Component](https://github.com/myfreax/esp32-adc)
- [ESP32 Time Component](https://github.com/myfreax/esp32-time)

## Usage
```shell
git submodule add git@github.com:myfreax/esp32-adc-button.git components/button_driver
```
```c
#include "button_driver.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void button_led_press(void* arg, int64_t time_us, bool state, uint32_t voltage) {}
void button_led_release(void* arg, int64_t time_us, bool state, uint32_t voltage) {}

void app_main(void) {
  static const char* TAG = "BUTTON-DRIVER";

  button_config_t* button_led = button_create(0, 1100, 1300, &button_led_press,
                                              &button_led_release, &custom_parameter);
  button_config_t* button_15 = button_create(1, 800, 1000, &button_15V_press,
                                              &button_15_release, NULL);
  button_config_t* button_12 = button_create(1, 300, 500, NULL,
                                              &button_12_release, &custom_parameter);

  button_config_t* buttons[3] = {button_12v, button_15v, button_24v,
                                 button_display, button_led};

  button_driver_config_t* button_driver_config = button_driver_config_create(
      buttons, sizeof(buttons) / sizeof(button_config_t*), ADC1_CHANNEL_0);

  button_driver_install(button_driver_config);

  while (1) {
    ESP_LOGI(TAG, "Main Thread Task");
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}
```

## Example
The [ESP32 Example Project](https://github.com/myfreax/esp32-example-project) demonstrates how to use IDF framework build application of reusable component