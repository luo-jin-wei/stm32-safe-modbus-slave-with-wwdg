#ifndef CRC16_H
#define CRC16_H

#include <stdint.h>

/**
 * @brief  计算 Modbus RTU 帧的 CRC16（查表法）
 * @param  data   待计算的数据指针（不含 CRC 本身）
 * @param  length 数据长度（字节）
 * @retval CRC16 结果（uint16_t，低字节在前即为 CRC_L）
 */
uint16_t crc16_modbus(const uint8_t *data, uint16_t length);

/**
 * @brief  验证一帧数据的 CRC 是否正确
 * @param  data         帧数据指针（不含 CRC 本身）
 * @param  length       帧数据长度（字节）
 * @param  received_crc 收到的 CRC 值（uint16_t，已按高8位|低8位组合好）
 * @retval 1: CRC 正确  0: CRC 错误
 */
uint8_t crc16_modbus_verify(const uint8_t *data, uint16_t length, uint16_t received_crc);

/**
 * @brief  根据 FrameParser_t 结构计算 CRC16
 * @note   计算范围：addr + func + len + data[0..len-1]
 * @param  addr  设备地址
 * @param  func  功能码
 * @param  len   数据长度
 * @param  data  数据指针
 * @retval CRC16 结果
 */
uint16_t crc16_modbus_frame(uint8_t addr, uint8_t func, uint8_t len, const uint8_t *data);

#endif /* CRC16_H */
