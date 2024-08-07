#include "button_driver.h"

#include "datetime.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#ifdef CONFIG_BUTTON_DRIVER_DEBUG
static const char* TAG = "BUTTON DRIVER";
#endif

long_press_t* button_create_press(button_callback_t callback,
                                  float long_press_time) {
  long_press_t* button = malloc(sizeof(long_press_t));
  button->press = callback;
  button->time = long_press_time;
  button->status = false;
  return button;
}

button_config_t* button_create(char* name, unsigned char grouping_id,
                               unsigned int min_value, unsigned int max_value,
                               bool init_state, button_callback_t release,
                               button_callback_t press_once,
                               long_press_t** long_presses,
                               uint8_t long_presses_length,
                               void* callback_parameter) {
  button_config_t* button = malloc(sizeof(button_config_t));
  button->grouping_id = grouping_id;
  button->state = init_state;
  button->once_press = false;
  button->start_time = 0;
  button->value = 0;
  button->name = name;
  button->min_value = min_value;
  button->max_value = max_value;
  button->release = release;
  button->press_once = press_once;
  button->long_press_status = false;
  button->callback_parameter = callback_parameter;
  button->long_presses = long_presses;
  button->long_presses_length = long_presses_length;
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

static void reset_press_time(buttons_config_t* config,
                             button_config_t* current_button) {
  for (unsigned char i = 0; i < config->total; i++) {
    button_config_t* button = config->buttons[i];
    if (button != current_button) {
      button->start_time = 0;
    }
  }
}

static void reset_state(buttons_config_t* config,
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
      int64_t current_time = time_current_us();
      button_config_t* button = config->buttons[i];
      if (value < button->max_value && value > button->min_value) {
        if (button->start_time == 0) {
          button->start_time = current_time;
        } else {
          if (!button->once_press && button->press_once != NULL) {
            button->press_once(button->callback_parameter,
                               current_time - button->start_time, button->state,
                               button);
            button->once_press = true;
          }
#ifdef CONFIG_BUTTON_DRIVER_DEBUG
          ESP_LOGI(TAG, "%s button press time: %lld state: %d value: %ld",
                   button->name, current_time - button->start_time,
                   button->state, value);
#endif
          if (button->long_presses != NULL && button->long_presses_length > 0) {
            for (uint8_t i = 0; i < button->long_presses_length; i++) {
              long_press_t* long_press = button->long_presses[i];
              if (long_press != NULL) {
                if (current_time - button->start_time >
                        long_press->time * 1000 * 1000 &&
                    !long_press->status) {
                  if (long_press->press != NULL) {
#ifdef CONFIG_BUTTON_DRIVER_DEBUG
                    ESP_LOGI(
                        TAG,
                        "%s button long press time: %lld state: %d value: %ld",
                        button->name, current_time - button->start_time,
                        button->state, value);
#endif
                    button->long_press_status = true;
                    long_press->status = true;
                    long_press->press(button->callback_parameter,
                                      current_time - button->start_time,
                                      button->state, button);
                  }
                }
              }
            }
          }
        }
      } else {
        if (button->start_time != 0) {
          if ((current_time - button->start_time) >
              button_driver_config->debounce_us) {
            button->state = !button->state;
            if (button->state) {
              reset_state(config, button);
            }
            if (value > button->min_value) {
#ifdef CONFIG_BUTTON_DRIVER_DEBUG
              ESP_LOGI(TAG, "%s button release time: %lld state: %d value: %ld",
                       button->name, current_time - button->start_time,
                       button->state, value);
#endif
              if (!button->long_press_status && button->release != NULL) {
                button->release(button->callback_parameter,
                                current_time - button->start_time,
                                button->state, button);
              }
              reset_press_time(config, button);
              button->once_press = false;
              for (uint8_t i = 0; i < button->long_presses_length; i++) {
                long_press_t* long_press = button->long_presses[i];
                if (long_press != NULL) {
                  long_press->status = false;
                }
              }
              button->long_press_status = false;
            }
#ifdef CONFIG_BUTTON_DRIVER_DEBUG
            else {
              ESP_LOGI(TAG, "%s button release time: %lld state: %d value: %ld",
                       button->name, current_time - button->start_time,
                       button->state, value);
              if (!button->long_press_status && button->release != NULL) {
                button->release(button->callback_parameter,
                                current_time - button->start_time,
                                button->state, button);
              }
              button->once_press = false;
              for (uint8_t i = 0; i < button->long_presses_length; i++) {
                long_press_t* long_press = button->long_presses[i];
                if (long_press != NULL) {
                  long_press->status = true;
                }
              }
              button->long_press_status = false;
            }
#endif
          }
          button->start_time = 0;
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
              uxPriority, NULL);
}