// //////////////////////////////////////////// //
//                                              //
//                DEPENDENCIES                  //
//                                              //
// //////////////////////////////////////////// //
#include "freertos/FreeRTOS.h"                  //
#include "freertos/task.h"                      //
                                                //
#include "esp_logging.h"                        //
#include "configuration_file.h"                 //
#include "iic_configuration.h"                  //
#include "master_task.h"                        //
#include "receiver_task.h"                      //
                                                //
#include <string.h>                             //
// //////////////////////////////////////////// //

// //////////////////////////////////////////////////////////////////// //
//                                                                      //
//                       FORWARD DECLARATIONS                           //
//                                                                      //
// //////////////////////////////////////////////////////////////////// //
                                                                        //
void command_received_from_master_event_handler(const char * command);  //
void client_data_received_callback(connected_device_t * client);        //
                                                                        //
// //////////////////////////////////////////////////////////////////// //

static const char * MAIN_TAG = "main";

/**
 *  These values are taken from sdkconfig
 */
#ifdef CONFIG_GSDC_SPIFFS_CONFIG_FILE_SYSTEM_ROOT
static const char * base_path = CONFIG_GSDC_SPIFFS_CONFIG_FILE_SYSTEM_ROOT;
#else
static const char * base_path = "/spiffs";
#endif

#ifdef CONFIG_GSDC_SPIFFS_PARTITION_LABEL
static const char * partition_label = CONFIG_GSDC_SPIFFS_PARTITION_LABEL;
#else 
static const char * partition_label = "data";
#endif

#if CONFIG_GSDC_IIC_OVERRIDE_CONFIGURATION_FILE_NAME
static const char * iic_configuration_file_name = CONFIG_GSDC_IIC_CONFIGURATION_FILE_NAME;
#else
static const char * iic_configuration_file_name = "configuration";
#endif

void initialize_receiver(iic_configuration_t * configuration)
{
    ESP_LOGI(MAIN_TAG, "Starting the IIC receiver thread ...");
    i2c_create_receiver_task(configuration, (iic_command_received_from_master_event_handler_t *)&command_received_from_master_event_handler);
}

void initialize_master(iic_configuration_t * configuration)
{
    ESP_LOGI(MAIN_TAG, "Starting the IIC Master transmission thread ...");
    configuration->data_received_from_client = &client_data_received_callback;
    i2c_master_task_create(configuration);
}

void app_main(void)
{
    configuration_file_data_t spiffs_info = {
        partition_label,
        base_path,
        iic_configuration_file_name,
    };

    ESP_LOGI(MAIN_TAG, "Initializing the file system");
    iic_configuration_t * configuration = gsdc_iic_configuration_init(&spiffs_info);
    ESP_LOGI(MAIN_TAG, "Loading the IIC configuration");
    configuration->load();

#if CONFIG_GSDC_IIC_IS_MASTER
     initialize_master(configuration);
#else
     initialize_receiver(configuration);
#endif
}

/**
 * This is where we actually do something with the data received from the connected devices.
 * 
 * This callback function is executed from within its own Task and therefore processing the
 * received data will not interfere with the polling of the connected clients
 */
void client_data_received_callback(connected_device_t * client)
{
    char message[128];
    sprintf(message, "                                      %4i bytes received from %2X\n", client->DataLength, client->I2CAddress);
    take_semaphore();
    display_buffer_contents((uint8_t *)&message, strlen(message));
    display_buffer_contents(client->ReceivedData, client->DataLength);
    give_semaphore();
}

/**
 *  If we want the client to process any commands this is where we 
 *  provide that functionality.
 * 
 * This callback function is executed from within its own Task, and therefore
 * there could be other incoming commands whilst this one is being processed,
 * and so it is preferable to not do anything too time consuming within this function.
 */
void command_received_from_master_event_handler(const char * command)
{
    ESP_LOGV(MAIN_TAG, "Command received: %s", command);
    if(strcmp(IIC_COMMANDS_SEND_ALL_DATA, command) == 0)
    {
#ifdef CONFIG_USE_TEST_MESSAGE_DATA
        receiver_send_test_data();
#else
    // collect and send the real data from the ocnnected sensor
    char * collated_data =  "This is not test data, this is data collected outside of the iic component library, and transmitted by the iic component";
    i2c_send_data(collated_data, strlen(collated_data));
#endif
    }
}
