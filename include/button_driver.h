#pragma once
#include <stdbool.h>

#include "adc.h"

typedef void (*button_callback_t)(int64_t time_us, bool state);

typedef struct {
  bool state;
  unsigned int press_time;
  unsigned int max_voltage;
  unsigned int min_voltage;
  button_callback_t press;
  button_callback_t lift;
} button_config_t;

typedef struct {
  button_config_t** buttons;
  unsigned char total;
} buttons_config_t;

typedef struct {
  adc_channel_t adc_channel;
  buttons_config_t* buttons_config;
} button_driver_config_t;

void button_driver_install(button_driver_config_t* button_driver_config);

button_driver_config_t* button_driver_config_create(button_config_t** buttons,
                                                    unsigned char total,
                                                    adc1_channel_t adc_channel);

button_config_t* button_create(unsigned int min_voltage,
                               unsigned int max_voltage,
                               button_callback_t press, button_callback_t lift);
