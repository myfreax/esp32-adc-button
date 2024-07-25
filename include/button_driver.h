#pragma once
#include <stdbool.h>
#include <stdint.h>

struct button;

typedef void (*button_callback_t)(void* arg, int64_t time_us, bool state,
                                  struct button* button);
typedef uint32_t (*sampling_func_t)(void* arg);

typedef struct {
  button_callback_t press;
  float time;
  bool status;
} long_press_t;

typedef struct button {
  char* name;
  unsigned char grouping_id;
  bool state;
  bool once_press;
  unsigned int value;
  long long int start_time;
  unsigned int max_value;
  unsigned int min_value;
  button_callback_t release;
  button_callback_t press_once;
  button_callback_t long_press;
  float long_press_time;
  bool long_press_status;
  long_press_t** long_presses;
  uint8_t long_presses_length;
  void* callback_parameter;
} button_config_t;

typedef struct {
  button_config_t** buttons;
  unsigned char total;
} buttons_config_t;

typedef struct {
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
    sampling_func_t sampling_func);

long_press_t* button_create_press(button_callback_t callback,
                                  float long_press_time);

button_config_t* button_create(char* name, unsigned char grouping_id,
                               unsigned int min_value, unsigned int max_value,
                               bool init_state, button_callback_t release,
                               button_callback_t press_once,
                               long_press_t** long_presses,
                               uint8_t long_presses_length,
                               void* callback_parameter);
