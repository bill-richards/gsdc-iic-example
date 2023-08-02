#include <stdio.h>
#include <string.h>
#include <esp_logging.h>
#include <esp_system_includes.h>
#include <led_controller.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "gsdc_iic_client.h"

void command_received_callback_task(void * parameters);
esp_err_t client_init(gsdc_iic_configuration_t * iicConfiguration);
void internal_client_task(void * parameters);

static const char * IIC_RECEIVER_TAG = "iic_receiver";

char message[] = I2C_TEST_DATA;
gsdc_iic_command_received_event_handler_t command_received_event_handler;


void gsdc_iic_client_create_task(gsdc_iic_configuration_t * iicConfiguration, gsdc_iic_command_received_event_handler_t * handler)
{
    command_received_event_handler = (gsdc_iic_command_received_event_handler_t)handler;
    client_init(iicConfiguration);
    xTaskCreate(internal_client_task, "internal_client_task", I2C_SLAVE_TX_BUF_LEN * 3, (void *)1, 10, NULL);
}

/**
 * @brief i2c slave initialization
 */
esp_err_t client_init(gsdc_iic_configuration_t * iicConfiguration)
{
    int iicPortNumber = I2C_SLAVE_NUM;
    i2c_config_t iicSlaveConfiguration = {
        .mode = I2C_MODE_SLAVE,
        .sda_io_num = I2C_SLAVE_SDA_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = I2C_SLAVE_SCL_IO,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .slave.addr_10bit_en = 0,
        .slave.slave_addr = iicConfiguration->I2CAddress, 
    };

    ESP_LOGI(IIC_RECEIVER_TAG, "Configuring iic for receiver %x", iicConfiguration->I2CAddress);

    esp_err_t err = i2c_param_config(iicPortNumber, &iicSlaveConfiguration);
    if (err != ESP_OK) {
        ESP_LOGE(IIC_RECEIVER_TAG, "Error [%i] configuring i2c", err);
        return err;
    }

    err = i2c_driver_install(iicPortNumber, iicSlaveConfiguration.mode, I2C_SLAVE_RX_BUF_LEN, I2C_SLAVE_TX_BUF_LEN, 0);
    
    ESP_LOGW(IIC_RECEIVER_TAG, "Installing i2c driver returned [%i] ", err);
    return err;
}

void gsdc_iic_client_send_test_data()
{
    char data_size[IIC_MESSAGE_LENGTH_FIELD_SIZE+1];
    size_t message_length = strlen(message);
    sprintf(data_size, "%4i", message_length);

    i2c_slave_write_buffer(I2C_SLAVE_NUM, (uint8_t*)data_size, IIC_MESSAGE_LENGTH_FIELD_SIZE, 10 / portTICK_PERIOD_MS);
    i2c_slave_write_buffer(I2C_SLAVE_NUM, (uint8_t*)&message, message_length, 10 / portTICK_PERIOD_MS);
}

void gsdc_iic_client_send_data(const char * out_message, size_t length)
{
    char data_size[IIC_MESSAGE_LENGTH_FIELD_SIZE+1];
    sprintf(data_size, "%4i", length);
    i2c_slave_write_buffer(I2C_SLAVE_NUM, (uint8_t*)data_size, IIC_MESSAGE_LENGTH_FIELD_SIZE, 10 / portTICK_PERIOD_MS);
    i2c_slave_write_buffer(I2C_SLAVE_NUM, (uint8_t*)out_message, length, 10 / portTICK_PERIOD_MS);
}

void internal_client_task(void * parameters) 
{
    int size;
    uint8_t * szCommand = (uint8_t *)calloc(GSDC_IIC_COMMAND_LENGTH, sizeof(uint8_t));
    led_controller_t * readingLEDController = create_led_controller_configured_for_gpio_pin(READING_GPIO);
    
    for( ;; )
    {
        readingLEDController->invert_led_state(readingLEDController);
        size = i2c_slave_read_buffer(I2C_SLAVE_NUM, szCommand, GSDC_IIC_COMMAND_LENGTH, 1000 / portTICK_PERIOD_MS);
        readingLEDController->invert_led_state(readingLEDController);        
        szCommand[GSDC_IIC_COMMAND_LENGTH] = '\0';
        if(size <= 0) { continue; }
        
        xTaskCreate(command_received_callback_task, "command_received_callback_task", I2C_SLAVE_TX_BUF_LEN / 2, (void *)szCommand, 10, NULL);
        portYIELD();
    }
}

void command_received_callback_task(void * parameters)
{
    char * szCommand = (char *)parameters;
    // invoke the callback
    command_received_event_handler(szCommand);
    // delete task when finished
    vTaskDelete(NULL);
}