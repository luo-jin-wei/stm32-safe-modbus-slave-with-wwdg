#ifndef SHT30_H
#define SHT30_H

#include <stdint.h>

/* SHT30 I2C 地址（ADDR 引脚接地时为 0x44，接 VCC 时为 0x45） */
#define SHT30_ADDR  0x44

/* 单次测量命令：时钟拉伸（Clock Stretching）+ 高重复性 */
#define SHT30_CMD_SINGLESHOT_CS_HIGH  0x2C06

/* 软复位命令 */
#define SHT30_CMD_SOFT_RESET          0x30A2

/**
 * @brief  SHT30 初始化（发送软复位）
 * @retval 0: 成功  非0: 失败
 */
uint8_t sht30_init(void);

/**
 * @brief  SHT30 软复位
 * @retval 0: 成功  非0: 失败
 */
uint8_t sht30_soft_reset(void);

/**
 * @brief  单次采集温度+湿度（带 Clock Stretching）
 * @param  temperature  输出温度值（摄氏度）
 * @param  humidity     输出湿度值（%RH）
 * @retval 0: 成功  1:发送错误  2:接收错误  3:温度CRC错  4:湿度CRC错
 */
uint8_t sht30_read_single_shot(float *temperature, float *humidity);

/**
 * @brief  SHT30 专用的 CRC-8 校验
 * @note   Polynomial: 0x31, Init: 0xFF
 * @param  data  数据指针
 * @param  len   数据长度（字节）
 * @retval CRC8 值
 */
uint8_t sht30_crc8(uint8_t *data, uint8_t len);

#endif /* SHT30_H */
