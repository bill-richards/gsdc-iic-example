#ifndef __LED_FLASHER_H__
#define __LED_FLASHER_H__

#include "freertos/FreeRTOS.h"
#include "driver/gpio.h"

#define READING_GPIO CONFIG_READ_GPIO
#define WRITING_GPIO CONFIG_WRITE_GPIO

#ifdef __cplusplus
extern "C" {
#endif

typedef struct led_controller led_controller_t;
typedef struct led controlled_led_t;

struct led {
    gpio_num_t GpioPin;
    uint8_t PinState;
    void(* invert_state)(controlled_led_t * controlled_led);
};

struct led_controller {
    controlled_led_t * ControlledLed;
    void (*invert_led_state)(const led_controller_t* led_controller);
};


led_controller_t * get_led_controller_configured_for_gpio_pin(gpio_num_t gpio_pin);

#ifdef __cplusplus
}
#endif

#endif // __LED_FLASHER_H__