#ifndef __LED_FLASHER_H__
#define __LED_FLASHER_H__

#include "freertos/FreeRTOS.h"
#include "driver/gpio.h"

#define READING_GPIO CONFIG_READ_GPIO
#define WRITING_GPIO CONFIG_WRITE_GPIO

#ifdef __cplusplus
extern "C" {
#endif
/**
 * @brief structure used to control a configured LED
 * @param ControlledLed (gsdc_controlled_led_t * ) a structure representing the LED being controlled
 */
typedef struct led_controller gsdc_led_controller_t;
/**
 * @brief structure to describe and control an LED
 * @param GpioPin (gpio_num_t) the GPIO pin to which the LED is connected
 * @param PinState (uint8_t) the state of the GPIO (HIGH/LOW)
 */
typedef struct led gsdc_controlled_led_t;

struct led {
    // @brief the GPIO pin to which the LED is connected
    gpio_num_t GpioPin;
    // @brief the state of the GPIO (HIGH/LOW)
    uint8_t PinState;
    /**
     * @brief inverts the PinState of this gsdc_controlled_led_t
     * @param controlled_led (gsdc_controlled_led_t *) a structure representing the LED to be inverted
     */
    void(* invert_state)(gsdc_controlled_led_t * controlled_led);
};

struct led_controller {
    // @brief a structure representing the LED being controlled
    gsdc_controlled_led_t * ControlledLed;
    /**
     * @brief inverts the state of the controlled LED specified
     * @param led_controller (const gsdc_led_controller_t *) the controller of the LED whose state should be inverted
     */
    void (* invert_led_state)(const gsdc_led_controller_t * led_controller);
};

/**
 * @brief Creates a gsdc_led_controller_t instance for an LED connected to the specified GPIO pin
 * @param gpio_pin (gpio_num_t) the number of the GPIO pin to which the LED will be connected
 */
gsdc_led_controller_t * create_led_controller_configured_for_gpio_pin(gpio_num_t gpio_pin);

#ifdef __cplusplus
}
#endif

#endif // __LED_FLASHER_H__