#ifndef __IIC_MASTER_TASK_H__
#define __IIC_MASTER_TASK_H__

#include "iic_configuration.h"
#include "led_controller.h"

#ifdef __cplusplus
extern "C" {
#endif

void i2c_master_task_create(iic_configuration_t * iic_configuration);
size_t make_request_to_device(uint8_t * command, connected_device_t * device, led_controller_t * read_indicator, led_controller_t * write_indicator);


#ifdef __cplusplus
}
#endif

#endif // __IIC_MASTER_TASK_H__