#include "sdkconfig.h"
#include "master_task.h"
#include "iic_transmisson.h"

#include "freertos/FreeRTOS.h"
#include "esp_system_includes.h"
#include "esp_logging.h"

#include <string.h>

void data_received_callback_task(void * parameters);
void master_writer_task(void *parameters);
esp_err_t i2c_master_init(void);
void initialize_if_needed(void);

static bool IsInitialized = false;
static const char * IIC_MASTER_TAG = "iic_master";
static const char * IIC_MASTER_TASK_TAG = "iic_master_task";

static uint8_t COMMAND[] = IIC_COMMANDS_SEND_ALL_DATA;


void i2c_master_task_create(iic_configuration_t * iic_configuration)
{
    initialize_if_needed();
    xTaskCreatePinnedToCore(master_writer_task, "master_writer_task", ((I2C_SLAVE_TX_BUF_LEN * 2)+ (sizeof(led_controller_t)*2)), (void *)iic_configuration, 10, NULL, 1);
}

void initialize_if_needed()
{
    if(IsInitialized) {
        return;
    }
    i2c_master_init();
}

/**
 * @brief i2c master initialization
 */
esp_err_t i2c_master_init(void)
{
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
        // .clk_flags = 0,          /*!< Optional, you can use I2C_SCLK_SRC_FLAG_* flags to choose i2c source clock here. */
    };
    ESP_LOGD(IIC_MASTER_TAG, "Configuring i2c");

    esp_err_t err = i2c_param_config(CONFIG_I2C_PORT_NUM, &conf);
    if (err != ESP_OK) {
        ESP_LOGE(IIC_MASTER_TAG, "Error [%i] configuring i2c", err);
        return err;
    }

    err = i2c_driver_install(CONFIG_I2C_PORT_NUM, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
    ESP_LOGW(IIC_MASTER_TAG, "Installing i2c driver returned [%i] ", err);
    if(err == 0){
        IsInitialized = true;
    }

    return err;
}

typedef struct {
    iic_configuration_t * configuration;
    connected_device_t * device;

} client_data_received_parameters_t;

void client_data_received_task(void * parameters)
{
    client_data_received_parameters_t * data_parameters = (client_data_received_parameters_t*)parameters;
    data_parameters->configuration->data_received_from_client(data_parameters->device);
    vTaskDelete(NULL);
}

void master_writer_task(void *parameters)
 {
    iic_configuration_t * configuration = (iic_configuration_t*) parameters;

    led_controller_t * read_indicator = get_led_controller_configured_for_gpio_pin(READING_GPIO);
    led_controller_t * write_indicator = get_led_controller_configured_for_gpio_pin(WRITING_GPIO);

    long duration, loop_start, loop_stop;
    TickType_t xFrequency = pdMS_TO_TICKS(MASTER_POLLING_PERIOD), xLastWakeTime = xTaskGetTickCount ();

    for( ;; )
    {
        loop_start = xTaskGetTickCount();
        for(uint8_t index = 0; index < configuration->ConnectedDeviceCount; index++)
        {
            connected_device_t * current_device = configuration->get_connected_device(index);
            memset(&current_device->ReceivedData, 0, current_device->DataLength);
            long start = xTaskGetTickCount();
            make_request_to_device(COMMAND, current_device, read_indicator, write_indicator);
            duration = xTaskGetTickCount() - start;

            ESP_LOGD(IIC_MASTER_TASK_TAG, "Sending command to device [%2X] took [%li] ticks", current_device->I2CAddress, duration);

            client_data_received_parameters_t parameters = { configuration, current_device };
            xTaskCreatePinnedToCore(client_data_received_task, "client_data_received_task", I2C_SLAVE_TX_BUF_LEN * 2, (void *)&parameters, 2, NULL, 1);
            // configuration->data_received_from_client(current_device);
        }

        loop_stop = xTaskGetTickCount();
        duration = loop_stop - loop_start;
        ESP_LOGW(IIC_MASTER_TAG, "Loop took [%li] ticks from [%li] to [%li] = [%.3f] seconds", 
                                 duration, loop_start, loop_stop, (double)(duration)/100);
        xTaskDelayUntil( &xLastWakeTime, xFrequency );
    }
}

size_t make_request_to_device(uint8_t * command, connected_device_t * device, led_controller_t * read_indicator, led_controller_t * write_indicator)
{
    int ret;
    initialize_if_needed();

    ESP_LOGV(IIC_MASTER_TAG, "Master requesting from IIC device : %2X ", device->I2CAddress);
    write_indicator->invert_led_state(write_indicator);
    ret = i2c_master_write_to_slave(device->I2CAddress, command, strlen((char*)command));
    write_indicator->invert_led_state(write_indicator);

    if(ret == ESP_ERR_TIMEOUT)
    {
        ESP_LOGW(IIC_MASTER_TAG, "Timeout. No device [%2X] connected ", device->I2CAddress);
    }
    else if(ret == ESP_ERR_NOT_FOUND)
    {
        ESP_LOGW(IIC_MASTER_TAG, "Device %2X not found ", device->I2CAddress);
    }
    else if(ret == ESP_ERR_INVALID_STATE)
    {
        ESP_LOGE(IIC_MASTER_TAG, "Master in invalid state ");
    }
    else if(ret != ESP_FAIL && ret != ESP_OK)
    {
        ESP_LOGE(IIC_MASTER_TAG, "Error %X on i2c_master_cmd_begin. ", ret);
    }

    if(ret != ESP_OK) { return 0; }
    read_indicator->invert_led_state(read_indicator);
    i2c_master_read_variable_length_from_slave(device);
    read_indicator->invert_led_state(read_indicator);

    return device->DataLength;
}