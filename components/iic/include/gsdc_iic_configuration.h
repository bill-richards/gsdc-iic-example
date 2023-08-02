#ifndef __IIC_CONFIGURATION_H__
#define __IIC_CONFIGURATION_H__

#include <stdbool.h>
#include <configuration_file.h>
#include <gsdc_iic_common.h>

#include "freertos/FreeRTOS.h"

#ifdef __cplusplus
extern "C" {
#endif

#define IIC_CONFIGURATION_MAXIMUM_CONNECTED_DEVICES 10

// @brief redefine gsdc_configuration_file_descriptor_t to encapsulate the configuration_file component
typedef gsdc_configuration_file_descriptor_t gsdc_iic_configuration_file_data_t;

/**
 * @brief The configuration object is used to manipulate the IIC configuration file
 * @param SpiffsInfo (gsdc_iic_configuration_file_data_t *) SPIFFS description
 * @param ConnectedDevices (gsdc_iic_connected_device_t *) array containing all of the connected devices
 * @param ConnectedDeviceCount (int) number of devices connected on the IIC Bus
 * @param I2CAddress (unit8_t) my IIC address
 * @param IncomingDataQueue (QueueHandle_t) handle to the queue which will be populated by incoming data (only used by the Master)
 */
typedef struct {
    // @brief SPIFFS description
    gsdc_iic_configuration_file_data_t * SpiffsInfo;
    // @brief Array containing all of the connected devices
    gsdc_iic_connected_device_t * ConnectedDevices;
    // @brief number of devices connected on the IIC Bus
    int ConnectedDeviceCount;
    // @brief my IIC address
    uint8_t I2CAddress;
    // @brief Handle to the queue which will be populated by incoming data (only used by the Master)
    QueueHandle_t IncomingDataQueue;

    // @brief pointer to the callback function invoked when data have been received (if this is a Master configuration)
    void (* data_received_from_client)(gsdc_iic_connected_device_t * client);
    // @brief gets a pointer to the connected device from within the array at the specified index 
    gsdc_iic_connected_device_t * (*get_connected_device)(int index);
    // @brief reads in the IIC configuration and populates this gsdc_iic_configuration_t object
    void (* load)(void);
    // @brief gets the value for the configuration key=value pair identified by key
    bool (* read_configuration_item)(const char * key, config_key_value_pair_t * configurationLine);
} gsdc_iic_configuration_t;


/**
 * @brief Instantiate, configure, and populate a gsdc_iic_configuration_t object
 * @param spiffsInformation (gsdc_iic_configuration_file_data_t *) description of the SPIFFS file system
 */
gsdc_iic_configuration_t * gsdc_iic_configuration_init(gsdc_iic_configuration_file_data_t * spiffsInformation);

#ifdef __cplusplus
}
#endif

#endif // __IIC_CONFIGURATION_H__