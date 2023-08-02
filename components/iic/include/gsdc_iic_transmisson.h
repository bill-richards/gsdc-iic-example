#ifndef __IIC_TRANSMISSION_H__
#define __IIC_TRANSMISSION_H__

#include "gsdc_iic_common.h"
#include "esp_logging.h"

#ifdef CONFIG_GSDC_IIC_SCL_FREQUENCY
#define CONFIG_I2C_MASTER_FREQUENCY CONFIG_GSDC_IIC_SCL_FREQUENCY
#else
#define CONFIG_I2C_MASTER_FREQUENCY 240000
#endif

#define I2C_MASTER_SCL_IO CONFIG_I2C_SCL                    /*!< gpio number for I2C master clock */
#define I2C_MASTER_SDA_IO CONFIG_I2C_SDA                    /*!< gpio number for I2C master data  */
#define I2C_MASTER_NUM I2C_NUMBER(CONFIG_I2C_PORT_NUM)      /*!< I2C port number for master dev */
#define I2C_MASTER_FREQ_HZ CONFIG_I2C_MASTER_FREQUENCY      /*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE 0                         /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE 0                         /*!< I2C master doesn't need buffer */
#define WRITE_BIT I2C_MASTER_WRITE                          /*!< I2C master write */
#define READ_BIT I2C_MASTER_READ                            /*!< I2C master read */


#ifdef __cplusplus
extern "C" {
#endif

esp_err_t __attribute__((unused)) iic_master_write_to_slave(uint8_t iicAddress, uint8_t *data_wr, size_t size);
void __attribute__((unused)) iic_master_read_variable_length_from_slave(gsdc_iic_connected_device_t * device);

#ifdef __cplusplus
}
#endif

#endif // __IIC_TRANSMISSION_H__