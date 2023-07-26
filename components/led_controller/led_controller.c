#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_logging.h"
#include "driver/gpio.h"
#include "led_controller.h"

static const char * LED_CONTROLLER_TAG = "led_controller";


///////////////////////////////////////////////////////////////
//
// These are private functions; they are the callback
// functions providing functionality for the led_controller_t
//
///////////////////////////////////////////////////////////////

void controller_invert_led_state(const led_controller_t * led_controller)
{
    led_controller->ControlledLed->PinState = !led_controller->ControlledLed->PinState;
    led_controller->ControlledLed->invert_state(led_controller->ControlledLed);
}

void led_invert_led(controlled_led_t * controlled_led)
{
    ESP_LOGV(LED_CONTROLLER_TAG, "calling gpio_set_level(%i, %i)", controlled_led->GpioPin, controlled_led->PinState);
    gpio_set_level(controlled_led->GpioPin, controlled_led->PinState);
}


///////////////////////////////////////////////////////////////
//
// This is the function exposed from led_controller.h
//
///////////////////////////////////////////////////////////////

led_controller_t * get_led_controller_configured_for_gpio_pin(gpio_num_t gpio_pin)
{
    ESP_LOGI(LED_CONTROLLER_TAG, "Configured to blink GPIO LED!");
    
    led_controller_t * led_controller = (led_controller_t*)calloc(1, sizeof(led_controller_t));
    led_controller->ControlledLed = (controlled_led_t*)calloc(1, sizeof(controlled_led_t));
    led_controller->ControlledLed->GpioPin = gpio_pin;
    led_controller->ControlledLed->invert_state = &led_invert_led;
    led_controller->ControlledLed->PinState = 1;
    led_controller->invert_led_state = &controller_invert_led_state;

    gpio_reset_pin(gpio_pin);
    gpio_set_direction(gpio_pin, GPIO_MODE_OUTPUT);
    return led_controller;
}

