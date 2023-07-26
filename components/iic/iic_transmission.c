#include "iic_transmisson.h"
#include "esp_system.h"

/**
 *
 * _______________________________________________________________________________________
 * | start | slave_addr + rd_bit +ack | read n-1 bytes + ack | read 1 byte + nack | stop |
 * --------|--------------------------|----------------------|--------------------|------|
 *
 */
esp_err_t __attribute__((unused)) i2c_master_read_from_slave(uint8_t iicAddress, uint8_t * pDataOut, size_t dataLength)
{
    if (dataLength == 0) {
        return ESP_OK;
    }

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    ESP_ERROR_CHECK(i2c_master_start(cmd));
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (iicAddress << 1) | READ_BIT, ACK_CHECK_EN));
    if (dataLength > 1) {
        ESP_ERROR_CHECK(i2c_master_read(cmd, pDataOut, dataLength - 1, ACK_VAL));
    }
    ESP_ERROR_CHECK(i2c_master_read_byte(cmd, pDataOut + dataLength - 1, NACK_VAL));
    ESP_ERROR_CHECK(i2c_master_stop(cmd));
    esp_err_t ret = i2c_master_cmd_begin(0, cmd, 100 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    pDataOut[dataLength] = '\0';
    
    return ret;
}
void i2c_master_read_variable_length_from_slave(connected_device_t * device)
{
    char data_size[IIC_MESSAGE_LENGTH_FIELD_SIZE+1];
    uint8_t *pSizeData = (uint8_t *)calloc(IIC_MESSAGE_LENGTH_FIELD_SIZE+1, sizeof(uint8_t));

    ESP_ERROR_CHECK(i2c_master_read_from_slave(device->I2CAddress, pSizeData, sizeof(pSizeData)));
    sprintf(data_size, "%s", pSizeData);
    free(pSizeData);
    device->DataLength = atoi(data_size);

    ESP_ERROR_CHECK(i2c_master_read_from_slave(device->I2CAddress, &device->ReceivedData[0], device->DataLength));
 }

/**
 * @brief Test code to write esp-i2c-slave
 *        Master device write data to slave(both esp32),
 *        the data will be stored in slave buffer.
 *        We can read them out from slave buffer.
 *
 * ___________________________________________________________________
 * | start | slave_addr + wr_bit + ack | write n bytes + ack  | stop |
 * --------|---------------------------|----------------------|------|
 *
 * @note cannot use master write slave on esp32c3 because there is only one i2c controller on esp32c3
 */
esp_err_t __attribute__((unused)) i2c_master_write_to_slave(uint8_t iicAddress, uint8_t * pDataIn, size_t dataLength)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    ESP_ERROR_CHECK(i2c_master_start(cmd));
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (iicAddress << 1) | WRITE_BIT, ACK_CHECK_EN));
    ESP_ERROR_CHECK(i2c_master_write(cmd, pDataIn, dataLength, ACK_CHECK_EN));
    ESP_ERROR_CHECK(i2c_master_stop(cmd));
    esp_err_t ret = i2c_master_cmd_begin(0, cmd, 100 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
}
