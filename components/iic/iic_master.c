#include "sdkconfig.h"
#include "gsdc_iic_master.h"
#include "gsdc_iic_transmisson.h"
#include "led_controller.h"
#include "esp_system_includes.h"
#include "esp_logging.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include <string.h>

typedef struct {
    gsdc_iic_configuration_t * configuration;
    gsdc_iic_connected_device_t * device;

} private_client_data_received_parameters_t;


void internal_client_data_received_task(void * parameters);
void internal_create_incoming_data_queue(gsdc_iic_configuration_t * iic_configuration);
esp_err_t internal_i2c_master_init(void);
void internal_incoming_data_queue_monitor_task(void * parameters);
void internal_initialize_if_needed(void);
void internal_master_writer_task(void *parameters);
size_t internal_send_request_to_device(uint8_t * command, gsdc_iic_connected_device_t * device, gsdc_led_controller_t * read_indicator, gsdc_led_controller_t * write_indicator);


static bool IsInitialized = false;
static const char * IIC_MASTER_TAG = "iic_master";
static const char * IIC_MASTER_TASK_TAG = "iic_master_task";

static uint8_t COMMAND[] = GSDC_IIC_COMMANDS_SEND_ALL_DATA;


void internal_client_data_received_task(void * parameters)
{
    private_client_data_received_parameters_t * data_parameters = (private_client_data_received_parameters_t*)parameters;
    configASSERT(data_parameters != NULL);

    data_parameters->configuration->data_received_from_client(data_parameters->device);
    vTaskDelete(NULL);
}

void internal_create_incoming_data_queue(gsdc_iic_configuration_t * iic_configuration)
{
    iic_configuration->IncomingDataQueue = xQueueCreate(iic_configuration->ConnectedDeviceCount, sizeof(private_client_data_received_parameters_t));
    xTaskCreatePinnedToCore(internal_incoming_data_queue_monitor_task, 
                            "internal_incoming_data_queue_monitor_task", 
                            I2C_SLAVE_TX_BUF_LEN * 2, 
                            (void *)iic_configuration->IncomingDataQueue, 
                            2, NULL, 1);
}

esp_err_t internal_i2c_master_init(void)
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

void internal_incoming_data_queue_monitor_task(void * parameters)
{
    QueueHandle_t incomingDataQueue = (QueueHandle_t)parameters;
    private_client_data_received_parameters_t receivedData;

    while(true)
    {
        if(xQueueReceive(incomingDataQueue, &receivedData, pdMS_TO_TICKS(10)) == pdPASS)
        {
            char task_name[36];
            sprintf(task_name, "%x_data_task", receivedData.device->I2CAddress);
            xTaskCreatePinnedToCore(internal_client_data_received_task, 
                                    task_name, 
                                    2048, 
                                    (void*)&receivedData, 
                                    3, NULL, 1);
        }

        taskYIELD();
    }
}

void internal_initialize_if_needed()
{
    if(IsInitialized) {
        return;
    }
    internal_i2c_master_init();
}

void internal_master_writer_task(void *parameters)
 {
    gsdc_iic_configuration_t * configuration = (gsdc_iic_configuration_t*) parameters;

    gsdc_led_controller_t * read_indicator = create_led_controller_configured_for_gpio_pin(READING_GPIO);
    gsdc_led_controller_t * write_indicator = create_led_controller_configured_for_gpio_pin(WRITING_GPIO);

    long duration, loop_start, loop_stop;
    TickType_t xFrequency = pdMS_TO_TICKS(MASTER_POLLING_PERIOD), xLastWakeTime = xTaskGetTickCount ();

    for( ;; )
    {
        loop_start = xTaskGetTickCount();
        for(uint8_t index = 0; index < configuration->ConnectedDeviceCount; index++)
        {
            gsdc_iic_connected_device_t * current_device = configuration->get_connected_device(index);
            memset(&current_device->ReceivedData, 0, current_device->DataLength);
            long start = xTaskGetTickCount();
            internal_send_request_to_device(COMMAND, current_device, read_indicator, write_indicator);
            duration = xTaskGetTickCount() - start;

            ESP_LOGD(IIC_MASTER_TASK_TAG, "Sending command to device [%2X] took [%li] ticks", current_device->I2CAddress, duration);

            private_client_data_received_parameters_t parameters = { configuration, current_device };
            xQueueSendToBack(configuration->IncomingDataQueue, (void*)&parameters, pdMS_TO_TICKS(10));
        }

        loop_stop = xTaskGetTickCount();
        duration = loop_stop - loop_start;
        ESP_LOGW(IIC_MASTER_TAG, "Loop took [%li] ticks from [%li] to [%li] = [%.3f] seconds", 
                                 duration, loop_start, loop_stop, (double)(duration)/100);
        xTaskDelayUntil( &xLastWakeTime, xFrequency );
    }
}

size_t internal_send_request_to_device(uint8_t * command, gsdc_iic_connected_device_t * device, gsdc_led_controller_t * read_indicator, gsdc_led_controller_t * write_indicator)
{
    int ret;
    internal_initialize_if_needed();

    ESP_LOGV(IIC_MASTER_TAG, "Master requesting from IIC device : %2X ", device->I2CAddress);
    write_indicator->invert_led_state(write_indicator);
    ret = iic_master_write_to_slave(device->I2CAddress, command, strlen((char*)command));
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
    iic_master_read_variable_length_from_slave(device);
    read_indicator->invert_led_state(read_indicator);

    return device->DataLength;
}

// ////////////////////////////////////////////////////
//
//                  Public members
//
// ////////////////////////////////////////////////////


void gsdc_iic_master_task_create(gsdc_iic_configuration_t * iic_configuration)
{
    internal_initialize_if_needed();
    internal_create_incoming_data_queue(iic_configuration);
    xTaskCreatePinnedToCore(internal_master_writer_task, "internal_master_writer_task", ((I2C_SLAVE_TX_BUF_LEN * 2)+(sizeof(gsdc_led_controller_t)*2)), (void *)iic_configuration, 10, NULL, 1);
}