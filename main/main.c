//#define SHOW_RECEIVED_DATA  /* Un-comment to turn on extra tracing output*/

// //////////////////////////////////////////// //
//                                              //
//                DEPENDENCIES                  //
//                                              //
// //////////////////////////////////////////// //
#include <freertos/FreeRTOS.h>                  //
#include <freertos/task.h>                      //
#include <string.h>                             //
                                                //
#include <esp_logging.h>                        //
#include "gsdc_iic_master.h"                    //
#include <gsdc_iic_receiver.h>                  //
                                                //
#include "main.h"                               //
// //////////////////////////////////////////// //

// //////////////////////////////////////////////////////////////////////// //
//                                                                          //
//                       FORWARD DECLARATIONS                               //
//                                                                          //
// //////////////////////////////////////////////////////////////////////// //
                                                                            //
void command_received_from_master_callback(const char * command);           //
void client_data_received_callback(gsdc_iic_connected_device_t * client);   //
void initialize_receiver(gsdc_iic_configuration_t * configuration);         //
void initialize_master(gsdc_iic_configuration_t * configuration);           //
                                                                            //
// //////////////////////////////////////////////////////////////////////// //

static const char * MAIN_TAG = "main";

void initialize_receiver(gsdc_iic_configuration_t * configuration)
{
    ESP_LOGI(MAIN_TAG, "Starting the IIC-Receiver thread ...");
    gsdc_iic_receiver_create_task(configuration, (gsdc_iic_command_received_event_handler_t *)&command_received_from_master_callback);
}

void initialize_master(gsdc_iic_configuration_t * configuration)
{
    ESP_LOGI(MAIN_TAG, "Starting the IIC-Master thread ...");
    configuration->data_received_from_client = &client_data_received_callback;
    gsdc_iic_master_task_create(configuration);
}

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
void client_data_received_callback(gsdc_iic_connected_device_t * client)
{
    ESP_LOGI(MAIN_TAG, "                                      %4i bytes received from %2X", client->DataLength, client->I2CAddress);
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
        gsdc_iic_receiver_send_test_data();
#else
    // collect and send the real data from the ocnnected sensor
    char * collated_data =  "This is not test data, this is data collected outside of the iic component library, and transmitted by the iic component";
    gsdc_iic_receiver_send_data(collated_data, strlen(collated_data));
#endif
    }
}
