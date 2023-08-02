#ifndef __IIC_MASTER_H__
#define __IIC_MASTER_H__

#include "gsdc_iic_configuration.h"

#ifdef __cplusplus
extern "C" {
#endif
/**
 * @brief Create and run the task for the Master IIC device. The task will poll all configured IIC clients on the IIC Bus, and return all data received.
 * @param iic_configuration an instantiated gsdc_iic_configuration_t
 */
void gsdc_iic_master_task_create(gsdc_iic_configuration_t * iic_configuration);

#ifdef __cplusplus
}
#endif

#endif // __IIC_MASTER_H__