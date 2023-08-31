// //////////////////////////////////////////// //
//                DEPENDENCIES                  //
// //////////////////////////////////////////// //
#include <freertos/FreeRTOS.h>                  //
#include <freertos/task.h>                      //
#include <iic_example.h>                        //
//                                              //
#include "main.h"                               //
// //////////////////////////////////////////// //

// //////////////////////////////////////////////////////////////// //
//                      LOCAL VARIABLES                             //
// //////////////////////////////////////////////////////////////// //
//                                                                  //
static const char * MAIN_TAG = "main";                              //
//                                                                  //
// //////////////////////////////////////////////////////////////// //

extern "C" void app_main(void)
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
    
    ESP_LOGI(MAIN_TAG, "Loading the OTA Update configuration");
    gsdc_ota_subsystem_initialize(&spiffs_info);

    IIC_Example App;
    if(configuration->ConnectedDeviceCount > 0) {
        App.initialize_master(configuration);
    } else {
        App.initialize_client(configuration);
    }
}

