#ifndef __IIC_RECEIVER_TASK_H__
#define __IIC_RECEIVER_TASK_H__

#include "iic_configuration.h"

#ifdef __cplusplus
extern "C" {
#endif

void i2c_create_receiver_task(iic_configuration_t * iic_configuration, iic_command_received_from_master_event_handler_t * handler);
void i2c_send_data(const char * out_message, size_t length);

#ifdef CONFIG_USE_TEST_MESSAGE_DATA
void receiver_send_test_data();
#endif // CONFIG_USE_TEST_MESSAGE_DATA

#ifdef __cplusplus
}
#endif

#endif // __IIC_RECEIVER_TASK_H__