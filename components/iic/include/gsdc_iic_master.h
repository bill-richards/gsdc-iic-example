#ifndef __IIC_MASTER_H__
#define __IIC_MASTER_H__

#include "led_controller.h"
#include "gsdc_iic_configuration.h"

#ifdef __cplusplus
extern "C" {
#endif

void gsdc_iic_master_task_create(gsdc_iic_configuration_t * iic_configuration);


#ifdef __cplusplus
}
#endif

#endif // __IIC_MASTER_H__