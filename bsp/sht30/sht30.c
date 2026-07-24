#include "sht30.h"
#include "i2c.h"
#include <stdio.h>

extern I2C_HandleTypeDef hi2c1;

/**
 * @brief SHT30 CRC-8 计算
 * Polynomial : 0x31 (x^8 + x^5 + x^4 + 1)
 * Initial    : 0xFF
 */
uint8_t sht30_crc8(uint8_t *data, uint8_t len)
{
    uint8_t crc = 0xFF;
    for (uint8_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x80) {
                crc = (crc << 1) ^ 0x31;
            } else {
                crc <<= 1;
            }
        }
    }
    return crc;
}

/**
 * @brief SHT30 初始化：发送软复位，确保传感器处于已知状态
 */
uint8_t sht30_init(void)
{
    return sht30_soft_reset();
}

/**
 * @brief SHT30 软复位
 */
uint8_t sht30_soft_reset(void)
{
    uint8_t cmd[2] = {0x30, 0xA2};
    HAL_StatusTypeDef status = HAL_I2C_Master_Transmit(&hi2c1, SHT30_ADDR << 1, cmd, 2, 100);
    if (status != HAL_OK) {
        return 1;
    }
    HAL_Delay(2);  // 软复位后等待 2ms（手册要求不超过 2ms）
    return 0;
}

/**
 * @brief 单次采集温度+湿度
 * 命令: 0x2C06 (单次测量, Clock Stretching, High Repeatability)
 * 返回: 6 字节  [Temp_MSB, Temp_LSB, Temp_CRC, Hum_MSB, Hum_LSB, Hum_CRC]
 */
uint8_t sht30_read_single_shot(float *temperature, float *humidity)
{
    uint8_t cmd[2] = {0x2C, 0x06};  // 高8位=0x2C, 低8位=0x06
    uint8_t data[6];
printf("RAW: %02X %02X %02X %02X %02X %02X\r\n",
         data[0], data[1], data[2], data[3], data[4], data[5]);
    /* 1. 发送测量命令 */
    HAL_StatusTypeDef status = HAL_I2C_Master_Transmit(
        &hi2c1, SHT30_ADDR << 1, cmd, 2, 100);
    if (status != HAL_OK) {
        printf("SHT30 TX ERR\r\n");
        return 1;
    }

    /* 2. 读取 6 字节数据
     * 因为命令带 Clock Stretching，SHT30 会自动拉低 SCL
     * 直到测量完成，HAL 会等待并自动接收 */
    status = HAL_I2C_Master_Receive(
        &hi2c1, SHT30_ADDR << 1, data, 6, 200);
    if (status != HAL_OK) {
        printf("SHT30 RX ERR\r\n");
        return 2;
    }

    /* 3. CRC 校验 */
    if (sht30_crc8(&data[0], 2) != data[2]) {
        printf("SHT30 TEMP CRC ERR\r\n");
        return 3;
    }
    if (sht30_crc8(&data[3], 2) != data[5]) {
        printf("SHT30 HUM CRC ERR\r\n");
        return 4;
    }

    /* 4. 温度换算: T = -45 + 175 * (raw / 65535.0) */
    uint16_t temp_raw = (data[0] << 8) | data[1];
    *temperature = -45.0f + 175.0f * ((float)temp_raw / 65535.0f);

    /* 5. 湿度换算: RH = 100 * (raw / 65535.0) */
    uint16_t hum_raw = (data[3] << 8) | data[4];
    *humidity = 100.0f * ((float)hum_raw / 65535.0f);

    return 0;
}
