#ifndef __IIC_MASTER_H__
#define __IIC_MASTER_H__

#include "led_controller.h"
#include "gsdc_iic_configuration.h"

#ifdef __cplusplus
extern "C" {
#endif

void gsdc_iic_master_task_create(gsdc_iic_configuration_t * iic_configuration);
size_t gsdc_iic_master_send_request_to_device(uint8_t * command, gsdc_iic_connected_device_t * device, led_controller_t * read_indicator, led_controller_t * write_indicator);

#ifdef __cplusplus
}
#endif

#endif // __IIC_MASTER_H__