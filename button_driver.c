#include "button_driver.h"

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "time.h"

#ifdef CONFIG_BUTTON_DRIVER_DEBUG
static const char* TAG = "BUTTON DRIVER";
#endif
button_config_t* button_create(char* name, unsigned char grouping_id,
                               unsigned int min_value, unsigned int max_value,
                               bool init_state, button_callback_t press,
                               button_callback_t release,
                               button_callback_t press_once,
                               void* callback_parameter) {
  button_config_t* button = malloc(sizeof(button_config_t));
  button->grouping_id = grouping_id;
  button->state = init_state;
  button->once_press = false;
  button->press_time = 0;
  button->value = 0;
  button->name = name;
  button->min_value = min_value;
  button->max_value = max_value;
  button->press = press;
  button->release = release;
  button->press_once = press_once;
  button->callback_parameter = callback_parameter;
  return button;
}

button_driver_config_t* button_driver_config_create(
    button_config_t** buttons, unsigned char total, uint8_t sampling_rate,
    uint32_t debounce_us, void* sampling_parameter,
    sampling_func_t sampling_func) {
  buttons_config_t* buttons_config = malloc(sizeof(buttons_config_t));
  buttons_config->total = total;
  buttons_config->buttons = buttons;
  button_driver_config_t* button_driver_config =
      malloc(sizeof(button_driver_config_t));
  button_driver_config->buttons_config = buttons_config;
  button_driver_config->sampling_rate = sampling_rate;
  button_driver_config->debounce_us = debounce_us;
  button_driver_config->sampling_func = sampling_func;
  button_driver_config->sampling_parameter = sampling_parameter;
  return button_driver_config;
}

static void reset_other_button_state(buttons_config_t* config,
                                     button_config_t* current_button) {
  for (unsigned char i = 0; i < config->total; i++) {
    button_config_t* button = config->buttons[i];
    if (button != current_button &&
        current_button->grouping_id == button->grouping_id) {
      button->state = false;
    }
  }
}

static void button_task(void* arg) {
  button_driver_config_t* button_driver_config = arg;
  buttons_config_t* config = button_driver_config->buttons_config;

  while (1) {
    uint32_t value = button_driver_config->sampling_func(
        button_driver_config->sampling_parameter);
#ifdef CONFIG_BUTTON_DRIVER_SAMPLING_DEBUG
    ESP_LOGI(TAG, "Sampling value: %ld", value);
#endif
    for (unsigned char i = 0; i < config->total; i++) {
      button_config_t* button = config->buttons[i];
      if (value < button->max_value && value > button->min_value) {
        int64_t time_us = time_currnet_us();
        if (button->press_time == 0) {
          button->press_time = time_us;
        } else {
          if (!button->once_press && button->press_once != NULL) {
            button->press_once(button->callback_parameter,
                               time_us - button->press_time, button->state,
                               button);
            button->once_press = true;
          }
#ifdef CONFIG_BUTTON_DRIVER_DEBUG
          ESP_LOGI(TAG, "%s button press time: %lld state: %d value: %ld",
                   button->name, time_us, button->state, value);
#endif
          if (button->press != NULL) {
            button->press(button->callback_parameter,
                          time_us - button->press_time, button->state, button);
          }
        }
      } else {
        if (button->press_time != 0) {
          int64_t time_us = time_currnet_us();
          if ((time_us - button->press_time) >
                  button_driver_config->debounce_us &&
              button->release != NULL) {
            button->state = !button->state;
            if (button->state) {
              reset_other_button_state(config, button);
            }
            if (value > button->min_value) {
#ifdef CONFIG_BUTTON_DRIVER_DEBUG
              ESP_LOGI(TAG, "%s button release time: %lld state: %d value: %ld",
                       button->name, time_us, button->state, value);
#endif
              button->release(button->callback_parameter,
                              time_us - button->press_time, button->state,
                              button);
              button->once_press = false;

            }
#ifdef CONFIG_BUTTON_DRIVER_DEBUG
            else {
              ESP_LOGI(TAG, "%s button release time: %lld state: %d value: %ld",
                       button->name, time_us, button->state, value);
              button->release(button->callback_parameter,
                              time_us - button->press_time, button->state,
                              button);
              button->once_press = false;
            }
#endif
          }
          button->press_time = 0;
        }
      }
    }
    vTaskDelay(button_driver_config->sampling_rate / portTICK_PERIOD_MS);
  }
}

void button_driver_install(button_driver_config_t* button_driver_config,
                           const uint32_t usStackDepth,
                           unsigned int uxPriority) {
  xTaskCreate(&button_task, "button_task", usStackDepth, button_driver_config,
              10, NULL);
}