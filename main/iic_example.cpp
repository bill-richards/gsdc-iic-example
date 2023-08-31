#include <iic_example.h>
#include <gsdc_bme280_iic.h>
#include <string.h>

static const char * IIC_EXAMPLE_TAG = "iic_example";
gsdc_bme280::GSDC_SENSOR * connected_sensor;

void client_data_received_callback(gsdc_iic_connected_device_t * client);   
void command_received_from_master_callback(const char * command);    


void IIC_Example::initialize_master(gsdc_iic_configuration_t * configuration)
{
    ESP_LOGI(IIC_EXAMPLE_TAG, "Starting the IIC-Master thread ...");
    configuration->data_received_from_client = &client_data_received_callback;

    gsdc_iic_master_task_create(configuration);
    gsdc_iic_send_reset_command_to_connected_devices();
    vTaskDelay(pdMS_TO_TICKS(100));
}

void IIC_Example::initialize_client(gsdc_iic_configuration_t * configuration)
{
    ESP_LOGI(IIC_EXAMPLE_TAG, "Starting the IIC-Client thread ...");
    gsdc_iic_client_create_task(configuration, (gsdc_iic_command_received_event_handler_t *)&command_received_from_master_callback);

    // determine the sensor type attached (this will be taken from the local configuration file)
    connected_sensor = new gsdc_bme280::BME280IIC();
    connected_sensor->Initialize();
}


/**
 * This is where we actually do something with the data received from the connected devices.
 * 
 * This callback function is executed from within its own Task and therefore processing the
 * received data will not interfere with the polling of the connected clients
 */
void client_data_received_callback(gsdc_iic_connected_device_t * client)
{
    ESP_LOGI(IIC_EXAMPLE_TAG, "\t\t\t\t\t\t%4i bytes received from %2X", client->DataLength, client->I2CAddress);
#ifdef SHOW_RECEIVED_DATA
    take_semaphore();
    display_buffer_contents(client->ReceivedData, client->DataLength);
    give_semaphore();
#endif
}

/**
 *  If we want the client to process any commands this is where we 
 *  provide that functionality.
 * 
 * This callback function is executed from within its own Task, and therefore
 * there could be other incoming commands whilst this one is being processed,
 * and so it is preferable to not do anything too time consuming within this function.
 */
void command_received_from_master_callback(const char * command)
{
    ESP_LOGV(IIC_EXAMPLE_TAG, "Command received: %s", command);

    if(strcmp(GSDC_IIC_COMMANDS_RESTART_MCU, command) == 0)
    {
        ESP_LOGW(IIC_EXAMPLE_TAG, "Reset Command received: resetting ...");
        esp_restart();
    } 

    if(strcmp(GSDC_IIC_COMMANDS_SEND_ALL_DATA, command) == 0)
    {
#ifdef CONFIG_USE_TEST_MESSAGE_DATA
        gsdc_iic_client_send_test_data();
#else
        char * collated_data = (char*)calloc(1024, sizeof(char));
        connected_sensor->ReadData(collated_data);
        gsdc_iic_client_send_data(collated_data, strlen(collated_data));
        free(collated_data);
#endif
        return;
    }
}
