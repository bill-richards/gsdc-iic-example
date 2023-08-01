#ifndef __IIC_COMMON_H__
#define __IIC_COMMON_H__

#include "driver/i2c.h"

#define PACKET_LENGTH 2048
#define CONFIG_I2C_PORT_NUM 0
#define IIC_MESSAGE_LENGTH_FIELD_SIZE 4

#if CONFIG_GSDC_IIC_IS_MASTER
    #define MASTER_POLLING_PERIOD CONFIG_GSDC_IIC_POLLING_PERIOD
#else
    #define MASTER_POLLING_PERIOD 5000
#endif

#ifdef CONFIG_GSDC_IIC_SCL_PIN
#define CONFIG_I2C_SCL CONFIG_GSDC_IIC_SCL_PIN
#else
#define CONFIG_I2C_SCL 22
#endif

#ifdef CONFIG_GSDC_IIC_SDA_PIN
#define CONFIG_I2C_SDA CONFIG_GSDC_IIC_SDA_PIN
#else
#define CONFIG_I2C_SDA 21
#endif

#define _I2C_NUMBER(num) I2C_NUM_##num
#define I2C_NUMBER(num) _I2C_NUMBER(num)

#define DATA_LENGTH PACKET_LENGTH               
#define I2C_SLAVE_TX_BUF_LEN (DATA_LENGTH)      /*!< I2C slave tx buffer size */
#define I2C_SLAVE_RX_BUF_LEN (DATA_LENGTH)      /*!< I2C slave rx buffer size */

#define ACK_CHECK_EN 0x1                        /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS 0x0                       /*!< I2C master will not check ack from slave */
#define ACK_VAL 0x0                             /*!< I2C ack value */
#define NACK_VAL 0x1                            /*!< I2C nack value */

#define _I2C_NUMBER(num) I2C_NUM_##num
#define I2C_NUMBER(num) _I2C_NUMBER(num)

#define GSDC_IIC_COMMAND_LENGTH 2
#define GSDC_IIC_COMMANDS_SEND_ALL_DATA "4C"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct connected_device gsdc_iic_connected_device_t;
typedef void (*gsdc_iic_data_received_from_slave_event_handler_t)(gsdc_iic_connected_device_t * pConnectedDevice);
typedef void (*gsdc_iic_command_received_event_handler_t)(const char * command);

struct connected_device {
        uint8_t I2CAddress;
        uint8_t MasterI2CAddress;
        uint8_t ReceivedData[DATA_LENGTH+1];
         size_t DataLength;
};

/**
 * @brief test function to show buffer contents
 */
void display_buffer_contents(uint8_t *buf, int len);
void give_semaphore();
void take_semaphore();

#ifdef __cplusplus
}
#endif

#endif // __IIC_COMMON_H__