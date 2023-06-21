#pragma once
#include <stdbool.h>

#include "adc.h"
struct button;

typedef void (*button_callback_t)(void* arg, int64_t time_us, bool state,
                                  unsigned int voltage, struct button* button);
typedef struct button {
  unsigned char group_id;
  bool state;
  bool once_press;
  unsigned int voltage;
  unsigned int press_time;
  unsigned int max_voltage;
  unsigned int min_voltage;
  unsigned int press_voltage;
  button_callback_t press;
  button_callback_t release;
  button_callback_t press_once;
  void* callback_parameter;
} button_config_t;

typedef struct {
  button_config_t** buttons;
  unsigned char total;
} buttons_config_t;

typedef struct {
  bool debug;
  adc_channel_t adc_channel;
  buttons_config_t* buttons_config;
} button_driver_config_t;

void button_driver_install(button_driver_config_t* button_driver_config,
                           const uint32_t usStackDepth);

button_driver_config_t* button_driver_config_create(button_config_t** buttons,
                                                    unsigned char total,
                                                    adc1_channel_t adc_channel,
                                                    bool debug);

button_config_t* button_create(unsigned char group_id, unsigned int min_voltage,
                               unsigned int max_voltage, bool init_state,
                               button_callback_t press,
                               button_callback_t release,
                               button_callback_t press_once,
                               void* callback_parameter);
