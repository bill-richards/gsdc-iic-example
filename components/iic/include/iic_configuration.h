#ifndef __IIC_CONFIGURATION_H__
#define __IIC_CONFIGURATION_H__

#include <stdbool.h>
#include <configuration_file.h>
#include <iic_common.h>

#include "freertos/FreeRTOS.h"

#ifdef __cplusplus
extern "C" {
#endif

#define IIC_CONFIGURATION_MAXIMUM_CONNECTED_DEVICES 10

typedef struct {
    configuration_file_data_t * SpiffsInfo;
    connected_device_t * ConnectedDevices;
    int ConnectedDeviceCount;
    uint8_t  I2CAddress;

    void (* data_received_from_client)(connected_device_t * client);
    connected_device_t * (*get_connected_device)(int index);    
    void (* load)(void);
    bool (* read_configuration_item)(const char * key, config_key_value_pair_t * configurationLine);
} iic_configuration_t;


iic_configuration_t * gsdc_iic_configuration_init(configuration_file_data_t * spiffsInformation);

#ifdef __cplusplus
}
#endif

#endif // __IIC_CONFIGURATION_H__