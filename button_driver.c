#include "button_driver.h"

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "time.h"

static const char* TAG = "BUTTON DRIVER";
button_config_t* button_create(unsigned char group_id, unsigned int min_voltage,
                               unsigned int max_voltage,
                               button_callback_t press,
                               button_callback_t release,
                               button_callback_t press_once,
                               void* callback_parameter) {
  button_config_t* button = malloc(sizeof(button_config_t));
  button->group_id = group_id;
  button->state = false;
  button->once_press = false;
  button->press_time = 0;
  button->voltage = 0;
  button->press_voltage = 0;
  button->min_voltage = min_voltage;
  button->max_voltage = max_voltage;
  button->press = press;
  button->release = release;
  button->press_once = press_once;
  button->callback_parameter = callback_parameter;
  return button;
}

button_driver_config_t* button_driver_config_create(button_config_t** buttons,
                                                    unsigned char total,
                                                    adc1_channel_t adc_channel,
                                                    bool debug) {
  buttons_config_t* buttons_config = malloc(sizeof(buttons_config_t));
  buttons_config->total = total;
  buttons_config->buttons = buttons;
  button_driver_config_t* button_driver_config =
      malloc(sizeof(button_driver_config_t));
  button_driver_config->buttons_config = buttons_config;
  button_driver_config->adc_channel = adc_channel;
  button_driver_config->debug = debug;
  return button_driver_config;
}

static void reset_other_button_state(buttons_config_t* config,
                                     button_config_t* current_button) {
  for (unsigned char i = 0; i < config->total; i++) {
    button_config_t* button = config->buttons[i];
    if (button != current_button &&
        current_button->group_id == button->group_id) {
      button->state = false;
    }
  }
}

static void button_task(void* arg) {
  button_driver_config_t* button_driver_config = arg;
  buttons_config_t* config = button_driver_config->buttons_config;
  esp_adc_cal_characteristics_t* adc_chars =
      adc_config(button_driver_config->adc_channel, ADC_WIDTH_BIT_DEFAULT, 1100,
                 ADC_ATTEN_DB_11);
  while (1) {
    uint32_t voltage =
        adc_voltage(button_driver_config->adc_channel, adc_chars);
    if (button_driver_config->debug) {
      ESP_LOGI(TAG, "Voltage: %d", voltage);
    }
    for (unsigned char i = 0; i < config->total; i++) {
      button_config_t* button = config->buttons[i];
      if (voltage < button->max_voltage && voltage > button->min_voltage) {
        int64_t time_us = time_currnet_us();
        if (button->press_time == 0) {
          button->press_time = time_us;
          button->press_voltage = voltage;
        } else {
          if (!button->once_press && button->press_once != NULL) {
            button->press_once(button->callback_parameter,
                               time_us - button->press_time, button->state,
                               voltage);
            button->once_press = true;
          }

          if (button->press != NULL) {
            button->press(button->callback_parameter,
                          time_us - button->press_time, button->state, voltage);
          }
        }
      } else {
        if (button->press_time != 0) {
          int64_t time_us = time_currnet_us();
          if ((time_us - button->press_time) > 30000 &&
              button->release != NULL) {
            button->state = (button->state == false) ? true : false;
            if (button->state) {
              reset_other_button_state(config, button);
            }
            if (voltage > button->min_voltage) {
              button->release(button->callback_parameter,
                              time_us - button->press_time, button->state,
                              voltage);
              button->once_press = false;
            } else if (button_driver_config->debug) {
              button->release(button->callback_parameter,
                              time_us - button->press_time, button->state,
                              voltage);
              button->once_press = false;
            }
          }
          button->press_time = 0;
        }
      }
    }
    vTaskDelay(20 / portTICK_PERIOD_MS);
  }
}

void button_driver_install(button_driver_config_t* button_driver_config,
                           const uint32_t usStackDepth) {
  xTaskCreate(&button_task, "button_task", usStackDepth, button_driver_config,
              10, NULL);
}