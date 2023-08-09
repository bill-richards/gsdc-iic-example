// //////////////////////////////////////////// //
//                DEPENDENCIES                  //
// //////////////////////////////////////////// //
#include <freertos/FreeRTOS.h>                  //
#include <freertos/task.h>                      //
#include <string.h>                             //
//                                              //
#include <esp_logging.h>                        //
#include <gsdc_iic_master.h>                    //
#include <gsdc_iic_client.h>                    //
#include <gsdc_ota_subsystem.h>                 //
//                                              //
#include "main.h"                               //
// //////////////////////////////////////////// //

// //////////////////////////////////////////////////////////////////////// //
//                       FORWARD DECLARATIONS                               //
// //////////////////////////////////////////////////////////////////////// //
//                                                                          //
void client_data_received_callback(gsdc_iic_connected_device_t * client);   //
void command_received_from_master_callback(const char * command);           //
void initialize_client(gsdc_iic_configuration_t * configuration);           //
void initialize_master(gsdc_iic_configuration_t * configuration);           //
//                                                                          //
// //////////////////////////////////////////////////////////////////////// //

// //////////////////////////////////////////////////////////////// //
//                      LOCAL VARIABLES                             //
// //////////////////////////////////////////////////////////////// //
//                                                                  //
static const char * MAIN_TAG = "main";                              //
//                                                                  //
// //////////////////////////////////////////////////////////////// //

void app_main(void)
{
    gsdc_iic_configuration_file_data_t spiffs_info = {
        partition_label,
        base_path,
        iic_configuration_file_name,
    };

    ESP_LOGI(MAIN_TAG, "Initializing the file system");
    gsdc_iic_configuration_t * configuration = gsdc_iic_configuration_init(&spiffs_info);
    ESP_LOGI(MAIN_TAG, "Loading the IIC configuration");
    configuration->load();
    gsdc_ota_subsystem_initialize(&spiffs_info);

    if(configuration->ConnectedDeviceCount > 0) {
        initialize_master(configuration);
    } else {
        initialize_client(configuration);
    }

// #if CONFIG_GSDC_IIC_IS_MASTER
//      initialize_master(configuration);
// #else
//     initialize_client(configuration);
// #endif

}

void initialize_master(gsdc_iic_configuration_t * configuration)
{
    ESP_LOGI(MAIN_TAG, "Starting the IIC-Master thread ...");
    configuration->data_received_from_client = &client_data_received_callback;

    gsdc_iic_master_task_create(configuration);
    gsdc_iic_send_reset_command_to_connected_devices();
}

void initialize_client(gsdc_iic_configuration_t * configuration)
{
    ESP_LOGI(MAIN_TAG, "Starting the IIC-Receiver thread ...");
    gsdc_iic_client_create_task(configuration, (gsdc_iic_command_received_event_handler_t *)&command_received_from_master_callback);
}

/**
 * This is where we actually do something with the data received from the connected devices.
 * 
 * This callback function is executed from within its own Task and therefore processing the
 * received data will not interfere with the polling of the connected clients
 */
void client_data_received_callback(gsdc_iic_connected_device_t * client)
{
    ESP_LOGI(MAIN_TAG, "\t\t\t\t\t\t%4i bytes received from %2X", client->DataLength, client->I2CAddress);
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
    ESP_LOGV(MAIN_TAG, "Command received: %s", command);
    if(strcmp(GSDC_IIC_COMMANDS_SEND_ALL_DATA, command) == 0)
    {
#ifdef CONFIG_USE_TEST_MESSAGE_DATA
        gsdc_iic_client_send_test_data();
#else
    // collect and send the real data from the ocnnected sensor
    char * collated_data =  "This is not test data, this is data collected outside of the iic component library, and transmitted by the iic component";
    gsdc_iic_client_send_data(collated_data, strlen(collated_data));
#endif
    }
    else if(strcmp(GSDC_IIC_COMMANDS_RESTART_MCU, command) == 0)
    {
        ESP_LOGW(MAIN_TAG, "Reset Command received: resetting ...");
        esp_restart();
    }
}
