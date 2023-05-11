# ESP32 ADC Button
Button component driven by ADC voltage sampling

## Usage
```shell
git submodule add git@github.com:myfreax/esp32-adc-button.git components/button_driver
```
```c
#include "button_driver.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char* TAG = "BUTTON-DRIVER";

void button_usb_press(int64_t time_us) {
  ESP_LOGI(TAG, "usb button press time: %lld", time_us);
}

void button_usb_lift(int64_t time_us) {
  ESP_LOGI(TAG, "usb button lift time: %lld", time_us);
}

void app_main(void) {
  button_config_t* usb_button =
      button_create(279, 569, &button_usb_press, &button_usb_lift);

  button_config_t* buttons[1] = {usb_button};

  button_driver_config_t* button_driver_config = button_driver_config_create(
      buttons, sizeof(buttons) / sizeof(button_config_t*), ADC1_CHANNEL_0);

  button_driver_install(button_driver_config);

  while (1) {
    ESP_LOGI(TAG, "Main Thread Task");
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}
```
## Debug
`idf.py menuconfig` -> `Button Driver` -> `Enable Button Driver Debug`