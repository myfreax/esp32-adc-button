#pragma once
#include <stdbool.h>

#include "adc.h"
struct button;

typedef void (*button_callback_t)(void* arg, int64_t time_us, bool state,
                                  struct button* button);
typedef uint32_t (*sampling_func_t)(void* arg);

typedef struct button {
  char* name;
  unsigned char grouping_id;
  bool state;
  bool once_press;
  unsigned int voltage;
  unsigned int press_time;
  unsigned int max_voltage;
  unsigned int min_voltage;
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
  uint8_t sampling_rate;
  uint32_t debounce_us;
  buttons_config_t* buttons_config;
  void* sampling_parameter;
  sampling_func_t sampling_func;
} button_driver_config_t;

void button_driver_install(button_driver_config_t* button_driver_config,
                           const uint32_t usStackDepth,
                           unsigned int uxPriority);

button_driver_config_t* button_driver_config_create(
    button_config_t** buttons, unsigned char total, uint8_t sampling_rate,
    uint32_t debounce_us, void* sampling_parameter,
    sampling_func_t sampling_func, bool debug);

button_config_t* button_create(char* name, unsigned char grouping_id,
                               unsigned int min_voltage,
                               unsigned int max_voltage, bool init_state,
                               button_callback_t press,
                               button_callback_t release,
                               button_callback_t press_once,
                               void* callback_parameter);
