# ESP32 ADC Button

Can be used not only for buttons driven via ADC, but also for other driving methods, depending on the sampling function you pass

## Feature

- Detect button press, long press and release event and triger event callback
- Allow Pass custom parameter to event callback function
- Pass button open and close state to callback function
- Allow you grouping button by set grouping id
- Automatically reset the state of other buttons in the current group
- Combine button support. You can press one of button then press other button
- Define multiple long buttons to use for a specified time
## Dependences
- [ESP32 Time Component](https://github.com/myfreax/esp32-time)

## Steup
```shell
git submodule add git@github.com:myfreax/esp32-time.git components/datetime
git submodule add git@github.com:myfreax/esp32-adc-button.git components/button_driver
```

## Usage
```c
#include "button_driver.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
static uint32_t button_sampling_voltage(void* arg) {
  // your code ...
  return 1234;
}
static void light_long_press(void* arg, int64_t time_us, bool state, button_config_t* button) {}
static void light_release(void* arg, int64_t time_us, bool state, button_config_t* button) {}
// more ...

void app_main(void) {
  static const char* TAG = "BUTTON-DRIVER";

  // custom params pass button event callback  
  button_params_t button_params = {
      .screen = screen,
  };
  // configure ESP32 ADC
  adc_chan_t* button_adc =
      adc_chan_config(ADC_CHANNEL_0, ADC_BITWIDTH_DEFAULT, ADC_ATTEN_DB_12);

  // power button
  long_press_t* power_presses[] = {
      button_create_press(power_long_press, 1.5),
  };
  button_config_t* power_button = button_create(
      "power", 0, 100, 798, false, power_release, power_press_once,
      power_presses, sizeof(power_presses) / sizeof(long_press_t*), &ctx);
  // light button
  long_press_t* light_presses[] = {
      button_create_press(light_long_press, 1.5),
      button_create_press(light_falsh_mode, 5),
  };
  button_config_t* light_button = button_create(
      "light", 1, 799, 1349, false, light_release, light_press_once,
      light_presses, sizeof(light_presses) / sizeof(long_press_t*), &ctx);
  // AC button
  long_press_t* ac_presses[] = {
      button_create_press(ac_long_press, 1.5),
  };
  button_config_t* ac_button = button_create(
      "AC", 2, 1350, 1869, false, ac_release, ac_press_once, ac_presses,
      sizeof(ac_presses) / sizeof(long_press_t*), &ctx);
  // Standby button
  long_press_t* standby_presses[] = {
      button_create_press(standby_long_press, 1.5),
  };
  button_config_t* standby_button = button_create(
      "standby", 3, 1870, 2139, false, standby_release, standby_press_once,
      standby_presses, sizeof(standby_presses) / sizeof(long_press_t*), &ctx);

  // DC button
  long_press_t* dc_presses[] = {
      button_create_press(dc_long_press, 1.5),
  };
  button_config_t* dc_button = button_create(
      "DC", 4, 2140, 2250, false, dc_release, dc_press_once, dc_presses,
      sizeof(dc_presses) / sizeof(long_press_t*), &ctx);

  button_config_t* buttons[] = {light_button, dc_button, power_button,
                                ac_button, standby_button};
  // create driver configuration params and pass buttons and sampling rate,
  // debounce time, sampling function and params of sampling function
  button_driver_config_t* button_driver_config = button_driver_config_create(
      buttons, sizeof(buttons) / sizeof(button_config_t*), 15, 30000,
      button_adc, button_sampling_voltage);
  button_driver_install(button_driver_config, 8192, 25);

  while (1) {
    ESP_LOGI(TAG, "Main Thread Task");
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}
```