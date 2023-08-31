#ifndef __IIC_EXAMPLE_H__
#define __IIC_EXAMPLE_H__

#include <esp_logging.h>
#include <gsdc_iic_master.h>
#include <gsdc_iic_client.h>
#include <gsdc_ota_subsystem.h>

#include <gsdc_iic_common.h>
#include <gsdc_iic_configuration.h>
#include <gsdc_sensor.h>


class IIC_Example final
{
public:
    void initialize_client(gsdc_iic_configuration_t * configuration);
    void initialize_master(gsdc_iic_configuration_t * configuration);
};


#endif // __IIC_EXAMPLE_H__